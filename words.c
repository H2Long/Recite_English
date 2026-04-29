// ============================================================================
// 单词管理模块
// 功能：单词加载、进度管理、持久化存储
// ============================================================================

#include "words.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

// 动态单词库（支持从文件加载）
static WordEntry* g_wordLibrary = NULL;
static int g_wordLibraryCapacity = 0;

// 单词数据（含学习进度）
WordWithProgress g_words[MAX_WORDS];
int g_wordProgressCount = 0;
int g_wordCount = 0;

// 学单词模式滚动偏移
float g_learnScrollOffset = 0;

// 进度文件路径（用于持久化存储学习进度）
static const char* g_progressFile = "./progress.txt";

// Fisher-Yates 洗牌算法：随机打乱数组顺序
void shuffleArray(int *array, int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// 从文件加载单词
// 文件格式：word|phonetic|definition|example（每行一个单词）
// 如果文件不存在，使用内置的10个默认单词
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

// 释放单词库内存
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

// 初始化单词数据
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
    FILE* fp = fopen(g_progressFile, "w");
    if (fp == NULL) {
        printf("WARNING: Cannot save progress to: %s\n", g_progressFile);
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
    printf("INFO: Progress saved to %s\n", g_progressFile);
}

// 从文件加载学习进度
// 程序启动时调用，恢复之前的学习状态
// 认识次数>=3的单词自动标记为"已掌握"
void loadProgress(void) {
    FILE* fp = fopen(g_progressFile, "r");
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
    printf("INFO: Progress loaded from %s\n", g_progressFile);
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
