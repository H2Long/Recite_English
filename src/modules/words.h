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

/**
 * g_words[MAX_WORDS] - 所有单词数据（含学习进度）
 * 
 * 这是程序核心数据：从 g_wordLibrary 复制而来，关联了每个单词的学习进度。
 * g_wordProgressCount 表示实际有效的单词数。
 * 所有学习模式（学单词、背单词、测试）都基于此数组操作。
 */
extern WordWithProgress g_words[MAX_WORDS];

/**
 * g_wordLibrary - 单词库动态数组（WordEntry 指针）
 * 
 * 从 words.txt 加载的原始单词数据，支持运行时增删改。
 * g_wordLibrary 是动态分配的内存，通过 g_wordCount 跟踪有效数量。
 * 词库修改后需调用 reloadWords() 同步到 g_words 数组。
 */
extern WordEntry* g_wordLibrary;

/**
 * g_wordProgressCount - 已加载到 g_words 的单词数量
 * 
 * 即 g_words 数组中实际有效的单词个数。
 * 用于遍历单词列表、生成题目等场景。
 * 注意此值 ≤ g_wordCount 且 ≤ MAX_WORDS。
 */
extern int g_wordProgressCount;

/**
 * g_wordCount - 单词库中单词的总数
 * 
 * 即 g_wordLibrary 中的单词数量，可能大于 g_wordProgressCount
 * （当词库超过 MAX_WORDS 时，g_wordProgressCount 最多为 MAX_WORDS）。
 */
extern int g_wordCount;

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
 * setProgressFilePath - 设置学习进度文件路径（用于多用户切换）
 * 
 * 不同用户登录后调用此函数切换到该用户专属的进度文件，
 * 例如 "progress_hhlong.txt"。切换后需调用 loadProgress() 加载数据。
 * 未登录时使用默认的 "progress.txt"。
 * 
 * @param path 进度文件路径（如 "./data/progress_hhlong.txt"）
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

// ============================================================================
// 词库管理函数
// ============================================================================

/**
 * 保存单词库到文件
 * 写入 words.txt 格式：word|phonetic|definition|example|exampleTranslation
 * @param filename 单词文件路径
 * @return true 保存成功
 */
bool saveWordsToFile(const char* filename);

/**
 * 添加新单词到词库
 */
bool addWordToLibrary(const char* word, const char* phonetic, const char* definition,
                      const char* example, const char* exampleTranslation);

/**
 * 编辑词库中的单词
 * @param index 单词索引
 */
bool editWordInLibrary(int index, const char* word, const char* phonetic,
                       const char* definition, const char* example,
                       const char* exampleTranslation);

/**
 * 删除词库中的单词
 * @param index 要删除的单词索引
 */
bool deleteWordFromLibrary(int index);

/**
 * 重新加载 g_words 进度数组（词库修改后调用）
 */
void reloadWords(void);

#endif // WORDS_H
