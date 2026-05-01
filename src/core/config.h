// 项目常量配置 — 窗口/颜色/学习参数/UI布局/文件路径

#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"

// 窗口
#define SCREEN_WIDTH  1600
#define SCREEN_HEIGHT 1000
#define WINDOW_TITLE  u8"背单词软件"

// 学习参数
#define MASTERED_THRESHOLD  3     // 认识几次算掌握
#define DAILY_GOAL_DEFAULT  20    // 默认每日目标
#define MAX_WRONG_COUNT     5     // 错误几次后标记
#define MAX_WORDS           100   // g_words 最大容量

// 测试/选项
#define TEST_MAX_QUESTIONS  10
#define OPTIONS_COUNT       4

// 文件路径
#define WORDS_FILE_PATH     "./data/words.txt"
#define PROGRESS_FILE_PATH  "./data/progress.txt"

#endif
