// ============================================================================
// 背单词模式父页面（显示三个子模式入口卡片）
// ============================================================================

#include "pages.h"

void MenuReviewRoot_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    const char* title = u8"背单词";
    Vector2 tSize = MeasureTextAuto(title, 42, 1);
    DrawTextAuto(title, (Vector2){contentRect.x + 50, titleRect.y}, 42, 1, STYLE->theme.primary);

    Rectangle descRect = UILayoutNext(&layout, -1, 50);
    const char* desc = u8"选择一种背单词模式开始学习";
    Vector2 dSize = MeasureTextAuto(desc, 24, 1);
    DrawTextAuto(desc, (Vector2){contentRect.x + 50, descRect.y}, 24, 1, STYLE->theme.textSecondary);

    Rectangle cardsRect = UILayoutNext(&layout, -1, 300);
    UILayout cardsLayout = UIBeginLayout(cardsRect, UI_DIR_HORIZONTAL, 30, 0);

    struct { const char* title; const char* desc; Color color; } modes[] = {
        {u8"卡片背单词", u8"闪卡翻转记忆\n正面显示单词\n背面显示释义", (Color){60, 179, 113, 255}},
        {u8"选词背单词", u8"看释义选单词\n四选一选择题\n得分按账号统计", (Color){70, 130, 180, 255}},
        {u8"测试模式", u8"看单词选释义\n四选一选择题\n完成后显示正确率", (Color){255, 140, 0, 255}},
    };

    for (int i = 0; i < 3; i++) {
        Rectangle cardRect = UILayoutNext(&cardsLayout, 320, -1);
        DrawRectangleRounded(cardRect, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardRect, 0.1f, 12, modes[i].color);

        Vector2 titleS = MeasureTextAuto(modes[i].title, 32, 1);
        DrawTextAuto(modes[i].title, (Vector2){cardRect.x + 20, cardRect.y + 20}, 32, 1, modes[i].color);

        Rectangle descArea = {cardRect.x + 20, cardRect.y + 70, cardRect.width - 40, cardRect.height - 130};
        UIDrawTextRec(modes[i].desc, descArea, 22, 1, true, STYLE->theme.textSecondary);

        Rectangle goBtn = {cardRect.x + cardRect.width / 2 - 60, cardRect.y + cardRect.height - 70, 120, 50};
        if (UIButton(modes[i].title, goBtn, STYLE, UI_STATE, 500 + i)) {
            if (i < g_app.currentMenu->childindex) {
                MENU* target = g_app.currentMenu->child[i];
                if (target != NULL) {
                    StackPush(AppState_GetMenuStack(), g_app.currentMenu);
                    CURRENT_MENU = target;
                }
            }
        }
    }
}
