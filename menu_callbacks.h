// ============================================================================
// 菜单回调函数头文件
// 功能：声明各菜单页面的显示函数和全局变量
// ============================================================================

#ifndef MENU_CALLBACKS_H
#define MENU_CALLBACKS_H

#include "raylib.h"
#include "raylib_word_ui.h"
#include "tree_menu.h"
#include "words.h"
#include "fonts.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// 窗口尺寸宏定义
// 注意：修改这些值会影响所有页面的布局
// ============================================================================

#define SCREEN_WIDTH 1600   // 窗口宽度（像素）
#define SCREEN_HEIGHT 1000  // 窗口高度（像素）

// ============================================================================
// 外部全局变量声明（由 main.c 提供）
// ============================================================================

// UI 样式和状态
extern UIStyle g_style;       // UI 主题样式（颜色、字体大小等）
extern UIState g_uiState;     // UI 交互状态（鼠标位置、按键状态等）

// 菜单系统
extern MENU* g_rootMenu;       // 根菜单节点
extern MENU* g_currentMenu;     // 当前显示的菜单
extern MenuStack g_menuStack;   // 菜单栈（用于"返回"功能）

// 学单词模式状态
extern int g_learnIndex;               // 当前选中的单词索引
extern bool g_learnFilterUnknown;      // 是否只显示未掌握的单词
extern float g_learnScrollOffset;      // 单词列表滚动位置

// 背单词模式（闪卡）状态
extern int g_reviewIndices[MAX_WORDS];        // 复习单词的索引数组
extern int g_reviewCount;                     // 待复习单词数量
extern int g_currentReviewIdx;                // 当前复习到第几个
extern CardFace g_flashcardFace;              // 闪卡当前显示正面还是背面
extern float g_flashcardAnimTime;             // 闪卡翻转动画时间
extern int g_knownInSession;                  // 本轮认识的数量
extern int g_unknownInSession;                 // 本轮不认识的的数量

// 测试模式状态
extern int g_testIndices[MAX_WORDS];           // 测试单词的索引数组
extern int g_testCount;                        // 测试题总数
extern int g_currentTestIdx;                   // 当前测试到第几题
extern int g_testCorrect;                      // 正确答案数
extern int g_testTotal;                        // 已答题总数
extern int g_selectedAnswer;                   // 用户选择的答案
extern int g_answerResult;                     // 答题结果（1=正确，0=错误，-1=未答）
extern int g_currentCorrectIdx;                // 正确答案的选项索引
extern bool g_wrongOptionsUsed[MAX_WORDS];     // 标记已使用的错误选项

// ============================================================================
// 菜单回调函数声明
// ============================================================================

/**
 * 主菜单显示
 * 包含欢迎信息、统计信息和功能卡片入口
 */
void MenuHome_Show(void);

/**
 * 学单词模式
 * 左侧：可滚动的单词列表
 * 右侧：选中单词的详细信息（单词、音标、释义、例句）
 */
void MenuLearn_Show(void);

/**
 * 背单词模式
 * 使用闪卡翻转学习：正面显示单词，背面显示释义
 * 认识3次后标记为"已掌握"
 */
void MenuReview_Show(void);

/**
 * 测试模式
 * 选择题测试：显示单词，从4个选项中选择正确释义
 * 测试完成后显示正确率和评价
 */
void MenuTest_Show(void);

/**
 * 进度模式
 * 显示学习统计和每个单词的详细进度
 */
void MenuProgress_Show(void);

// ============================================================================
// 菜单系统辅助函数
// ============================================================================

/**
 * 初始化菜单树
 * 创建所有菜单节点并建立父子关系
 */
void InitMenuTree(void);

/**
 * 释放菜单树内存
 */
void FreeMenuTree(void);

/**
 * 获取菜单项文本
 * 根据菜单的显示函数确定菜单名称
 * @param menu 菜单节点
 * @return 菜单显示名称
 */
const char* GetMenuItemText(MENU* menu);

/**
 * 绘制树形菜单（左侧导航栏）
 * @param menuRect 菜单区域
 */
void DrawTreeMenu(Rectangle menuRect);

#endif // MENU_CALLBACKS_H
