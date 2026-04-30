// ============================================================================
// 单词管理模块
// 功能：单词加载、进度管理、持久化存储
// ============================================================================

#include "words.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

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
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("WARNING: Cannot open word file: %s, using default words\n", filename);
        // 使用默认单词
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

// 保存学习进度到文件
// 格式：word|knownCount|unknownCount|lastReview（时间戳）
// 每次背单词操作后自动调用
void saveProgress(void) {
    FILE* fp = fopen(g_progressFilePath, "w");
    if (fp == NULL) {
        printf("WARNING: Cannot save progress to: %s\n", g_progressFilePath);
        return;
    }
    
    for (int i = 0; i < g_wordProgressCount; i++) {
        fprintf(fp, "%s|%d|%d|%ld\n",
                g_words[i].entry.word,
                g_words[i].progress.knownCount,
                g_words[i].progress.unknownCount,
                (long)g_words[i].progress.lastReview);
    }
    
    fclose(fp);
    printf("INFO: Progress saved to %s\n", g_progressFilePath);
}

// 从文件加载学习进度
// 程序启动时调用，恢复之前的学习状态
// 认识次数>=3的单词自动标记为"已掌握"
void loadProgress(void) {
    FILE* fp = fopen(g_progressFilePath, "r");
    if (fp == NULL) {
        printf("INFO: No progress file found, starting fresh\n");
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        // 去掉换行符
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        
        if (strlen(line) == 0) continue;
        
        // 解析格式: word|knownCount|unknownCount|lastReview
        char* fields[4] = {NULL, NULL, NULL, NULL};
        int fieldIdx = 0;
        char* token = strtok(line, "|");
        while (token != NULL && fieldIdx < 4) {
            fields[fieldIdx++] = token;
            token = strtok(NULL, "|");
        }
        
        if (fieldIdx >= 4) {
            // 查找对应的单词
            for (int i = 0; i < g_wordProgressCount; i++) {
                if (strcmp(g_words[i].entry.word, fields[0]) == 0) {
                    g_words[i].progress.knownCount = atoi(fields[1]);
                    g_words[i].progress.unknownCount = atoi(fields[2]);
                    g_words[i].progress.lastReview = (time_t)atol(fields[3]);
                    // 如果认识3次以上，标记为已掌握
                    if (g_words[i].progress.knownCount >= 3) {
                        g_words[i].progress.mastered = true;
                    }
                    break;
                }
            }
        }
    }
    
    fclose(fp);
    printf("INFO: Progress loaded from %s\n", g_progressFilePath);
}

// 清除所有学习进度
// 重置所有单词的认识/不认识次数、上次复习时间、掌握状态
// 用于"清除进度"按钮
void clearProgress(void) {
    for (int i = 0; i < g_wordProgressCount; i++) {
        g_words[i].progress.knownCount = 0;
        g_words[i].progress.unknownCount = 0;
        g_words[i].progress.lastReview = 0;
        g_words[i].progress.mastered = false;
    }
    saveProgress();
    printf("INFO: Progress cleared\n");
}

// ============================================================================
// 单词搜索功能（支持正则表达式）
// ============================================================================

#include <regex.h>

/**
 * 使用正则表达式搜索单词
 * @param pattern 搜索模式（支持正则表达式）
 * @param results 存放搜索结果索引的数组
 * @param maxResults 最大结果数量
 * @return 匹配到的结果数量
 */
