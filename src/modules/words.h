// 单词管理 — 词库加载/进度持久化/通配符搜索/CRUD

#ifndef WORDS_H
#define WORDS_H

#include "raylib.h"
#include "raylib_word_ui.h"
#include "config.h"
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

extern WordWithProgress g_words[MAX_WORDS];
extern WordEntry* g_wordLibrary;
extern int g_wordProgressCount;
extern int g_wordCount;

void load_words_from_file(const char* filename);
void free_word_library(void);
void init_words(void);
void shuffle_array(int *array, int count);
void set_progress_file_path(const char* path);
void save_progress(void);
void load_progress(void);
void clear_progress(void);

int search_words_by_regex(const char* pattern, int* results, int maxResults);
int search_words_simple(const char* query, int* results, int maxResults);

bool save_words_to_file(const char* filename);
bool add_word_to_library(const char* word, const char* phonetic, const char* definition,
                      const char* example, const char* exampleTranslation);
bool edit_word_in_library(int index, const char* word, const char* phonetic,
                       const char* definition, const char* example,
                       const char* exampleTranslation);
bool delete_word_from_library(int index);
void reload_words(void);

#endif
