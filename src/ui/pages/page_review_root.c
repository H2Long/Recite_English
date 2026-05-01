// 背单词父页 — 三个子模式入口

#include "pages.h"

void MenuReviewRoot_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);

    // 标题
    Rectangle tr = UILayoutNext(&lay, -1, 60);
    Vector2 ts = MeasureTextAuto(u8"背单词", 42, 1);
    DrawTextAuto(u8"背单词", (Vector2){cr.x + 50, tr.y}, 42, 1, STYLE->theme.primary);

    // 描述
    Rectangle dr = UILayoutNext(&lay, -1, 50);
    Vector2 ds = MeasureTextAuto(u8"选择一种背单词模式开始学习", 24, 1);
    DrawTextAuto(u8"选择一种背单词模式开始学习", (Vector2){cr.x + 50, dr.y},
        24, 1, STYLE->theme.textSecondary);

    // 三个模式卡片
    struct { const char* t; const char* d; Color c; } modes[] = {
        {u8"卡片背单词", u8"闪卡翻转记忆\n正面显示单词\n背面显示释义", {60, 179, 113, 255}},
        {u8"选词背单词", u8"看释义选单词\n四选一选择题\n得分按账号统计", {70, 130, 180, 255}},
        {u8"测试模式",   u8"看单词选释义\n四选一选择题\n完成后显示正确率", {255, 140, 0, 255}},
    };
    Rectangle cardsR = UILayoutNext(&lay, -1, 300);
    UILayout cl = UIBeginLayout(cardsR, UI_DIR_HORIZONTAL, 30, 0);

    for (int i = 0; i < 3; i++) {
        Rectangle card = UILayoutNext(&cl, 320, -1);
        DrawRectangleRounded(card, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(card, 0.1f, 12, modes[i].c);
        Vector2 tsz = MeasureTextAuto(modes[i].t, 32, 1);
        DrawTextAuto(modes[i].t, (Vector2){card.x + 20, card.y + 20}, 32, 1, modes[i].c);
        Rectangle da = {card.x + 20, card.y + 70, card.width - 40, card.height - 130};
        UIDrawTextRec(modes[i].d, da, 22, 1, true, STYLE->theme.textSecondary);
        Rectangle btn = {card.x + card.width/2 - 60, card.y + card.height - 70, 120, 50};
        if(UIButton(modes[i].t, btn, STYLE, UI_STATE, 500 + i) && i < CURRENT_MENU->childindex) {
            MENU* t = CURRENT_MENU->child[i];
            if(t) {
                StackPush(AppState_GetMenuStack(), CURRENT_MENU);
                CURRENT_MENU = t;
            }
        }
    }
}
