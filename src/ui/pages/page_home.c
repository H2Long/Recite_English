// ============================================================================
// 主菜单页面
// 包含欢迎信息、统计卡片和三个功能入口
// ============================================================================

#include "pages.h"

void MenuHome_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // 欢迎信息
    Rectangle welcomeRect = UILayoutNext(&layout, -1, 90);
    const char* welcome = u8"欢迎使用背单词软件！";
    Vector2 wSize = MeasureTextAuto(welcome, 52, 1);
    DrawTextAuto(welcome, (Vector2){SCREEN_WIDTH/2 - wSize.x/2, welcomeRect.y}, 52, 1, STYLE->theme.primary);

    Rectangle descRect = UILayoutNext(&layout, -1, 60);
    const char* desc = u8"选择左侧导航栏中的模式开始学习";
    Vector2 dSize = MeasureTextAuto(desc, 28, 1);
    DrawTextAuto(desc, (Vector2){SCREEN_WIDTH/2 - dSize.x/2, descRect.y}, 28, 1, STYLE->theme.textSecondary);

    // 统计信息卡片
    Rectangle statsRect = UILayoutNext(&layout, -1, 100);
    DrawRectangleRounded(statsRect, 0.1f, 12, STYLE->theme.panelBg);

    char statsText[256];
    int mastered = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if (g_words[i].progress.mastered) mastered++;
    }
    snprintf(statsText, sizeof(statsText),
        u8"总单词数: %d  |  已掌握: %d  |  学习中: %d",
        g_wordProgressCount, mastered, g_wordProgressCount - mastered);

    UILayout statsLayout = UIBeginLayout(statsRect, UI_DIR_HORIZONTAL, 0, 25);
    Rectangle statsTextRect = UILayoutNext(&statsLayout, -1, -1);
    DrawTextAuto(statsText, (Vector2){statsTextRect.x, statsTextRect.y + 28}, 22, 1, STYLE->theme.textPrimary);

    // 功能卡片
    Rectangle cardsRect = UILayoutNext(&layout, -1, 160);
    UILayout cardsLayout = UIBeginLayout(cardsRect, UI_DIR_HORIZONTAL, 30, 0);

    struct { const char* title; const char* desc; MENU* target; Color color; } modes[] = {
        {u8"学单词", u8"详细学习每个单词的释义和用法", NULL, (Color){70, 130, 180, 255}},
        {u8"背单词", u8"包含卡片背单词、选词背单词、测试三种模式", NULL, (Color){60, 179, 113, 255}},
        {u8"查找单词", u8"正则表达式快速查找", NULL, (Color){138, 43, 226, 255}}
    };

    for (int i = 0; i < g_app.rootMenu->childindex && i < 3; i++) {
        modes[i].target = g_app.rootMenu->child[i];
    }

    for (int i = 0; i < 3; i++) {
        Rectangle cardRect = UILayoutNext(&cardsLayout, 240, -1);
        DrawRectangleRounded(cardRect, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardRect, 0.1f, 12, modes[i].color);

        Vector2 titleS = MeasureTextAuto(modes[i].title, 32, 1);
        DrawTextAuto(modes[i].title, (Vector2){cardRect.x + 15, cardRect.y + 15}, 32, 1, modes[i].color);

        Rectangle descArea = {cardRect.x + 15, cardRect.y + 55, cardRect.width - 30, cardRect.height - 80};
        UIDrawTextRec(modes[i].desc, descArea, 18, 1, true, STYLE->theme.textSecondary);

        Rectangle goBtn = {cardRect.x + cardRect.width - 100, cardRect.y + cardRect.height - 55, 85, 45};
        if (UIButton(u8"开始", goBtn, STYLE, UI_STATE, 100 + i) && modes[i].target != NULL) {
            StackPush(AppState_GetMenuStack(), modes[i].target);
            CURRENT_MENU = modes[i].target;
        }
    }
}
