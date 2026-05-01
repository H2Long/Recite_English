// 页面公共头文件 — 包含所有依赖, 声明所有页面函数, 提供访问宏

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

extern WordEntry* g_wordLibrary;

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

#endif
