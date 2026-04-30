// ============================================================================
// 背单词父页面
// 功能：显示三个子模式（卡片背单词/选词背单词/测试）的入口卡片
// ============================================================================

#include "pages.h"

/**
 * MenuReviewRoot_Show - 渲染背单词模式的父页面
 *
 * 展示三个功能卡片，每个卡片包含标题、说明文字和进入按钮。
 * 点击按钮后通过菜单栈导航到对应子页面。
 * 子页面对应关系：child[0]=卡片背单词, child[1]=选词背单词, child[2]=测试
 */
void MenuReviewRoot_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);

    Rectangle tr = UILayoutNext(&layout, -1, 60);
    Vector2 ts = MeasureTextAuto(u8"背单词", 42, 1);
    DrawTextAuto(u8"背单词", (Vector2){cr.x + 50, tr.y}, 42, 1, STYLE->theme.primary);
    Rectangle dr = UILayoutNext(&layout, -1, 50);
    Vector2 ds = MeasureTextAuto(u8"选择一种背单词模式开始学习", 24, 1);
    DrawTextAuto(u8"选择一种背单词模式开始学习", (Vector2){cr.x + 50, dr.y}, 24, 1, STYLE->theme.textSecondary);

    // 三个模式卡片（水平排列）
    struct { const char* t; const char* d; Color c; } modes[] = {
        {u8"卡片背单词", u8"闪卡翻转记忆\n正面显示单词\n背面显示释义", (Color){60, 179, 113, 255}},
        {u8"选词背单词", u8"看释义选单词\n四选一选择题\n得分按账号统计", (Color){70, 130, 180, 255}},
        {u8"测试模式",   u8"看单词选释义\n四选一选择题\n完成后显示正确率", (Color){255, 140, 0, 255}},
    };
    Rectangle cardsR = UILayoutNext(&layout, -1, 300);
    UILayout cl = UIBeginLayout(cardsR, UI_DIR_HORIZONTAL, 30, 0);

    for (int i = 0; i < 3; i++) {
        Rectangle cardR = UILayoutNext(&cl, 320, -1);
        DrawRectangleRounded(cardR, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardR, 0.1f, 12, modes[i].c);

        Vector2 tSize = MeasureTextAuto(modes[i].t, 32, 1);
        DrawTextAuto(modes[i].t, (Vector2){cardR.x + 20, cardR.y + 20}, 32, 1, modes[i].c);

        Rectangle da = {cardR.x + 20, cardR.y + 70, cardR.width - 40, cardR.height - 130};
        UIDrawTextRec(modes[i].d, da, 22, 1, true, STYLE->theme.textSecondary);

        Rectangle goBtn = {cardR.x + cardR.width/2 - 60, cardR.y + cardR.height - 70, 120, 50};
        if (UIButton(modes[i].t, goBtn, STYLE, UI_STATE, 500 + i)
            && i < g_app.currentMenu->childindex) {
            MENU* target = g_app.currentMenu->child[i];
            if (target) {
                StackPush(AppState_GetMenuStack(), g_app.currentMenu);
                CURRENT_MENU = target;
            }
        }
    }
}
