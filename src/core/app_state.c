// 统一状态管理 — 实现

#include "app_state.h"
#include "account.h"
#include <string.h>
#include <stdlib.h>

AppState g_app = {0};

void AppState_Init(void)
{
    UIStyleInit(&g_app.style);

    g_app.rootMenu = NULL;
    g_app.currentMenu = NULL;
    StackInit(&g_app.menuStack);

    g_app.learn.learnIndex = 0;
    g_app.learn.learnFilterUnknown = false;
    g_app.learn.learnScrollOffset = 0.0f;

    g_app.review.reviewCount = 0;
    g_app.review.currentReviewIdx = 0;
    g_app.review.flashcardFace = CARD_FRONT;
    g_app.review.flashcardAnimTime = 0.0f;
    g_app.review.knownInSession = 0;
    g_app.review.unknownInSession = 0;

    g_app.test.testCount = 0;
    g_app.test.currentTestIdx = 0;
    g_app.test.testCorrect = 0;
    g_app.test.testTotal = 0;
    g_app.test.selectedAnswer = -1;
    g_app.test.answerResult = -1;
    g_app.test.currentCorrectIdx = 0;
    memset(g_app.test.wrongOptionsUsed, 0, sizeof(g_app.test.wrongOptionsUsed));

    memset(g_app.search.searchInput, 0, sizeof(g_app.search.searchInput));
    g_app.search.searchResultCount = 0;
    memset(&g_app.search.searchBar, 0, sizeof(g_app.search.searchBar));

    g_app.selectWord.selectCount = 0;
    g_app.selectWord.currentSelectIdx = 0;
    g_app.selectWord.selectCorrect = 0;
    g_app.selectWord.selectTotal = 0;
    g_app.selectWord.selectedAnswer = -1;
    g_app.selectWord.answerResult = -1;
    g_app.selectWord.currentCorrectIdx = 0;
    memset(g_app.selectWord.wrongOptionsUsed, 0, sizeof(g_app.selectWord.wrongOptionsUsed));

    Account_SetState(&g_app.account);
    Plan_SetState(&g_app.plan);
}

void AppState_Reset(void)
{
    g_app.learn.learnIndex = 0;
    g_app.learn.learnFilterUnknown = false;
    g_app.learn.learnScrollOffset = 0.0f;

    g_app.review.flashcardFace = CARD_FRONT;
    g_app.review.flashcardAnimTime = 0.0f;
    g_app.review.knownInSession = 0;
    g_app.review.unknownInSession = 0;

    g_app.test.currentTestIdx = 0;
    g_app.test.testCorrect = 0;
    g_app.test.testTotal = 0;
    g_app.test.selectedAnswer = -1;
    g_app.test.answerResult = -1;
    memset(g_app.test.wrongOptionsUsed, 0, sizeof(g_app.test.wrongOptionsUsed));
}

void AppState_Deinit(void)
{
    if(g_app.rootMenu == NULL) {
        return ;
    }
    for (int i = 0; i < g_app.rootMenu->childindex; i++) {
        if(g_app.rootMenu->child[i]) {
            free(g_app.rootMenu->child[i]);
        }
    }
    free(g_app.rootMenu);
    g_app.rootMenu = NULL;
    g_app.currentMenu = NULL;
}

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

bool AppState_IsDarkMode(void) {
    return g_app.isDarkMode;
}

void AppState_SetDarkMode(bool isDark) {
    g_app.isDarkMode = isDark;
    g_app.style.theme = isDark ? UIThemeDark() : UIThemeLight();
}

void AppState_ToggleDarkMode(void) {
    AppState_SetDarkMode(!g_app.isDarkMode);
}
