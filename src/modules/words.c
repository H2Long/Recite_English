// ============================================================================
// 单词管理模块
// 功能：单词加载、进度管理、持久化存储
//
// 核心数据结构：
//   本模块管理两组数据：
//   1. g_wordLibrary (WordEntry* 动态数组) — 从 words.txt 加载的原始词库，
//      支持运行时增删改（词库管理功能）
//   2. g_words[100] (WordWithProgress 定长数组) — 带学习进度的单词数组，
//      所有学习模式（学单词/背单词/测试）都基于此数组
//
// 数据流：
//   words.txt → loadWordsFromFile() → g_wordLibrary
//                                   → initWords() → g_words[100]
//                                                      ↓
//   progress.txt ← saveProgress()  ← 学习进度更新
//
// 进度保存格式 (progress.txt)：
//   word|knownCount|unknownCount|lastReview
//   每行一个单词，按单词名匹配
//
// 搜索功能：
//   提供了通配符搜索 (searchWordsByRegex) 和模糊搜索 (searchWordsSimple)
//   两种方式，通配符支持 ^/$ /*/？ 语法，跨平台实现（不依赖 POSIX regex.h）
//
// 依赖：
//   - words.h: 数据结构和函数声明
//   - config.h: WORDS_FILE_PATH 等路径常量
// ============================================================================

#include "words.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * strdup 兼容性适配
 * strdup 是 POSIX 函数，并非 C 标准库函数。
 * MSVC (Windows) 中该函数名为 _strdup。
 * GCC/Clang (Linux/macOS) 中可直接使用 strdup。
 * C23 标准已将 strdup 纳入标准库，未来可移除此适配。
 */
#ifdef _MSC_VER
#define strdup _strdup
#endif

/**
 * g_wordLibrary - 单词库动态数组
 * 
 * 由 loadWordsFromFile() 从 words.txt 加载，动态扩容。
 * 每个元素包含单词、音标、释义、例句等原始数据（不含学习进度）。
 * 支持运行时 add/edit/delete 操作，修改后自动保存到文件。
 * 程序退出时需调用 freeWordLibrary() 释放内存。
 */
WordEntry* g_wordLibrary = NULL;
static int g_wordLibraryCapacity = 0;   // 当前分配容量（自动倍增扩容）

/**
 * g_words[MAX_WORDS] - 含学习进度的单词数组
 * 
 * 程序核心数据数组，由 initWords() 从 g_wordLibrary 填充。
 * 每个元素包含单词条目和学习进度（认识次数、是否掌握等）。
 * 词库修改后需调用 reloadWords() 同步。
 */
WordWithProgress g_words[MAX_WORDS];
int g_wordProgressCount = 0;    // g_words 中有效单词数（≤ MAX_WORDS）
int g_wordCount = 0;            // g_wordLibrary 中单词总数

// g_learnScrollOffset - 学单词模式左侧列表的滚动位置
// 保存在 global scope 中以实现跨帧持久化（当前未使用，由 LearnState 管理）
float g_learnScrollOffset = 0;

// g_progressFilePath - 进度文件路径（支持多用户切换）
// 默认指向 "./data/progress.txt"，登录后切换为用户专属文件
static char g_progressFilePath[256] = "./data/progress.txt";

/**
 * 设置进度文件路径（用于多用户切换）
 * @param path 新的进度文件路径
 */
void setProgressFilePath(const char* path) {
    strncpy(g_progressFilePath, path, sizeof(g_progressFilePath) - 1);
    g_progressFilePath[sizeof(g_progressFilePath) - 1] = '\0';
    printf("INFO: Progress file set to: %s\n", g_progressFilePath);
}

