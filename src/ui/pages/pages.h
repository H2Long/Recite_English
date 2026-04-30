// ============================================================================
// 页面函数公共头文件
// 功能：所有页面渲染函数的集中声明 + 公共访问宏
//
// 设计决策：
//   本文件是所有页面文件（page_*.c）唯一需要包含的头文件。
//   它集中了：
//   1. 所有外部依赖的头文件（raylib/words/fonts/account/plan 等）
//   2. 访问 AppState 子状态的公共宏（STYLE / UI_STATE / LEARN 等）
//   3. 外部全局变量的 extern 声明（g_wordLibrary）
//   4. 所有 14 个页面函数的声明
//
//   这样做的好处是：每个页面文件只需 #include "pages.h"，
//   不需要分别包含 raylib.h / app_state.h / words.h 等，
//   大大简化了页面文件的依赖管理。
// ============================================================================

#ifndef PAGES_H
#define PAGES_H

// ============================================================================
// 外部依赖
// raylib.h: 所有 raylib API（绘图、输入、数学向量等）
// raylib_word_ui.h: UI 组件（UIButton/UITextBox/UICheckbox 等）
// tree_menu.h: 菜单树节点 MENU 类型 + StackPush/Pop
// words.h: g_words / g_wordProgressCount / g_wordCount 等
// fonts.h: MeasureTextAuto / DrawTextAuto / formatTime 等
// app_state.h: AppState_GetStyle / AppState_GetLearnState 等
// config.h: SCREEN_WIDTH / SCREEN_HEIGHT 等常量
// account.h: Account_IsLoggedIn / Account_Save 等
// plan.h: Plan_GetActive / Plan_Create 等
// ============================================================================

#include "raylib.h"
#include "raylib_word_ui.h"
#include "tree_menu.h"
#include "words.h"
#include "fonts.h"
#include "app_state.h"
#include "config.h"
#include "account.h"
#include "plan.h"

#include <stdbool.h>   /* bool 类型 */
#include <stdio.h>      /* snprintf 格式化 */
#include <stdlib.h>     /* rand/atoi/atol */
#include <string.h>     /* strncpy/strcmp/strlen */
#include <time.h>       /* time/localtime */
#include <ctype.h>      /* tolower */

// ============================================================================
// 外部引用（由 words.c 定义）
// g_wordLibrary: 单词库动态数组，用于词库管理页面的增删改
// ============================================================================

extern WordEntry* g_wordLibrary;

// ============================================================================
// 公共宏：AppState 子状态访问器
//
// 使用方式：
//   在页面函数中直接用 STYLE 代替 AppState_GetStyle()，
//   用 UI_STATE 代替 AppState_GetUIState()。
//   这些宏大大简化了代码，是每个页面函数的基础工具。
//
// 注意：
//   CURRENT_MENU 和 MENU_STACK 用于页面导航（StackPush/Pop）。
//   LOGIN_MSG 用于登录/注册页面的提示信息。
// ============================================================================

#define STYLE       (AppState_GetStyle())          /* UI 样式指针（主题/字体/字号） */
#define UI_STATE    (AppState_GetUIState())        /* UI 交互状态（鼠标/键盘） */
#define CURRENT_MENU (*AppState_GetCurrentMenu())  /* 当前页面菜单节点 */
#define MENU_STACK  (AppState_GetMenuStack())      /* 菜单导航栈 */
#define LEARN       (*AppState_GetLearnState())    /* 学单词模式状态 */
#define REVIEW      (*AppState_GetReviewState())   /* 背单词模式状态 */
#define TEST        (*AppState_GetTestState())     /* 测试模式状态 */
#define SEARCH      (*AppState_GetSearchState())   /* 查找单词模式状态 */
#define ACCOUNT     (*AppState_GetAccountState())  /* 账号系统状态 */
#define SELECT_WORD (*AppState_GetSelectWordState()) /* 选词背单词状态 */
#define PLAN_STATE  (*AppState_GetPlanState())     /* 学习计划状态 */
#define LOGIN_MSG   (AppState_GetLoginMsg())       /* 登录/注册提示消息 */

// ============================================================================
// 页面函数声明
// 每个函数对应一个菜单页面，由 InitMenuTree() 在启动时注册到菜单树中。
// 所有函数遵循 void func(void) 签名，由 menu_callbacks.c 的
// InitMenuTree() 通过 CreatMenuTreeNode(NULL, func) 注册。
// ============================================================================

void MenuHome_Show(void);           /* 主页：欢迎 + 统计 + 三个入口卡片 */
void MenuLearn_Show(void);          /* 学单词：左侧列表 + 右侧详情 */
void MenuReviewRoot_Show(void);     /* 背单词父页：三个子模式入口 */
void MenuCardReview_Show(void);     /* 卡片背单词：闪卡翻转 */
void MenuSelectWord_Show(void);     /* 选词背单词：看释义选单词 */
void MenuTest_Show(void);           /* 测试模式：看单词选释义 */
void MenuSearch_Show(void);         /* 查找单词：通配符搜索 */

void MenuPlanRoot_Show(void);       /* 学习计划父页：当前计划 + 进度条 */
void MenuPlanManager_Show(void);    /* 计划管理：创建/选择/删除 */
void MenuProgress_Show(void);       /* 学习进度：统计 + 详细列表 */

void MenuSettings_Show(void);       /* 设置：深色模式 + 版本信息 */
void MenuAccount_Show(void);        /* 账号管理：登录/注册/用户列表 */
void MenuLogin_Show(void);          /* 登录子页面 */
void MenuRegister_Show(void);       /* 注册子页面 */
void MenuWordManager_Show(void);    /* 词库管理：搜索/添加/编辑/删除 */

#endif // PAGES_H
