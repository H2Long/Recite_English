// ============================================================================
// 统一状态管理模块实现
// 功能：管理全局 AppState 实例的生命周期，提供子状态访问器
//
// 设计决策：
//   AppState 是一个全局单例结构体（g_app），包含所有模块的状态。
//   访问器函数（AppState_GetXXX）返回子结构的指针，允许直接读写字段。
//   这种方式优于"setter/getter"模式，因为：
//   1. 避免为每个字段写单独的 get/set 函数（太多样板代码）
//   2. 调用方可以通过指针直接访问嵌套字段（如 LEARN.learnIndex）
//   3. 通过宏（STYLE / LEARN 等）进一步简化使用
// ============================================================================

#include "app_state.h"
#include "account.h"    /* Account_SetState 绑定账号状态 */
#include <string.h>     /* memset */
#include <stdlib.h>     /* free */

// ============================================================================
// 全局状态实例
// ============================================================================

/*
 * g_app — 唯一的全局 AppState 实例。
 * 使用 {0} 初始化确保所有字段为零值。
 * 所有模块（account / plan / learn / review 等）的状态都嵌在此结构体中。
 * 通过 AppState_GetXXX() 访问器获取各个子状态的指针。
 */
AppState g_app = {0};

// ============================================================================
// 状态管理函数
// ============================================================================

/**
 * AppState_Init - 初始化所有子状态为默认值
 *
 * 在 main() 的初始化阶段调用，执行顺序：
 *   1. 初始化 UI 样式（浅色主题 + 默认字体 + 默认字号）
 *   2. 初始化菜单系统（rootMenu = NULL, currentMenu = NULL, 栈清空）
 *   3. 初始化学习模式状态（learn / review / test / search / selectWord 全清零）
 *   4. 绑定 account 和 plan 模块的外部状态指针
 *
 * 注意：菜单树的实际创建在 InitMenuTree() 中完成，不在本函数中。
 */
void AppState_Init(void) {
    /* ---- 1. UI 样式初始化 ---- */
    /* 使用 UIStyleInit 填充默认值（浅色主题、默认字体、三档字号） */
    UIStyleInit(&g_app.style);

    /* ---- 2. 菜单系统初始化 ---- */
    g_app.rootMenu = NULL;          /* 根菜单节点（稍后由 InitMenuTree 创建） */
    g_app.currentMenu = NULL;       /* 当前页面（初始化时为空） */
    StackInit(&g_app.menuStack);    /* 清空导航栈 */

    /* ---- 3. 各学习模式状态初始化 ---- */
    /* 学单词：从第一个单词开始，不过滤，滚动在顶部 */
    g_app.learn.learnIndex = 0;
    g_app.learn.learnFilterUnknown = false;
    g_app.learn.learnScrollOffset = 0.0f;

    /* 背单词：无复习列表，正面朝上，动画归零，会话统计归零 */
    g_app.review.reviewCount = 0;
    g_app.review.currentReviewIdx = 0;
    g_app.review.flashcardFace = CARD_FRONT;
    g_app.review.flashcardAnimTime = 0.0f;
    g_app.review.knownInSession = 0;
    g_app.review.unknownInSession = 0;

    /* 测试：无题目，归零统计，未选择答案 */
    g_app.test.testCount = 0;
    g_app.test.currentTestIdx = 0;
    g_app.test.testCorrect = 0;
    g_app.test.testTotal = 0;
    g_app.test.selectedAnswer = -1;      /* -1 表示未选择 */
    g_app.test.answerResult = -1;         /* -1 表示未答题 */
    g_app.test.currentCorrectIdx = 0;
    /* wrongOptionsUsed 标记哪些单词已被用作干扰选项 */
    memset(g_app.test.wrongOptionsUsed, 0, sizeof(g_app.test.wrongOptionsUsed));

    /* 查找单词：清空搜索输入，无结果 */
    memset(g_app.search.searchInput, 0, sizeof(g_app.search.searchInput));
    g_app.search.searchResultCount = 0;
    memset(&g_app.search.searchBar, 0, sizeof(g_app.search.searchBar));

    /* 选词背单词：无题目，归零统计（结构与测试模式类似） */
    g_app.selectWord.selectCount = 0;
    g_app.selectWord.currentSelectIdx = 0;
    g_app.selectWord.selectCorrect = 0;
    g_app.selectWord.selectTotal = 0;
    g_app.selectWord.selectedAnswer = -1;
    g_app.selectWord.answerResult = -1;
    g_app.selectWord.currentCorrectIdx = 0;
    memset(g_app.selectWord.wrongOptionsUsed, 0, sizeof(g_app.selectWord.wrongOptionsUsed));

    /* ---- 4. 绑定独立模块的状态指针 ---- */
    /*
     * Account_SetState(&g_app.account) 让 account 模块内部的 g_pState
     * 指向 &g_app.account。此后 account 模块的 getState() 返回 &g_app.account，
     * 而不是内部备用状态。
     * Plan_SetState(&g_app.plan) 同理。
     */
    Account_SetState(&g_app.account);
    Plan_SetState(&g_app.plan);
}

