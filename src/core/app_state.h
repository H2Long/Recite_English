// ============================================================================
// 统一状态管理模块
// 功能：集中管理所有全局状态变量，解决变量分散管理的问题
// ============================================================================

#ifndef APP_STATE_H
#define APP_STATE_H

#include "raylib.h"
#include "raylib_word_ui.h"
#include "tree_menu.h"
#include "words.h"
#include "account.h"
#include "plan.h"
#include "config.h"

// ============================================================================
// 子状态结构体定义
// ============================================================================

// 学单词模式状态
typedef struct {
    int learnIndex;              // 当前选中的单词索引
    bool learnFilterUnknown;     // 是否只显示未掌握的单词
    float learnScrollOffset;     // 单词列表滚动位置
} LearnState;

// 背单词模式（闪卡）状态
typedef struct {
    int reviewIndices[MAX_WORDS];    // 复习单词的索引数组
    int reviewCount;                  // 待复习单词数量
    int currentReviewIdx;             // 当前复习到第几个
    CardFace flashcardFace;           // 闪卡当前显示正面还是背面
    float flashcardAnimTime;         // 闪卡翻转动画时间
    int knownInSession;              // 本轮认识的数量
    int unknownInSession;            // 本轮不认识的的数量
} ReviewState;

// 测试模式状态
typedef struct {
    int testIndices[MAX_WORDS];      // 测试单词的索引数组
    int testCount;                    // 测试题总数
    int currentTestIdx;               // 当前测试到第几题
    int testCorrect;                  // 正确答案数
    int testTotal;                    // 已答题总数
    int selectedAnswer;              // 用户选择的答案
    int answerResult;                // 答题结果（1=正确，0=错误，-1=未答）
    int currentCorrectIdx;           // 正确答案的选项索引
    bool wrongOptionsUsed[MAX_WORDS]; // 标记已使用的错误选项
} TestState;

// 查找单词模式状态
typedef struct {
    char searchInput[256];           // 搜索输入文本
    int searchResults[MAX_WORDS];    // 搜索结果索引数组
    int searchResultCount;           // 搜索结果数量
    SearchBarState searchBar;        // 搜索栏状态
} SearchState;

// 选词背单词模式状态（看释义选单词）
typedef struct {
    int selectIndices[MAX_WORDS];    // 单词索引数组
    int selectCount;                  // 题目总数
    int currentSelectIdx;             // 当前题目索引
    int selectCorrect;               // 正确数
    int selectTotal;                 // 已答题总数
    int selectedAnswer;             // 用户选择的答案
    int answerResult;               // 答题结果（1正确0错误-1未答）
    int currentCorrectIdx;          // 正确答案选项索引
    bool wrongOptionsUsed[MAX_WORDS]; // 已用错误选项
} SelectWordState;

// ============================================================================
// 应用状态结构体（统一管理所有状态）
// ============================================================================

typedef struct {
    // UI 样式
    UIStyle style;
    
    // UI 交互状态
    UIState uiState;
    
    // 主题设置
    bool isDarkMode;          // 是否为深色模式
    
    // 菜单系统
    MENU* rootMenu;          // 根菜单节点
    MENU* currentMenu;       // 当前显示的菜单
    MenuStack menuStack;     // 菜单栈（用于"返回"功能）
    
    // 各模式状态
    LearnState learn;       // 学单词模式状态
    ReviewState review;      // 背单词模式状态（卡片）
    TestState test;         // 测试模式状态
    SelectWordState selectWord; // 选词背单词模式状态
    SearchState search;     // 查找单词模式状态
    
    // 账号系统
    AccountState account;   // 账号管理状态
    char loginMsg[128];      // 登录/注册提示信息
    
    // 学习计划
    PlanState plan;          // 学习计划状态
    
    // 登录强制
    bool loginRequired;      // 是否要求登录才能使用
} AppState;

// ============================================================================
// 全局状态实例
// ============================================================================

extern AppState g_app;

// ============================================================================
// 状态管理函数
// ============================================================================

// 初始化应用状态
void AppState_Init(void);

// 重置各模式状态（不清除菜单树）
void AppState_Reset(void);

// 释放应用状态相关内存
void AppState_Deinit(void);

// ============================================================================
// 状态访问器函数（供 menu_callbacks.c 使用）
// ============================================================================

// 获取 UI 样式指针
UIStyle* AppState_GetStyle(void);

// 获取 UI 状态指针
UIState* AppState_GetUIState(void);

// 获取当前菜单指针
MENU** AppState_GetCurrentMenu(void);

// 获取菜单栈指针
MenuStack* AppState_GetMenuStack(void);

// 获取学单词状态指针
LearnState* AppState_GetLearnState(void);

// 获取背单词状态指针
ReviewState* AppState_GetReviewState(void);

// 获取测试状态指针
TestState* AppState_GetTestState(void);

// 获取查找单词状态指针
SearchState* AppState_GetSearchState(void);

// 获取选词背单词状态指针
SelectWordState* AppState_GetSelectWordState(void);

// 获取账号状态指针
AccountState* AppState_GetAccountState(void);

// 获取登录提示信息
char* AppState_GetLoginMsg(void);

// 获取学习计划状态指针
PlanState* AppState_GetPlanState(void);

// 获取/设置深色模式
bool AppState_IsDarkMode(void);              // 获取当前是否为深色模式
void AppState_ToggleDarkMode(void);          // 切换深色/浅色模式
void AppState_SetDarkMode(bool isDark);     // 设置深色模式

#endif // APP_STATE_H