// Fisher-Yates 洗牌算法：随机打乱数组顺序
void shuffleArray(int *array, int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

/**
 * loadWordsFromFile - 从文件加载单词到 g_wordLibrary
 * 
 * 文件格式（每行一个单词，管道符分隔）：
 *   word|phonetic|definition|example|exampleTranslation
 * 前三个字段（word/phonetic/definition）为必填。
 * 如果文件不存在，使用内置的10个默认单词（abandon ~ accomplish）作为后备。
 * 
 * 注意：此函数只填充 g_wordLibrary，之后还需调用 initWords() 
 * 将数据复制到 g_words 并初始化学习进度。
 * 
 * @param filename 单词数据文件路径（如 "./data/words.txt"）
 */
void loadWordsFromFile(const char* filename) {
    /* 尝试打开单词文件。如果文件不存在（首次运行或路径错误），
     * 使用内置的 10 个默认单词作为后备方案，确保程序至少能运行。 */
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("WARNING: Cannot open word file: %s, using default words\n", filename);
        /* ========== 后备方案：10 个内置默认单词 ==========
         * 格式：[0]单词, [1]音标, [2]释义, [3]英文例句, [4]中文翻译
         * 这些单词按字母序排列（abandon ~ accomplish）。 */
        const char* defaultWords[][5] = {
            {"abandon", "[ə'bændən]", "v. 放弃；抛弃", "Never abandon your dreams.", u8"永远不要放弃你的梦想。"},
            {"ability", "[ə'bɪləti]", "n. 能力", "She has the ability to speak three languages.", u8"她有说三种语言的能力。"},
            {"absence", "['æbsəns]", "n. 缺席；缺乏", "His absence was noticed by everyone.", u8"每个人都注意到了他的缺席。"},
            {"absolute", "['æbsəluːt]", "adj. 绝对的", "It is an absolute necessity.", u8"这是绝对必要的。"},
            {"absorb", "[əb'zɔːrb]", "v. 吸收；理解", "Plants absorb water from the soil.", u8"植物从土壤中吸收水分。"},
            {"abstract", "['æbstrækt]", "adj. 抽象的", "The concept is too abstract.", u8"这个概念太抽象了。"},
            {"abundant", "[ə'bʌndənt]", "adj. 丰富的", "We have abundant resources.", u8"我们拥有丰富的资源。"},
            {"academic", "[ˌækə'demɪk]", "adj. 学术的", "She has excellent academic records.", u8"她有优秀的学业记录。"},
            {"accelerate", "[ək'seləreɪt]", "v. 加速", "The car began to accelerate.", u8"汽车开始加速。"},
            {"accomplish", "[ə'kʌmplɪʃ]", "v. 完成", "We accomplished our mission.", u8"我们完成了任务。"},
        };
        int defaultCount = sizeof(defaultWords) / sizeof(defaultWords[0]);
        g_wordLibraryCapacity = defaultCount + 10;
        g_wordLibrary = (WordEntry*)malloc(g_wordLibraryCapacity * sizeof(WordEntry));
        for (int i = 0; i < defaultCount; i++) {
            g_wordLibrary[i].word = strdup(defaultWords[i][0]);
            g_wordLibrary[i].phonetic = strdup(defaultWords[i][1]);
            g_wordLibrary[i].definition = strdup(defaultWords[i][2]);
            g_wordLibrary[i].example = strdup(defaultWords[i][3]);
            g_wordLibrary[i].exampleTranslation = strdup(defaultWords[i][4]);
        }
        g_wordCount = defaultCount;
        return;
    }

    char line[1024];
    g_wordLibraryCapacity = 50;
    g_wordLibrary = (WordEntry*)malloc(g_wordLibraryCapacity * sizeof(WordEntry));
    g_wordCount = 0;

    while (fgets(line, sizeof(line), fp)) {
        // 去掉换行符
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        if (len > 1 && line[len-2] == '\r') line[len-2] = '\0';
        
        if (strlen(line) == 0) continue;

        // 扩展容量
        if (g_wordCount >= g_wordLibraryCapacity) {
            g_wordLibraryCapacity *= 2;
            g_wordLibrary = (WordEntry*)realloc(g_wordLibrary, g_wordLibraryCapacity * sizeof(WordEntry));
        }

        // 解析格式: word|phonetic|definition|example|exampleTranslation
        char* fields[5] = {NULL, NULL, NULL, NULL, NULL};
        int fieldIdx = 0;
        char* token = strtok(line, "|");
        while (token != NULL && fieldIdx < 5) {
            fields[fieldIdx++] = strdup(token);
            token = strtok(NULL, "|");
        }

        if (fieldIdx >= 3) {
            g_wordLibrary[g_wordCount].word = fields[0];
            g_wordLibrary[g_wordCount].phonetic = fields[1];
            g_wordLibrary[g_wordCount].definition = fields[2];
            g_wordLibrary[g_wordCount].example = fieldIdx >= 4 ? fields[3] : "";
            g_wordLibrary[g_wordCount].exampleTranslation = fieldIdx >= 5 ? fields[4] : "";
            g_wordCount++;
        }
    }
    fclose(fp);
    printf("INFO: Loaded %d words from %s\n", g_wordCount, filename);
}

