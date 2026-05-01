// 统一状态管理 — 单例 g_app 管理所有全局状态

#ifndef APP_STATE_H
#define APP_STATE_H

#include "raylib.h"
#include "raylib_word_ui.h"
#include "tree_menu.h"
#include "words.h"
#include "account.h"
#include "plan.h"
#include "config.h"

// 学单词模式
typedef struct {
    int learnIndex;
    bool learnFilterUnknown;
    float learnScrollOffset;
} LearnState;

// 背单词模式(闪卡)
typedef struct {
    int reviewIndices[MAX_WORDS];
    int reviewCount;
    int currentReviewIdx;
    CardFace flashcardFace;
    float flashcardAnimTime;
    int knownInSession;
    int unknownInSession;
} ReviewState;

// 测试模式
typedef struct {
    int testIndices[MAX_WORDS];
    int testCount;
    int currentTestIdx;
    int testCorrect;
    int testTotal;
    int selectedAnswer;
    int answerResult;
    int currentCorrectIdx;
    bool wrongOptionsUsed[MAX_WORDS];
} TestState;

// 查找单词
typedef struct {
    char searchInput[256];
    int searchResults[MAX_WORDS];
    int searchResultCount;
    SearchBarState searchBar;
} SearchState;

// 选词背单词
typedef struct {
    int selectIndices[MAX_WORDS];
    int selectCount;
    int currentSelectIdx;
    int selectCorrect;
    int selectTotal;
    int selectedAnswer;
    int answerResult;
    int currentCorrectIdx;
    bool wrongOptionsUsed[MAX_WORDS];
} SelectWordState;

// 总状态
typedef struct {
    UIStyle style;
    UIState uiState;
    bool isDarkMode;
    MENU* rootMenu;
    MENU* currentMenu;
    MenuStack menuStack;
    LearnState learn;
    ReviewState review;
    TestState test;
    SelectWordState selectWord;
    SearchState search;
    AccountState account;
    char loginMsg[128];
    PlanState plan;
    bool loginRequired;
} AppState;

extern AppState g_app;

void AppState_Init(void);
void AppState_Reset(void);
void AppState_Deinit(void);

UIStyle* AppState_GetStyle(void);
UIState* AppState_GetUIState(void);
MENU** AppState_GetCurrentMenu(void);
MenuStack* AppState_GetMenuStack(void);
LearnState* AppState_GetLearnState(void);
ReviewState* AppState_GetReviewState(void);
TestState* AppState_GetTestState(void);
SearchState* AppState_GetSearchState(void);
SelectWordState* AppState_GetSelectWordState(void);
AccountState* AppState_GetAccountState(void);
char* AppState_GetLoginMsg(void);
PlanState* AppState_GetPlanState(void);

bool AppState_IsDarkMode(void);
void AppState_ToggleDarkMode(void);
void AppState_SetDarkMode(bool isDark);

#endif