int searchWordsByRegex(const char* pattern, int* results, int maxResults) {
    if (pattern == NULL || strlen(pattern) == 0) {
        return 0;
    }

    regex_t regex;
    int ret;
    int count = 0;

    // 编译正则表达式（不区分大小写）
    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
    if (ret != 0) {
        // 正则表达式编译失败，尝试简单匹配
        for (int i = 0; i < g_wordProgressCount && count < maxResults; i++) {
            if (strstr(g_words[i].entry.word, pattern) != NULL) {
                results[count++] = i;
            }
        }
        return count;
    }

    // 搜索所有单词
    for (int i = 0; i < g_wordProgressCount && count < maxResults; i++) {
        ret = regexec(&regex, g_words[i].entry.word, 0, NULL, 0);
        if (ret == 0) {
            results[count++] = i;
        }
    }

    regfree(&regex);
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

bool saveWordsToFile(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("WARNING: Cannot save words to %s\n", filename);
        return false;
    }
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
 * 参数检查通过后在 g_wordLibrary 末尾追加新条目，
 * 所有字段都通过 strdup 复制，确保内存独立。
 * 添加后自动保存到 words.txt 文件。
 * 注意：添加后需调用 reloadWords() 同步到 g_words 进度数组。
 * 
 * @param w 单词（必填，不能为空）
 * @param ph 音标（可选，为空时存空字符串）
 * @param def 释义（可选）
 * @param ex 例句（可选）
 * @param ext 例句翻译（可选）
 * @return true 添加成功，false 添加失败（单词为空或已到上限）
 */
bool addWordToLibrary(const char* w, const char* ph, const char* def,
                      const char* ex, const char* ext) {
    if (w == NULL || strlen(w) == 0) return false;
    if (g_wordCount >= MAX_WORDS + 200) return false;
    if (g_wordCount >= g_wordLibraryCapacity) {
        g_wordLibraryCapacity = g_wordLibraryCapacity == 0 ? 50 : g_wordLibraryCapacity * 2;
        g_wordLibrary = (WordEntry*)realloc(g_wordLibrary, g_wordLibraryCapacity * sizeof(WordEntry));
    }
    g_wordLibrary[g_wordCount].word = strdup(w);
    g_wordLibrary[g_wordCount].phonetic = strdup(ph ? ph : "");
    g_wordLibrary[g_wordCount].definition = strdup(def ? def : "");
    g_wordLibrary[g_wordCount].example = strdup(ex ? ex : "");
    g_wordLibrary[g_wordCount].exampleTranslation = strdup(ext ? ext : "");
    g_wordCount++;
    saveWordsToFile(WORDS_FILE_PATH);
    return true;
}

/**
 * editWordInLibrary - 编辑词库中指定索引的单词
 * 
 * 先释放原有字段的内存，再通过 strdup 复制新内容。
 * 编辑后自动保存到 words.txt 文件并需调用 reloadWords() 同步。
 * 
 * @param idx 要编辑的单词索引（0 ~ g_wordCount-1）
 * @param w 新单词内容（必填）
 * @param ph 新音标
 * @param def 新释义
 * @param ex 新例句
 * @param ext 新例句翻译
 * @return true 编辑成功，false 编辑失败（索引无效或单词为空）
 */
bool editWordInLibrary(int idx, const char* w, const char* ph,
                       const char* def, const char* ex, const char* ext) {
    if (idx < 0 || idx >= g_wordCount || w == NULL || strlen(w) == 0) return false;
    free((void*)g_wordLibrary[idx].word);
    free((void*)g_wordLibrary[idx].phonetic);
    free((void*)g_wordLibrary[idx].definition);
    free((void*)g_wordLibrary[idx].example);
    free((void*)g_wordLibrary[idx].exampleTranslation);
    g_wordLibrary[idx].word = strdup(w);
    g_wordLibrary[idx].phonetic = strdup(ph ? ph : "");
    g_wordLibrary[idx].definition = strdup(def ? def : "");
    g_wordLibrary[idx].example = strdup(ex ? ex : "");
    g_wordLibrary[idx].exampleTranslation = strdup(ext ? ext : "");
    saveWordsToFile(WORDS_FILE_PATH);
    return true;
}

/**
 * deleteWordFromLibrary - 从词库删除指定索引的单词
 * 
 * 释放该单词的所有字段内存，将后续单词前移填补空缺，
 * 最后递减 g_wordCount。删除后自动保存到 words.txt。
 * 
 * @param idx 要删除的单词索引
 * @return true 删除成功，false 索引无效
 */
bool deleteWordFromLibrary(int idx) {
    if (idx < 0 || idx >= g_wordCount) return false;
    free((void*)g_wordLibrary[idx].word);
    free((void*)g_wordLibrary[idx].phonetic);
    free((void*)g_wordLibrary[idx].definition);
    free((void*)g_wordLibrary[idx].example);
    free((void*)g_wordLibrary[idx].exampleTranslation);
    for (int i = idx; i < g_wordCount - 1; i++) g_wordLibrary[i] = g_wordLibrary[i + 1];
    g_wordCount--;
    saveWordsToFile(WORDS_FILE_PATH);
    return true;
}

/**
 * reloadWords - 重新加载 g_words 进度数组（词库修改后调用）
 * 
 * 将 g_wordLibrary 的最新数据同步到 g_words 数组。
 * 对于新增索引（i >= oldCnt）初始化进度为默认值，
 * 对于已有索引保留原有的 knownCount 等进度数据。
 * 
 * 在词库管理页面的添加/编辑/删除操作后调用。
 */
void reloadWords(void) {
    int oldCnt = g_wordProgressCount;
    g_wordProgressCount = 0;
    for (int i = 0; i < g_wordCount && i < MAX_WORDS; i++) {
        g_words[i].entry = g_wordLibrary[i];
        g_words[i].progress.wordIndex = i;
        if (i >= oldCnt) {
            g_words[i].progress.knownCount = 0;
            g_words[i].progress.unknownCount = 0;
            g_words[i].progress.lastReview = 0;
            g_words[i].progress.mastered = false;
        }
        g_wordProgressCount++;
    }
    printf("INFO: Reloaded %d words\n", g_wordProgressCount);
}