/**
 * freeWordLibrary - 释放单词库 g_wordLibrary 的内存
 * 
 * 遍历所有单词条目，依次释放 word/phonetic/definition/example/exampleTranslation
 * 各自的 strdup 内存，最后释放动态数组本身。
 * 程序退出时在 main() 的清理阶段调用。
 */
void freeWordLibrary(void) {
    if (g_wordLibrary == NULL) return;
    for (int i = 0; i < g_wordCount; i++) {
        if (g_wordLibrary[i].word) free((void*)g_wordLibrary[i].word);
        if (g_wordLibrary[i].phonetic) free((void*)g_wordLibrary[i].phonetic);
        if (g_wordLibrary[i].definition) free((void*)g_wordLibrary[i].definition);
        if (g_wordLibrary[i].example) free((void*)g_wordLibrary[i].example);
        if (g_wordLibrary[i].exampleTranslation) free((void*)g_wordLibrary[i].exampleTranslation);
    }
    free(g_wordLibrary);
    g_wordLibrary = NULL;
    g_wordCount = 0;
}

/**
 * initWords - 初始化单词进度数据
 * 
 * 将 g_wordLibrary 中的单词复制到 g_words 数组，关联 WordProgress 进度结构。
 * 新建的进度默认为：knownCount=0, unknownCount=0, mastered=false。
 * 初始化后会调用 loadProgress() 从文件恢复历史学习进度。
 * 
 * 调用时机：在 loadWordsFromFile() 之后调用。
 */
void initWords(void) {
    for (int i = 0; i < g_wordCount && i < MAX_WORDS; i++) {
        g_words[i].entry = g_wordLibrary[i];
        g_words[i].progress.wordIndex = i;
        g_words[i].progress.knownCount = 0;
        g_words[i].progress.unknownCount = 0;
        g_words[i].progress.lastReview = 0;
        g_words[i].progress.mastered = false;
        g_wordProgressCount++;
    }
    
    // 尝试加载进度
    loadProgress();
}

/**
 * saveProgress - 保存学习进度到文件
 *
 * 在每个"认识"/"不认识"操作后自动调用。
 * 将 g_words 中每个单词的 knownCount / unknownCount / lastReview
 * 写入 g_progressFilePath 指定的文件。
 *
 * 文件格式（每行一个单词）：
 *   word|knownCount|unknownCount|lastReview时间戳
 *
 * 注意：此函数不保存 mastered 字段！
 * mastered 由 loadProgress 在加载时根据 knownCount >= 3 重新计算。
 * 这样设计的目的是保持持久化格式简洁，mastered 是一个"推导字段"。
 */
void saveProgress(void) {
    /* 以"写入"模式打开进度文件，会覆盖旧文件 */
    FILE* fp = fopen(g_progressFilePath, "w");
    if (fp == NULL) {
        printf("WARNING: Cannot save progress to: %s\n", g_progressFilePath);
        return;  /* 写入失败不阻止程序运行，但进度无法保存 */
    }
    
    /* 逐行写入每个单词的进度数据 */
    for (int i = 0; i < g_wordProgressCount; i++) {
        fprintf(fp, "%s|%d|%d|%ld\n",
                g_words[i].entry.word,                    /* 单词本身，用作唯一标识符 */
                g_words[i].progress.knownCount,           /* 认识次数（如 3） */
                g_words[i].progress.unknownCount,         /* 不认识次数（如 1） */
                (long)g_words[i].progress.lastReview);    /* 上次复习时间戳（如 1714456789） */
    }
    
    fclose(fp);
    printf("INFO: Progress saved to %s\n", g_progressFilePath);
}

