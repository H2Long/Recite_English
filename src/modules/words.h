// ============================================================================
// 单词管理头文件
// 功能：定义单词数据结构（WordEntry/WordProgress/WordWithProgress）、
//       外部全局变量声明、所有单词管理函数声明
//
// 核心数据流：
//   words.txt ──→ g_wordLibrary (WordEntry* 动态数组，支持增删改)
//                       │
//                       ▼
//                 g_words[100] (WordWithProgress 定长数组，含学习进度)
//                       │
//   progress.txt ←── saveProgress() / loadProgress()
//
// 注意：g_words[i].entry 是 g_wordLibrary[i] 的浅拷贝（结构体赋值），
//       两者共享 WordEntry 中的 char* 指针，修改词库后需调用 reloadWords()。
// ============================================================================

#ifndef WORDS_H
#define WORDS_H

#include "raylib.h"        /* Vector2/Font/Color 等 raylib 类型 */
#include "raylib_word_ui.h" /* WordEntry 类型定义 */
#include <time.h>          /* time_t 类型 */

// ============================================================================
// 数据结构
// ============================================================================

/**
 * WordProgress - 单词学习进度
 *
 * 记录用户对单个单词的学习情况。
 * 当 knownCount >= MASTERED_THRESHOLD（3）时 mastered = true。
 */
typedef struct {
    int wordIndex;         /* 单词在 g_words 数组中的索引（与 entry 对应） */
    int knownCount;        /* 认识次数：点击"认识"按钮累加 */
    int unknownCount;      /* 不认识次数：点击"不认识"按钮累加 */
    time_t lastReview;     /* 上次复习时间戳（Unix 秒数，0=从未复习） */
    bool mastered;         /* 是否已掌握：knownCount >= 3 时为 true */
} WordProgress;

/**
 * WordWithProgress - 单词条目 + 学习进度的组合结构
 *
 * 这是 g_words[MAX_WORDS] 数组的元素类型。
 * entry 字段是 g_wordLibrary 中对应条目的浅拷贝（结构体赋值）。
 * progress 字段独立管理，由 saveProgress/loadProgress 持久化。
 */
typedef struct {
    WordEntry entry;       /* 单词信息（word/phonetic/definition/example 等） */
    WordProgress progress; /* 学习进度（认识次数/掌握状态/上次复习时间） */
} WordWithProgress;

// ============================================================================
// 常量
// ============================================================================

#define MAX_WORDS 100  /* g_words 数组的最大容量。词库超过此数时会被截断。 */

// ============================================================================
// 外部变量声明（由 words.c 提供）
// ============================================================================

extern WordWithProgress g_words[MAX_WORDS];  /* 所有单词数据（含进度），核心数据 */
extern WordEntry* g_wordLibrary;              /* 单词库动态数组（支持增删改） */
extern int g_wordProgressCount;               /* g_words 中有效单词数 */
extern int g_wordCount;                       /* g_wordLibrary 中单词总数 */

// ============================================================================
// 函数声明
// ============================================================================

void loadWordsFromFile(const char* filename);   /* 从 words.txt 加载词库 */
void freeWordLibrary(void);                     /* 释放 g_wordLibrary 内存 */
void initWords(void);                           /* 初始化 g_words 进度数据 */
void shuffleArray(int *array, int count);       /* Fisher-Yates 洗牌 */
void setProgressFilePath(const char* path);     /* 设置进度文件路径（多用户切换） */
void saveProgress(void);                        /* 保存学习进度到文件 */
void loadProgress(void);                        /* 从文件恢复学习进度 */
void clearProgress(void);                       /* 清除所有学习进度 */

int searchWordsByRegex(const char* pattern, int* results, int maxResults);  /* 通配符模式搜索 */
int searchWordsSimple(const char* query, int* results, int maxResults);     /* 子串模糊搜索 */

bool saveWordsToFile(const char* filename);                                          /* 保存词库 */
bool addWordToLibrary(const char* word, const char* phonetic, const char* definition,  /* 添加单词 */
                      const char* example, const char* exampleTranslation);
bool editWordInLibrary(int index, const char* word, const char* phonetic,              /* 编辑单词 */
                       const char* definition, const char* example,
                       const char* exampleTranslation);
bool deleteWordFromLibrary(int index);        /* 删除单词 */
void reloadWords(void);                        /* 词库修改后同步到 g_words */

#endif // WORDS_H
