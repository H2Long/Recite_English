// 背单词软件 — 主入口

#include "raylib.h"
#include "app_state.h"
#include "words.h"
#include "fonts.h"
#include "menu_callbacks.h"
#include "account.h"
#include "plan.h"
#include "tree_menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define STYLE        (AppState_GetStyle())
#define UI_STATE     (AppState_GetUIState())
#define CURRENT_MENU (*AppState_GetCurrentMenu())
#define LEARN        (*AppState_GetLearnState())
#define REVIEW       (*AppState_GetReviewState())
#define TEST         (*AppState_GetTestState())
#define SELECT_WORD  (*AppState_GetSelectWordState())

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    // 初始化阶段
    loadWordsFromFile(WORDS_FILE_PATH);           // 1. 加载单词
    Account_Init();                               // 2. 账号系统
    Plan_Init();                                  // 3. 学习计划
    initWords();                                  // 4. 单词进度
    srand((unsigned int)(time(NULL) ^ (uintptr_t)&srand));  // 5. 随机种子
    loadFonts();                                  // 6. 字体
    UISetFonts(g_mergedFont, g_englishFont);       // 7. UI字体
    UISetLatinFont(g_latinFont);                  // 8. IPA字体

    AppState_Init();
    g_app.loginRequired = true;
    InitMenuTree();

    // 准备闪卡复习
    REVIEW.reviewCount = 0;
    for (int j = 0; j < g_wordProgressCount; j++) {
        if(g_words[j].progress.lastReview == 0) {
            REVIEW.reviewIndices[REVIEW.reviewCount++] = j;
        }
    }
    shuffleArray(REVIEW.reviewIndices, REVIEW.reviewCount);
    REVIEW.currentReviewIdx = 0;

    // 准备测试
    TEST.testCount = g_wordProgressCount < 10 ? g_wordProgressCount : 10;
    for (int j = 0; j < g_wordProgressCount; j++) TEST.testIndices[j] = j;
    shuffleArray(TEST.testIndices, g_wordProgressCount);
    TEST.currentTestIdx = 0;
    TEST.testCorrect = 0;
    TEST.testTotal = 0;
    TEST.selectedAnswer = -1;
    TEST.answerResult = -1;

    // UI样式
    STYLE->font = g_mergedFont;
    STYLE->fontSizeSmall = 22.0f;
    STYLE->fontSizeNormal = 28.0f;
    STYLE->fontSizeLarge = 46.0f;

    // 主循环
    while (!WindowShouldClose()) {
        UIBegin(UI_STATE);
        BeginDrawing();
        ClearBackground(STYLE->theme.background);

        // 顶部标题栏
        Rectangle topBar = {0, 0, SCREEN_WIDTH, 60};
        DrawRectangleRec(topBar, STYLE->theme.panelBg);
        DrawLineEx((Vector2){0, 60}, (Vector2){SCREEN_WIDTH, 60}, 1, STYLE->theme.inputBorder);

        const char* topTitle = u8"背单词软件";
        Vector2 titleSize = MeasureTextAuto(topTitle, 40, 1);
        DrawTextAuto(topTitle, (Vector2){SCREEN_WIDTH/2 - titleSize.x/2, 8},
            40, 1, STYLE->theme.primary);

        // 右上角用户信息
        Rectangle userInfo = {SCREEN_WIDTH - 210, 12, 200, 36};
        bool hov = CheckCollisionPointRec(UI_STATE->mousePos, userInfo);
        if(hov) { DrawRectangleRounded(userInfo, 0.2f, 8, Fade(STYLE->theme.primary, 0.15f)); }
        DrawRectangleRoundedLines(userInfo, 0.2f, 8, Fade(STYLE->theme.textSecondary, 0.3f));

        const char* userText;
        Color userColor;
        if(Account_IsLoggedIn()) {
            userText = Account_GetCurrentUser();
            userColor = STYLE->theme.primary;
        }
        else {
            userText = u8"未登录";
            userColor = STYLE->theme.textSecondary;
        }
        Vector2 uts = MeasureTextAuto(userText, 22, 1);
        DrawTextAuto(userText, (Vector2){userInfo.x + userInfo.width/2 - uts.x/2,
            userInfo.y + (userInfo.height - uts.y)/2}, 22, 1, userColor);

        if(hov && UI_STATE->mouseReleased && g_accountMenuNode != NULL) {
            StackPush(AppState_GetMenuStack(), CURRENT_MENU);
            CURRENT_MENU = g_accountMenuNode;
        }

        // 左侧导航
        Rectangle menuRect = {10, 70, 230, SCREEN_HEIGHT - 80};
        DrawTreeMenu(menuRect);

        // 当前页面
        if(CURRENT_MENU->show != NULL) { CURRENT_MENU->show(); }

        // 登录强制层
        if(g_app.loginRequired && !Account_IsLoggedIn()) {
            bool isAcc = (CURRENT_MENU->show == MenuAccount_Show
                       || CURRENT_MENU->show == MenuLogin_Show
                       || CURRENT_MENU->show == MenuRegister_Show);
            if(!isAcc) {
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));
                Rectangle panel = {SCREEN_WIDTH/2 - 280, SCREEN_HEIGHT/2 - 160, 560, 320};
                DrawRectangleRounded(panel, 0.1f, 12, STYLE->theme.panelBg);
                DrawRectangleRoundedLines(panel, 0.1f, 12, STYLE->theme.primary);

                DrawTextAuto(u8"请先登录", (Vector2){SCREEN_WIDTH/2 - 80, panel.y + 30},
                    36, 1, STYLE->theme.primary);
                DrawTextAuto(u8"使用本软件前需要登录账号",
                    (Vector2){SCREEN_WIDTH/2 - 140, panel.y + 80}, 22, 1, STYLE->theme.textSecondary);
                DrawTextAuto(u8"学习进度将按账号独立保存",
                    (Vector2){SCREEN_WIDTH/2 - 130, panel.y + 110}, 22, 1, STYLE->theme.textSecondary);

                Rectangle loginBtn = {SCREEN_WIDTH/2 - 120, panel.y + 170, 240, 55};
                if(UIButton(u8"登录", loginBtn, STYLE, UI_STATE, 800)) {
                    if(g_accountMenuNode != NULL) {
                        StackPush(AppState_GetMenuStack(), CURRENT_MENU);
                        CURRENT_MENU = g_accountMenuNode;
                    }
                }
                Rectangle regBtn = {SCREEN_WIDTH/2 - 120, panel.y + 240, 240, 55};
                if(UIButton(u8"注册账号", regBtn, STYLE, UI_STATE, 801)) {
                    if(g_accountMenuNode != NULL) {
                        StackPush(AppState_GetMenuStack(), CURRENT_MENU);
                        CURRENT_MENU = g_accountMenuNode;
                    }
                }
            }
        }

        EndDrawing();
        UIEnd(UI_STATE);
    }

    // 清理
    AppState_Deinit();
    unloadFonts();
    freeWordLibrary();
    CloseWindow();
    return 0;
}