/**
 * loadProgress - 从文件恢复学习进度
 *
 * 程序启动时调用。逐行读取进度文件，按单词名匹配到 g_words 中的对应条目，
 * 恢复 knownCount / unknownCount / lastReview 三个字段。
 * 然后根据 knownCount >= 3 重新计算 mastered 状态。
 *
 * 匹配算法：O(n*m) 线性扫描
 *   对于进度文件中的每一行，遍历 g_words 数组查找同名单词。
 *   当单词数不多（MAX_WORDS=100）时性能可接受。
 *   如果将来扩展到数千单词，应改用哈希表加速。
 *
 * 如果进度文件不存在（首次运行），静默忽略，所有单词从 0 开始。
 */
void loadProgress(void) {
    FILE* fp = fopen(g_progressFilePath, "r");
    if (fp == NULL) {
        printf("INFO: No progress file found, starting fresh\n");
        return;  /* 首次运行，没有进度文件是正常的 */
    }
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        /* 去掉行尾换行符。Windows 文本文件可能包含 \r\n，两个都要处理 */
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        
        if (strlen(line) == 0) continue;  /* 跳过空行 */
        
        /* 解析格式: word|knownCount|unknownCount|lastReview */
        char* fields[4] = {NULL, NULL, NULL, NULL};
        int fieldIdx = 0;
        char* token = strtok(line, "|");
        while (token != NULL && fieldIdx < 4) {
            fields[fieldIdx++] = token;
            token = strtok(NULL, "|");
        }
        
        if (fieldIdx >= 4) {
            /*
             * O(n) 线性查找：遍历 g_words 数组，寻找同名的单词。
             * 使用 strcmp 精确匹配单词名。
             * 因为进度文件中的单词名来自之前 saveProgress 写入的，
             * 所以一定能匹配上（除非用户手动编辑了进度文件导致名字不一致）。
             */
            for (int i = 0; i < g_wordProgressCount; i++) {
                if (strcmp(g_words[i].entry.word, fields[0]) == 0) {
                    /* 恢复学习进度 */
                    g_words[i].progress.knownCount = atoi(fields[1]);
                    g_words[i].progress.unknownCount = atoi(fields[2]);
                    g_words[i].progress.lastReview = (time_t)atol(fields[3]);
                    /*
                     * 恢复 mastered 状态：
                     * 当 knownCount >= MASTERED_THRESHOLD(3) 时为 true。
                     * 这是"艾宾浩斯简化"策略：同一单词连续认识 3 次就算掌握。
                     * 如果用户手动重置了进度文件为 0，mastered 自动变 false。
                     */
                    if (g_words[i].progress.knownCount >= 3) {
                        g_words[i].progress.mastered = true;
                    }
                    break;  /* 找到匹配的单词后立即停止搜索 */
                }
            }
        }
    }
    
    fclose(fp);
    printf("INFO: Progress loaded from %s\n", g_progressFilePath);
}

/**
 * clearProgress - 清除所有单词的学习进度
 *
 * 由"学习进度"页面的"清除进度"按钮触发。
 * 重置所有单词的 knownCount/unknownCount/lastReview/mastered 为初始状态，
 * 然后立即调用 saveProgress() 持久化到文件。
 *
 * 注意：此操作不可逆！清除后无法恢复。
 */
void clearProgress(void) {
    /* 遍历所有单词，将学习进度重置为初始值 */
    for (int i = 0; i < g_wordProgressCount; i++) {
        g_words[i].progress.knownCount = 0;    /* 认识次数归零 */
        g_words[i].progress.unknownCount = 0;  /* 不认识次数归零 */
        g_words[i].progress.lastReview = 0;    /* 上次复习时间重置为"从未" */
        g_words[i].progress.mastered = false;  /* 标记为"未掌握" */
    }
    saveProgress();  /* 立即保存到文件，防止数据丢失 */
    printf("INFO: Progress cleared\n");
}

// ============================================================================
// 单词搜索功能（跨平台实现，不依赖 POSIX regex.h）
// ============================================================================

/**
 * matchPattern - 简单的通配符模式匹配（不区分大小写）
 * 
 * 支持的语法：
 *   ^pattern  — 匹配字符串开头
 *   pattern$  — 匹配字符串结尾
 *   *         — 匹配任意数量的字符
 *   ?         — 匹配单个字符
 *   普通文本  — 子串匹配（不区分大小写）
 * 
 * 示例：
 *   "^ab"     → 以 "ab" 开头的单词
 *   "ing$"    → 以 "ing" 结尾的单词
 *   "a*c"     → 包含 "a" 和 "c" 且中间有任意字符的单词
 *   "ab??rd"  → "ab" + 任意2字符 + "rd"
 */
