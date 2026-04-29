// ============================================================================
// 背单词软件 - 主程序
// 功能：学单词、背单词（闪卡）、测试（选择题）、学习进度管理
// 依赖库：raylib (图形渲染)
// 重构后：使用 AppState 统一管理所有状态变量
// ============================================================================

#include "raylib.h"
#include "app_state.h"
#include "words.h"
#include "fonts.h"
#include "menu_callbacks.h"
#include "account.h"
#include "tree_menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// ============================================================================
// 宏定义：简化访问器调用
// ============================================================================

#define STYLE        (AppState_GetStyle())
#define UI_STATE     (AppState_GetUIState())
#define CURRENT_MENU (*AppState_GetCurrentMenu())
#define LEARN        (*AppState_GetLearnState())
#define REVIEW       (*AppState_GetReviewState())
#define TEST         (*AppState_GetTestState())

// ============================================================================
// 主程序入口
// ============================================================================

int main(void) {
    // 初始化 raylib 窗口
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, u8"背单词软件");
    SetTargetFPS(60);

    // --------------------------------------------------------------------
    // 初始化阶段
    // --------------------------------------------------------------------
    loadWordsFromFile("./words.txt");  // 从文件加载单词数据
    Account_Init();                    // 初始化账号系统
    initWords();                       // 初始化单词进度（加载学习进度）
    srand(time(NULL));                 // 初始化随机数种子
    loadFonts();                       // 加载中英文字体
    UISetFonts(g_mergedFont, g_englishFont);  // 设置 UI 使用的字体
    UISetLatinFont(g_latinFont);       // 设置 IPA 音标字体

    // 初始化应用状态（统一管理所有状态变量）
    AppState_Init();

    // 初始化菜单树
    InitMenuTree();

    // 准备闪卡复习（只显示未复习过的单词）
    REVIEW.reviewCount = 0;
    for (int j = 0; j < g_wordProgressCount; j++) {
        if (g_words[j].progress.lastReview == 0) {
            REVIEW.reviewIndices[REVIEW.reviewCount++] = j;
        }
    }
    shuffleArray(REVIEW.reviewIndices, REVIEW.reviewCount);
    REVIEW.currentReviewIdx = 0;

    // 准备测试模式（最多10题）
    TEST.testCount = (g_wordProgressCount < 10) ? g_wordProgressCount : 10;
    for (int j = 0; j < g_wordProgressCount; j++) {
        TEST.testIndices[j] = j;
    }
    shuffleArray(TEST.testIndices, g_wordProgressCount);
    TEST.currentTestIdx = 0;
    TEST.testCorrect = 0;
    TEST.testTotal = 0;
    TEST.selectedAnswer = -1;
    TEST.answerResult = -1;

    // 初始化 UI 样式（字体大小在此调整）
    STYLE->font = g_mergedFont;
    // 以下是字体大小参数，可根据需要调整
    STYLE->fontSizeSmall = 22.0f;    // 小号字体（如选项标签）
    STYLE->fontSizeNormal = 28.0f;   // 普通字体（如按钮文字）
    STYLE->fontSizeLarge = 46.0f;    // 大号字体（如单词显示）

    // --------------------------------------------------------------------
    // 主循环
    // --------------------------------------------------------------------
    while (!WindowShouldClose()) {
        // 更新 UI 状态
        UIBegin(UI_STATE);

        BeginDrawing();
        ClearBackground(STYLE->theme.background);

        // 绘制顶部标题栏
        Rectangle topBar = {0, 0, SCREEN_WIDTH, 60};
        DrawRectangleRec(topBar, STYLE->theme.panelBg);
        DrawLineEx((Vector2){0, 60}, (Vector2){SCREEN_WIDTH, 60}, 1, STYLE->theme.inputBorder);

        // 标题文字
        const char* topTitle = u8"背单词软件";
        Vector2 titleSize = MeasureTextAuto(topTitle, 40, 1);
        DrawTextAuto(topTitle, (Vector2){SCREEN_WIDTH/2 - titleSize.x/2, 8}, 40, 1, STYLE->theme.primary);

        // --------------------------------------------------------------------
        // 右上角用户信息模块
        // --------------------------------------------------------------------
        Rectangle userInfoRect = {SCREEN_WIDTH - 210, 12, 200, 36};
        bool hoverUser = CheckCollisionPointRec(UI_STATE->mousePos, userInfoRect);
        if (hoverUser) {
            DrawRectangleRounded(userInfoRect, 0.2f, 8, Fade(STYLE->theme.primary, 0.15f));
        }
        DrawRectangleRoundedLines(userInfoRect, 0.2f, 8, Fade(STYLE->theme.textSecondary, 0.3f));

        const char* userText;
        Color userColor;
        if (Account_IsLoggedIn()) {
            userText = Account_GetCurrentUser();
            userColor = STYLE->theme.primary;
        } else {
            userText = u8"未登录";
            userColor = STYLE->theme.textSecondary;
        }
        Vector2 userTextSize = MeasureTextAuto(userText, 22, 1);
        DrawTextAuto(userText,
                     (Vector2){userInfoRect.x + userInfoRect.width/2 - userTextSize.x/2,
                               userInfoRect.y + (userInfoRect.height - userTextSize.y)/2},
                     22, 1, userColor);

        // 点击跳转到账号管理页面
        if (hoverUser && UI_STATE->mouseReleased && g_accountMenuNode != NULL) {
            StackPush(AppState_GetMenuStack(), CURRENT_MENU);
            CURRENT_MENU = g_accountMenuNode;
        }

        // --------------------------------------------------------------------
        // 绘制左侧树形导航菜单
        Rectangle menuRect = {10, 70, 230, SCREEN_HEIGHT - 80};
        DrawTreeMenu(menuRect);

        // 根据当前菜单显示对应页面
        if (CURRENT_MENU->show != NULL) {
            CURRENT_MENU->show();
        }

        EndDrawing();
        UIEnd(UI_STATE);
    }

    // --------------------------------------------------------------------
    // 清理阶段
    // --------------------------------------------------------------------
    AppState_Deinit();    // 释放应用状态
    unloadFonts();        // 卸载字体
    freeWordLibrary();    // 释放单词库内存
    CloseWindow();        // 关闭 raylib 窗口

    return 0;
}
