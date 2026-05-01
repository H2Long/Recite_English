// 单词管理 — 词库加载/进度持久化/通配符搜索/CRUD

#ifndef WORDS_H
#define WORDS_H

#include "raylib.h"
#include "raylib_word_ui.h"
#include <time.h>

// 单词学习进度
typedef struct {
    int wordIndex;
    int knownCount;           // 认识次数
    int unknownCount;         // 不认识次数
    time_t lastReview;        // 上次复习时间戳
    bool mastered;            // 是否已掌握(knownCount>=3)
} WordProgress;

// 单词 + 进度
typedef struct {
    WordEntry entry;
    WordProgress progress;
} WordWithProgress;

#define MAX_WORDS 100

extern WordWithProgress g_words[MAX_WORDS];
extern WordEntry* g_wordLibrary;
extern int g_wordProgressCount;
extern int g_wordCount;

void loadWordsFromFile(const char* filename);
void freeWordLibrary(void);
void initWords(void);
void shuffleArray(int *array, int count);
void setProgressFilePath(const char* path);
void saveProgress(void);
void loadProgress(void);
void clearProgress(void);

int searchWordsByRegex(const char* pattern, int* results, int maxResults);
int searchWordsSimple(const char* query, int* results, int maxResults);

bool saveWordsToFile(const char* filename);
bool addWordToLibrary(const char* word, const char* phonetic, const char* definition,
                      const char* example, const char* exampleTranslation);
bool editWordInLibrary(int index, const char* word, const char* phonetic,
                       const char* definition, const char* example,
                       const char* exampleTranslation);
bool deleteWordFromLibrary(int index);
void reloadWords(void);

#endif