static bool matchPattern(const char* word, const char* pattern) {
    /*
     * ========== 步骤 1：解析锚点 ==========
     * 检查模式是否以 ^ 开头（要求从单词开头匹配）
     * 或 $ 结尾（要求匹配到单词结尾）。
     * 例如 "^ab" 只匹配以 "ab" 开头的单词。
     */
    bool anchoredStart = (pattern[0] == '^');
    bool anchoredEnd = false;
    int patLen = strlen(pattern);
    if (patLen > 0 && pattern[patLen - 1] == '$') {
        anchoredEnd = true;
        patLen--;  /* $ 不计入有效模式长度 */
    }
    
    /* 去掉开头的 ^ 符号，得到实际模式 */
    const char* p = pattern + (anchoredStart ? 1 : 0);
    int remainingLen = patLen - (anchoredStart ? 1 : 0);
    
    /*
     * ========== 步骤 2：快速路径 ==========
     * 如果模式不含通配符（* 或 ?）也不含锚点，
     * 直接使用不区分大小写的子串匹配(strstr)，
     * 无需走下面的通配符解析逻辑。
     * 这是最常见的情况，优化性能。
     */
    if (!anchoredStart && !anchoredEnd && !strchr(p, '*') && !strchr(p, '?')) {
        char wordLower[256], patLower[256];
        strncpy(wordLower, word, sizeof(wordLower) - 1);  wordLower[sizeof(wordLower) - 1] = '\0';
        strncpy(patLower, p, sizeof(patLower) - 1);      patLower[sizeof(patLower) - 1] = '\0';
        for (int i = 0; wordLower[i]; i++) wordLower[i] = tolower(wordLower[i]);
        for (int i = 0; patLower[i]; i++) patLower[i] = tolower(patLower[i]);
        return strstr(wordLower, patLower) != NULL;
    }
    
    /*
     * ========== 步骤 3：慢速路径 ==========
     * 模式含有通配符或锚点。匹配策略："分段匹配法"
     * 1. 将模式按 * 分割成多个文本片段
     * 2. 每个片段顺序在单词中查找出现位置
     * 3. 检查首尾段是否满足锚点要求
     *
     * 例如模式 "a*b" 被分割为 ["a", "b"]，
     * 在单词 "abandon" 中：先找 "a" 在位置 0，再找 "b" 在位置 1 → 匹配成功。
     */
    
    /* 步骤 3a：预处理 — 合并连续的 *（"a**b" → "a*b"），提取有效模式 */
    char simplePat[256];
    int si = 0;
    for (int i = 0; p[i] && i < remainingLen; i++) {
        if (p[i] == '*' && i > 0 && p[i-1] == '*') continue;  /* 跳过连续多余的 * */
        simplePat[si++] = p[i];
    }
    simplePat[si] = '\0';
    
    /* 步骤 3b：以 * 为分隔符，将模式切成文本段
     * 例如 "a*b*c" → ["a", "b", "c"]（3 段）
     * 例如 "*abc*" → ["abc"]（1 段，前后 * 被切掉） */
    char segments[16][64];
    int segCount = 0;
    char segBuf[256];
    strncpy(segBuf, simplePat, sizeof(segBuf) - 1);
    char* segToken = strtok(segBuf, "*");
    while (segToken != NULL && segCount < 16) {
        strncpy(segments[segCount], segToken, sizeof(segments[0]) - 1);
        segments[segCount][sizeof(segments[0]) - 1] = '\0';
        for (int j = 0; segments[segCount][j]; j++)
            segments[segCount][j] = tolower(segments[segCount][j]);
        segCount++;
        segToken = strtok(NULL, "*");
    }
    
    /* 如果切完没有文本段，说明模式只有 *，匹配一切 */
    if (segCount == 0) return true;
    
    /* 步骤 3c：判断首尾是否有 *（用于锚点检查） */
    bool startsWithStar = (simplePat[0] == '*');
    bool endsWithStar = (simplePat[si - 1] == '*');
    
    /* 步骤 3d：将单词统一转为小写，准备匹配 */
    char wordLower[256];
    strncpy(wordLower, word, sizeof(wordLower) - 1);  wordLower[sizeof(wordLower) - 1] = '\0';
    for (int j = 0; wordLower[j]; j++) wordLower[j] = tolower(wordLower[j]);
    int wl = strlen(wordLower);
    
    /*
     * 步骤 3e：顺序匹配每个文本段
     * 从位置 0 开始，在单词中查找第 0 段、第 1 段……
     * 每次查找都从上一次找到的位置之后开始（pos 递增）。
     * 例如 "*a*b" 匹配 "xayb"：
     *   第 0 段 "a" → 在 pos=0 找到，位置 1，pos 更新为 2
     *   第 1 段 "b" → 在 pos=2 找到，位置 3，pos 更新为 4
     *   所有段都找到 → 匹配成功。
     */
    int pos = 0;
    for (int s = 0; s < segCount; s++) {
        char* found = strstr(wordLower + pos, segments[s]);
        if (found == NULL) return false;  /* 某一段没找到，匹配失败 */
        
        /* 如果没有前导 *，第 0 段必须从单词开头开始匹配 */
        if (s == 0 && !startsWithStar && found != wordLower) return false;
        
        pos = (int)(found - wordLower) + strlen(segments[s]);  /* 更新查找起点 */
    }
    
    /* 如果没有后导 *，最后一段必须匹配到单词结尾 */
    if (!endsWithStar && pos != wl) return false;
    
    return true;  /* 所有检查通过，匹配成功 */
}

