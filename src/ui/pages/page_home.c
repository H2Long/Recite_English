// 主页 — 欢迎+统计+快捷入口

#include "pages.h"

void MenuHome_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);

    // 欢迎信息
    Rectangle wr = UILayoutNext(&lay, -1, 90);
    Vector2 ws = MeasureTextAuto(u8"欢迎使用背单词软件！", 52, 1);
    DrawTextAuto(u8"欢迎使用背单词软件！", (Vector2){SCREEN_WIDTH/2 - ws.x/2, wr.y},
        52, 1, STYLE->theme.primary);

    Rectangle dr = UILayoutNext(&lay, -1, 60);
    Vector2 ds = MeasureTextAuto(u8"选择左侧导航栏中的模式开始学习", 28, 1);
    DrawTextAuto(u8"选择左侧导航栏中的模式开始学习",
        (Vector2){SCREEN_WIDTH/2 - ds.x/2, dr.y}, 28, 1, STYLE->theme.textSecondary);

    // 统计: 总单词数 / 已掌握 / 学习中
    Rectangle sr = UILayoutNext(&lay, -1, 100);
    DrawRectangleRounded(sr, 0.1f, 12, STYLE->theme.panelBg);
    int mastered = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if(g_words[i].progress.mastered) { mastered++; }
    }
    char st[256];
    snprintf(st, sizeof(st), u8"总单词数: %d  |  已掌握: %d  |  学习中: %d",
        g_wordProgressCount, mastered, g_wordProgressCount - mastered);
    UILayout sl = UIBeginLayout(sr, UI_DIR_HORIZONTAL, 0, 25);
    Rectangle stR = UILayoutNext(&sl, -1, -1);
    DrawTextAuto(st, (Vector2){stR.x, stR.y + 28}, 22, 1, STYLE->theme.textPrimary);

    // 三个功能卡片
    Rectangle cr2 = UILayoutNext(&lay, -1, 160);
    UILayout cl = UIBeginLayout(cr2, UI_DIR_HORIZONTAL, 30, 0);
    struct { const char* t; const char* d; MENU* target; Color c; } modes[] = {
        {u8"学单词", u8"详细学习每个单词的释义和用法", NULL, {70, 130, 180, 255}},
        {u8"背单词", u8"包含卡片背单词、选词背单词、测试三种模式", NULL, {60, 179, 113, 255}},
        {u8"查找单词", u8"正则表达式快速查找", NULL, {138, 43, 226, 255}},
    };
    for (int i = 0; i < g_app.rootMenu->childindex && i < 3; i++) {
        modes[i].target = g_app.rootMenu->child[i];
    }
    for (int i = 0; i < 3; i++) {
        Rectangle card = UILayoutNext(&cl, 240, -1);
        DrawRectangleRounded(card, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(card, 0.1f, 12, modes[i].c);
        DrawTextAuto(modes[i].t, (Vector2){card.x + 15, card.y + 15}, 32, 1, modes[i].c);
        Rectangle da = {card.x + 15, card.y + 55, card.width - 30, card.height - 80};
        UIDrawTextRec(modes[i].d, da, 18, 1, true, STYLE->theme.textSecondary);
        Rectangle btn = {card.x + card.width - 100, card.y + card.height - 55, 85, 45};
        if(UIButton(u8"开始", btn, STYLE, UI_STATE, 100 + i) && modes[i].target != NULL) {
            StackPush(AppState_GetMenuStack(), modes[i].target);
            CURRENT_MENU = modes[i].target;
        }
    }
}
