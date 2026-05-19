// 主页 — 欢迎+统计+快捷入口

#include "pages.h"

void MenuHome_Show(void) {
    Rectangle cr = {280, 80, SCREEN_WIDTH - 300, SCREEN_HEIGHT - 100};
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

    // 统计: 三个独立卡片
    int mastered = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if(g_words[i].progress.mastered) { mastered++; }
    }
    int learning = g_wordProgressCount - mastered;

    Rectangle sr = UILayoutNext(&lay, -1, 100);
    UILayout sl = UIBeginLayout(sr, UI_DIR_HORIZONTAL, 20, 0);
    struct { const char* label; int value; Color accent; } stats[] = {
        {u8"总单词数", g_wordProgressCount, STYLE->theme.primary},
        {u8"已掌握", mastered, STYLE->theme.success},
        {u8"学习中", learning, (Color){255,165,0,255}},
    };
    for (int i = 0; i < 3; i++) {
        Rectangle card = UILayoutNext(&sl, -1, -1);
        UIDrawCard(card, 0.08f, STYLE);
        DrawRectangleRoundedLines(card, 0.08f, 8, Fade(stats[i].accent, 0.3f));
        char val[32];
        snprintf(val, sizeof(val), "%d", stats[i].value);
        Vector2 vs = MeasureTextAuto(val, 40, 1);
        DrawTextAuto(val, (Vector2){card.x + card.width/2 - vs.x/2, card.y + 12},
            40, 1, stats[i].accent);
        Vector2 ls = MeasureTextAuto(stats[i].label, 18, 1);
        DrawTextAuto(stats[i].label, (Vector2){card.x + card.width/2 - ls.x/2, card.y + 60},
            18, 1, STYLE->theme.textSecondary);
    }

    // 当前计划信息
    LearningPlan* active = Plan_GetActive();
    if(active != NULL) {
        Rectangle pr = UILayoutNext(&lay, -1, 60);
        UIDrawCard(pr, 0.08f, STYLE);
        int remaining = Plan_GetRemainingToday();
        char planInfo[256];
        snprintf(planInfo, sizeof(planInfo), u8"当前计划: %s  |  今日目标: %d 词  |  剩余: %d 词",
            active->name, active->dailyWordCount, remaining);
        DrawTextAuto(planInfo, (Vector2){pr.x + 20, pr.y + 18}, 20, 1,
            remaining > 0 ? STYLE->theme.textPrimary : STYLE->theme.success);
    }

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
        UIDrawCard(card, 0.08f, STYLE);
        DrawRectangleRoundedLines(card, 0.08f, 12, Fade(modes[i].c, 0.4f));
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