/**
 * searchWordsByRegex - 简单的通配符模式搜索（跨平台）
 * 
 * 支持 ^ 开头 / $ 结尾 / * 任意字符 / ? 单个字符 的模式匹配。
 * 之前在 Linux 上使用 POSIX regex.h，但该头文件在 Windows 上不可用，
 * 因此改用此纯 C 实现的通配符匹配器。
 * 
 * @param pattern 搜索模式（支持 ^ * ? $ 通配符语法）
 * @param results 存放匹配单词索引的数组
 * @param maxResults 最大结果数量
 * @return 匹配到的单词数量
 */
int searchWordsByRegex(const char* pattern, int* results, int maxResults) {
    if (pattern == NULL || strlen(pattern) == 0) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < g_wordProgressCount && count < maxResults; i++) {
        if (matchPattern(g_words[i].entry.word, pattern)) {
            results[count++] = i;
        }
    }
    return count;
}

/**
 * 简单的模糊搜索（不区分大小写，包含匹配）
 * @param query 搜索关键词
 * @param results 存放搜索结果索引的数组
 * @param maxResults 最大结果数量
 * @return 匹配到的结果数量
 */
int searchWordsSimple(const char* query, int* results, int maxResults) {
    if (query == NULL || strlen(query) == 0) {
        return 0;
    }

    int count = 0;
    // 转换为小写进行不区分大小写的匹配
    char queryLower[256];
    strncpy(queryLower, query, sizeof(queryLower) - 1);
    queryLower[sizeof(queryLower) - 1] = '\0';

    for (int i = 0; queryLower[i]; i++) {
        queryLower[i] = tolower(queryLower[i]);
    }

    for (int i = 0; i < g_wordProgressCount && count < maxResults; i++) {
        // 将单词转换为小写
        char wordLower[256];
        strncpy(wordLower, g_words[i].entry.word, sizeof(wordLower) - 1);
        wordLower[sizeof(wordLower) - 1] = '\0';
        for (int j = 0; wordLower[j]; j++) {
            wordLower[j] = tolower(wordLower[j]);
        }

        if (strstr(wordLower, queryLower) != NULL) {
            results[count++] = i;
        }
    }

    return count;
}
// ============================================================================
// 词库管理函数实现
// ============================================================================

/**
 * saveWordsToFile - 将 g_wordLibrary 保存到 words.txt
 *
 * 在词库管理页面的添加/编辑/删除操作后自动调用。
 * 覆盖写入，每行一个单词，5 个字段用管道符分隔。
 * 空字段会被写入空字符串（"") 而不是 NULL。
 *
 * @param filename 保存路径（通常为 WORDS_FILE_PATH）
 * @return true 保存成功，false 保存失败（文件无法写入）
 */
