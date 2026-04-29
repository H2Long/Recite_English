// ============================================================================
// 单词管理头文件
// 功能：定义单词数据结构、学习进度结构和单词管理接口
// ============================================================================

#ifndef WORDS_H
#define WORDS_H

#include "raylib.h"
#include "raylib_word_ui.h"
#include <time.h>

// ============================================================================
// 数据结构
// ============================================================================

/**
 * 单词学习进度
 * 记录用户对每个单词的学习情况
 */
typedef struct {
    int wordIndex;         // 单词在数组中的索引
    int knownCount;        // 认识次数（点击"认识"按钮累加）
    int unknownCount;      // 不认识次数（点击"不认识"按钮累加）
    time_t lastReview;     // 上次复习时间戳（0表示从未复习）
    bool mastered;         // 是否已掌握（认识次数>=3时为true）
} WordProgress;

/**
 * 单词条目 + 学习进度
 * 组合了单词本身的信息和学习进度
 */
typedef struct {
    WordEntry entry;       // 单词信息（单词、音标、释义、例句）
    WordProgress progress; // 学习进度
} WordWithProgress;

// 最大单词数限制
#define MAX_WORDS 100

// ============================================================================
// 外部变量声明（由 words.c 提供）
// ============================================================================

extern WordWithProgress g_words[MAX_WORDS];      // 所有单词数据（含进度）
extern int g_wordProgressCount;                  // 已加载的单词数量
extern int g_wordCount;                          // 单词库总数（可能大于已加载数）

// ============================================================================
// 函数声明
// ============================================================================

/**
 * 从文件加载单词
 * 文件格式：word|phonetic|definition|example（每行一个单词）
 * @param filename 单词文件路径
 */
void loadWordsFromFile(const char* filename);

/**
 * 释放单词库内存
 */
void freeWordLibrary(void);

/**
 * 初始化单词数据
 * 分配学习进度并设置默认值
 */
void initWords(void);

/**
 * Fisher-Yates 洗牌算法
 * 随机打乱数组顺序（用于随机出题）
 * @param array 目标数组
 * @param count 数组长度
 */
void shuffleArray(int *array, int count);

/**
 * 设置进度文件路径（用于多用户切换）
 * @param path 进度文件路径
 */
void setProgressFilePath(const char* path);

/**
 * 保存学习进度到文件
 */
void saveProgress(void);

/**
 * 从文件加载学习进度
 */
void loadProgress(void);

/**
 * 清除所有学习进度
 * 重置所有单词的进度为初始状态
 */
void clearProgress(void);

/**
 * 使用正则表达式搜索单词
 * @param pattern 搜索模式（支持正则表达式）
 * @param results 存放搜索结果索引的数组
 * @param maxResults 最大结果数量
 * @return 匹配到的结果数量
 */
int searchWordsByRegex(const char* pattern, int* results, int maxResults);

/**
 * 简单的模糊搜索（不区分大小写，包含匹配）
 * @param query 搜索关键词
 * @param results 存放搜索结果索引的数组
 * @param maxResults 最大结果数量
 * @return 匹配到的结果数量
 */
int searchWordsSimple(const char* query, int* results, int maxResults);

#endif // WORDS_H
