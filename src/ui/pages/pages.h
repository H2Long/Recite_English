// ============================================================================
// 页面函数公共头文件
// 功能：声明所有页面的显示函数，集中管理公共宏和外部引用
// ============================================================================

#ifndef PAGES_H
#define PAGES_H

#include "raylib.h"
#include "raylib_word_ui.h"
#include "tree_menu.h"
#include "words.h"
#include "fonts.h"
#include "app_state.h"
#include "config.h"
#include "account.h"
#include "plan.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// ============================================================================
// 外部引用（由 words.c 提供）
// ============================================================================

extern WordEntry* g_wordLibrary;

// ============================================================================
// 公共宏：简化 AppState 访问器调用
// ============================================================================

#define STYLE       (AppState_GetStyle())
#define UI_STATE    (AppState_GetUIState())
#define CURRENT_MENU (*AppState_GetCurrentMenu())
#define MENU_STACK  (AppState_GetMenuStack())
#define LEARN       (*AppState_GetLearnState())
#define REVIEW      (*AppState_GetReviewState())
#define TEST        (*AppState_GetTestState())
#define SEARCH      (*AppState_GetSearchState())
#define ACCOUNT     (*AppState_GetAccountState())
#define SELECT_WORD (*AppState_GetSelectWordState())
#define PLAN_STATE  (*AppState_GetPlanState())
#define LOGIN_MSG   (AppState_GetLoginMsg())

// ============================================================================
// 页面函数声明（每个页面的渲染逻辑）
// ============================================================================

void MenuHome_Show(void);
void MenuLearn_Show(void);
void MenuReviewRoot_Show(void);
void MenuCardReview_Show(void);
void MenuSelectWord_Show(void);
void MenuTest_Show(void);
void MenuSearch_Show(void);

void MenuPlanRoot_Show(void);
void MenuPlanManager_Show(void);
void MenuProgress_Show(void);

void MenuSettings_Show(void);
void MenuAccount_Show(void);
void MenuLogin_Show(void);
void MenuRegister_Show(void);
void MenuWordManager_Show(void);

#endif // PAGES_H