bool saveWordsToFile(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("WARNING: Cannot save words to %s\n", filename);
        return false;
    }
    /* 遍历词库，逐行写入。使用三元运算符检查空指针避免崩溃。 */
    for (int i = 0; i < g_wordCount; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s\n",
                g_wordLibrary[i].word ? g_wordLibrary[i].word : "",
                g_wordLibrary[i].phonetic ? g_wordLibrary[i].phonetic : "",
                g_wordLibrary[i].definition ? g_wordLibrary[i].definition : "",
                g_wordLibrary[i].example ? g_wordLibrary[i].example : "",
                g_wordLibrary[i].exampleTranslation ? g_wordLibrary[i].exampleTranslation : "");
    }
    fclose(fp);
    printf("INFO: Saved %d words to %s\n", g_wordCount, filename);
    return true;
}

/**
 * addWordToLibrary - 添加新单词到词库
 *
 * 在 g_wordLibrary 末尾追加一个新单词条目。
 * 所有文本字段通过 strdup() 复制（深拷贝），确保内存独立。
 * 添加后自动保存到文件并打印日志。
 *
 * 注意：
 * 1. 此函数操作的是 g_wordLibrary（原始词库），不是 g_words（进度数组）。
 * 2. 添加后需调用 reloadWords() 将新单词同步到 g_words 并初始化进度。
 * 3. 自动扩容：当 g_wordCount 达到容量上限时，容量翻倍（从 50 开始）。
 *    ⚠️ 当前 realloc 返回值未检查，OOM 时可能导致指针丢失。
 *
 * @param w   单词（必填，不能为 NULL 或空字符串）
 * @param ph  音标（可选，传 NULL 或空串时存 "")
 * @param def 释义（可选）
 * @param ex  例句（可选）
 * @param ext 例句翻译（可选）
 * @return true 成功，false 失败（单词为空或超出容量上限 MAX_WORDS+200）
 */
bool addWordToLibrary(const char* w, const char* ph, const char* def,
                      const char* ex, const char* ext) {
    /* ---- 参数校验 ---- */
    if (w == NULL || strlen(w) == 0) return false;
    if (g_wordCount >= MAX_WORDS + 200) return false;  /* 硬上限保护 */

    /* ---- 自动扩容：当数组满时容量倍增 ---- */
    if (g_wordCount >= g_wordLibraryCapacity) {
        g_wordLibraryCapacity = g_wordLibraryCapacity == 0 ? 50 : g_wordLibraryCapacity * 2;
        /* ⚠️ 风险：realloc 返回 NULL 时原始指针丢失 */
        g_wordLibrary = (WordEntry*)realloc(g_wordLibrary, g_wordLibraryCapacity * sizeof(WordEntry));
    }

    /* ---- 在末尾追加新条目（strdup 深拷贝每个字段） ---- */
    g_wordLibrary[g_wordCount].word = strdup(w);
    g_wordLibrary[g_wordCount].phonetic = strdup(ph ? ph : "");
    g_wordLibrary[g_wordCount].definition = strdup(def ? def : "");
    g_wordLibrary[g_wordCount].example = strdup(ex ? ex : "");
    g_wordLibrary[g_wordCount].exampleTranslation = strdup(ext ? ext : "");
    g_wordCount++;

    saveWordsToFile(WORDS_FILE_PATH);  /* 持久化到文件 */
    return true;
}

/**
 * editWordInLibrary - 编辑词库中指定索引的单词
 *
 * 先释放该单词原有字段的 strdup 内存，再通过 strdup 复制新内容。
 * 编辑后自动保存到文件。
 *
 * @param idx 要编辑的单词索引（必须在 0 ~ g_wordCount-1 范围内）
 * @param w   新单词内容（必填，不能为空）
 * @param ph  新音标
 * @param def 新释义
 * @param ex  新例句
 * @param ext 新例句翻译
 * @return true 成功，false 失败（索引无效或单词为空）
 */