/**
 * AppState_Reset - 重置各学习模式状态（不清除菜单树和账号）
 *
 * 用于"重新开始"场景，仅重置学习进度相关的状态。
 * 不影响：菜单树、账号登录状态、UI 主题样式。
 */
void AppState_Reset(void) {
    /* 学单词重置 */
    g_app.learn.learnIndex = 0;
    g_app.learn.learnFilterUnknown = false;
    g_app.learn.learnScrollOffset = 0.0f;

    /* 背单词重置（只重置会话状态，不清除 reviewCount） */
    g_app.review.flashcardFace = CARD_FRONT;
    g_app.review.flashcardAnimTime = 0.0f;
    g_app.review.knownInSession = 0;
    g_app.review.unknownInSession = 0;

    /* 测试重置 */
    g_app.test.currentTestIdx = 0;
    g_app.test.testCorrect = 0;
    g_app.test.testTotal = 0;
    g_app.test.selectedAnswer = -1;
    g_app.test.answerResult = -1;
    memset(g_app.test.wrongOptionsUsed, 0, sizeof(g_app.test.wrongOptionsUsed));
}

/**
 * AppState_Deinit - 释放应用状态相关内存
 *
 * 在 main() 的清理阶段调用。
 * 目前只负责释放菜单树（MENU 节点由 malloc 分配）。
 * 其他状态（如单词数据）由各自模块的清理函数处理。
 */
void AppState_Deinit(void) {
    /* 遍历根菜单的子节点，逐个释放 malloc 分配的内存 */
    if (g_app.rootMenu != NULL) {
        for (int i = 0; i < g_app.rootMenu->childindex; i++) {
            if (g_app.rootMenu->child[i]) {
                free(g_app.rootMenu->child[i]);  /* 释放子节点 */
            }
        }
        free(g_app.rootMenu);   /* 释放根节点 */
        g_app.rootMenu = NULL;
        g_app.currentMenu = NULL;
    }
}

// ============================================================================
// 状态访问器函数
// 每个函数返回 AppState 中对应子结构的指针。
// 调用方通过指针直接读写字段，无需额外的 get/set 函数。
// ============================================================================

UIStyle* AppState_GetStyle(void)           { return &g_app.style; }
UIState* AppState_GetUIState(void)          { return &g_app.uiState; }
MENU** AppState_GetCurrentMenu(void)       { return &g_app.currentMenu; }
MenuStack* AppState_GetMenuStack(void)     { return &g_app.menuStack; }
LearnState* AppState_GetLearnState(void)   { return &g_app.learn; }
ReviewState* AppState_GetReviewState(void) { return &g_app.review; }
TestState* AppState_GetTestState(void)     { return &g_app.test; }
SearchState* AppState_GetSearchState(void) { return &g_app.search; }
SelectWordState* AppState_GetSelectWordState(void) { return &g_app.selectWord; }
PlanState* AppState_GetPlanState(void)     { return &g_app.plan; }
AccountState* AppState_GetAccountState(void) { return &g_app.account; }
char* AppState_GetLoginMsg(void)           { return g_app.loginMsg; }

// ============================================================================
// 主题管理函数
// ============================================================================

/**
 * AppState_IsDarkMode - 检查当前是否为深色模式
 * @return true = 深色模式，false = 浅色模式
 */
bool AppState_IsDarkMode(void) {
    return g_app.isDarkMode;
}

/**
 * AppState_SetDarkMode - 设置深色/浅色模式
 * 设置 isDarkMode 标志，同时更新 style.theme 为对应的主题颜色方案。
 * @param isDark true = 深色模式，false = 浅色模式
 */
void AppState_SetDarkMode(bool isDark) {
    g_app.isDarkMode = isDark;
    /* 切换主题：UIThemeDark() 和 UIThemeLight() 定义在 raylib_word_ui.c */
    g_app.style.theme = isDark ? UIThemeDark() : UIThemeLight();
}

/**
 * AppState_ToggleDarkMode - 切换深色/浅色模式
 * 简单的取反操作，委托给 AppState_SetDarkMode。
 */
void AppState_ToggleDarkMode(void) {
    AppState_SetDarkMode(!g_app.isDarkMode);
}
