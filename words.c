// ============================================================================
// 单词管理模块
// 功能：单词加载、进度管理、持久化存储
// ============================================================================

#include "words.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        const char* defaultWords[][4] = {
            {"abandon", "[ə'bændən]", "v. 放弃；抛弃", "Never abandon your dreams."},
            {"ability", "[ə'bɪləti]", "n. 能力", "She has the ability to speak three languages."},
            {"absence", "['æbsəns]", "n. 缺席；缺乏", "His absence was noticed by everyone."},
            {"absolute", "['æbsəluːt]", "adj. 绝对的", "It is an absolute necessity."},
            {"absorb", "[əb'zɔːrb]", "v. 吸收；理解", "Plants absorb water from the soil."},
            {"abstract", "['æbstrækt]", "adj. 抽象的", "The concept is too abstract."},
            {"abundant", "[ə'bʌndənt]", "adj. 丰富的", "We have abundant resources."},
            {"academic", "[ˌækə'demɪk]", "adj. 学术的", "She has excellent academic records."},
            {"accelerate", "[ək'seləreɪt]", "v. 加速", "The car began to accelerate."},
            {"accomplish", "[ə'kʌmplɪʃ]", "v. 完成", "We accomplished our mission."},
        };
        int defaultCount = sizeof(defaultWords) / sizeof(defaultWords[0]);
        g_wordLibraryCapacity = defaultCount + 10;
        g_wordLibrary = (WordEntry*)malloc(g_wordLibraryCapacity * sizeof(WordEntry));
        for (int i = 0; i < defaultCount; i++) {
            g_wordLibrary[i].word = strdup(defaultWords[i][0]);
            g_wordLibrary[i].phonetic = strdup(defaultWords[i][1]);
            g_wordLibrary[i].definition = strdup(defaultWords[i][2]);
            g_wordLibrary[i].example = strdup(defaultWords[i][3]);
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

        // 解析格式: word|phonetic|definition|example
        char* fields[4] = {NULL, NULL, NULL, NULL};
        int fieldIdx = 0;
        char* token = strtok(line, "|");
        while (token != NULL && fieldIdx < 4) {
            fields[fieldIdx++] = strdup(token);
            token = strtok(NULL, "|");
        }

        if (fieldIdx >= 3) {
            g_wordLibrary[g_wordCount].word = fields[0];
            g_wordLibrary[g_wordCount].phonetic = fields[1];
            g_wordLibrary[g_wordCount].definition = fields[2];
            g_wordLibrary[g_wordCount].example = fieldIdx >= 4 ? fields[3] : "";
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