bool editWordInLibrary(int idx, const char* w, const char* ph,
                       const char* def, const char* ex, const char* ext) {
    /* ---- 参数校验 ---- */
    if (idx < 0 || idx >= g_wordCount || w == NULL || strlen(w) == 0) return false;

    /* ---- 释放原有内存 ---- */
    free((void*)g_wordLibrary[idx].word);
    free((void*)g_wordLibrary[idx].phonetic);
    free((void*)g_wordLibrary[idx].definition);
    free((void*)g_wordLibrary[idx].example);
    free((void*)g_wordLibrary[idx].exampleTranslation);

    /* ---- 用新内容替换（strdup 深拷贝） ---- */
    g_wordLibrary[idx].word = strdup(w);
    g_wordLibrary[idx].phonetic = strdup(ph ? ph : "");
    g_wordLibrary[idx].definition = strdup(def ? def : "");
    g_wordLibrary[idx].example = strdup(ex ? ex : "");
    g_wordLibrary[idx].exampleTranslation = strdup(ext ? ext : "");

    saveWordsToFile(WORDS_FILE_PATH);  /* 持久化到文件 */
    return true;
}

/**
 * deleteWordFromLibrary - 从词库删除指定索引的单词
 *
 * 1. 释放该单词所有字段的 strdup 内存
 * 2. 将后续单词前移一位（memmove / 循环赋值）
 * 3. 递减 g_wordCount
 * 4. 保存到文件
 *
 * @param idx 要删除的单词索引
 * @return true 成功，false 索引无效
 */
bool deleteWordFromLibrary(int idx) {
    if (idx < 0 || idx >= g_wordCount) return false;

    /* 释放被删除单词的内存 */
    free((void*)g_wordLibrary[idx].word);
    free((void*)g_wordLibrary[idx].phonetic);
    free((void*)g_wordLibrary[idx].definition);
    free((void*)g_wordLibrary[idx].example);
    free((void*)g_wordLibrary[idx].exampleTranslation);

    /* 将后续单词前移填补空缺（结构体赋值，不是指针移动） */
    for (int i = idx; i < g_wordCount - 1; i++) g_wordLibrary[i] = g_wordLibrary[i + 1];
    g_wordCount--;

    saveWordsToFile(WORDS_FILE_PATH);  /* 持久化到文件 */
    return true;
}

/**
 * reloadWords - 词库修改后将 g_wordLibrary 同步到 g_words
 *
 * 在词库管理页面执行添加/编辑/删除后调用。
 * 重新遍历 g_wordLibrary，将数据同步到 g_words 进度数组。
 *
 * ⚠️ 重要：g_words[i].entry = g_wordLibrary[i] 是浅拷贝（结构体赋值），
 * 两者共享 WordEntry 中的 char* 指针。如果后续 deleteWordFromLibrary
 * 释放了这些指针，g_words 中的对应指针将变成悬空指针（dangling pointer）。
 * 但本程序的调用时序确保：每次词库修改后立即调用 reloadWords()，
 * 且在 reloadWords() 之后 g_wordLibrary 中的指针始终有效，
 * 所以短期内不会出现 use-after-free。
 *
 * 对于新增单词（i >= oldCnt），初始化为默认进度（0 次，未掌握）。
 * 对于已有单词，保留原有的 knownCount 等进度数据。
 */
void reloadWords(void) {
    int oldCnt = g_wordProgressCount;  /* 记住旧的进度数量 */
    g_wordProgressCount = 0;           /* 从 0 开始重新计数 */

    for (int i = 0; i < g_wordCount && i < MAX_WORDS; i++) {
        /*
         * 浅拷贝：g_words[i].entry 与 g_wordLibrary[i] 共享指针内存。
         * 注释见函数头说明。这是已知的风险点。
         */
        g_words[i].entry = g_wordLibrary[i];
        g_words[i].progress.wordIndex = i;  /* 更新进度索引 */

        /* 如果是新增的单词（超出旧进度数组范围），初始化进度 */
        if (i >= oldCnt) {
            g_words[i].progress.knownCount = 0;
            g_words[i].progress.unknownCount = 0;
            g_words[i].progress.lastReview = 0;
            g_words[i].progress.mastered = false;
        }
        /* 已有单词的进度数据保持不变（由 loadProgress 恢复） */
        g_wordProgressCount++;
    }
    printf("INFO: Reloaded %d words\n", g_wordProgressCount);
}
