// 学习计划 — 计划管理+进度追踪

#include "pages.h"

// 计划管理表单
static struct {
    char name[64]; int daily, days;
    UITextBoxState nameState, dailyState, daysState;
    bool isAdding; char msg[128]; bool ready;
} g_planForm = {0};

void MenuPlanRoot_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);

    Rectangle tr = UILayoutNext(&lay, -1, 60);
    DrawTextAuto(u8"学习计划", (Vector2){cr.x + 50, tr.y}, 42, 1, STYLE->theme.primary);

    LearningPlan* active = Plan_GetActive();
    if(active != NULL) {
        Rectangle ir = UILayoutNext(&lay, -1, 80);
        DrawRectangleRounded(ir, 0.1f, 8, STYLE->theme.panelBg);
        char info[256];
        snprintf(info, sizeof(info), u8"当前计划: %s  |  第 %d/%d 天  |  今日 %d/%d 词",
            active->name, active->currentDay + 1, active->totalDays,
            active->studiedToday, active->dailyWordCount);
        DrawTextAuto(info, (Vector2){ir.x + 20, ir.y + 25}, 22, 1, STYLE->theme.primary);

        float prog = active->dailyWordCount > 0
            ? (float)active->studiedToday / active->dailyWordCount : 0;
        if(prog > 1) { prog = 1; }
        Rectangle bar = {ir.x + 20, ir.y + 55, ir.width - 40, 12};
        DrawRectangleRounded(bar, 0.5f, 4, STYLE->theme.inputBg);
        DrawRectangleRounded((Rectangle){bar.x, bar.y, bar.width*prog, bar.height},
            0.5f, 4, STYLE->theme.success);
    }
    else {
        Rectangle nr = UILayoutNext(&lay, -1, 60);
        DrawTextAuto(u8"暂未选择学习计划，请先创建或选择一个计划",
            (Vector2){nr.x, nr.y}, 22, 1, STYLE->theme.textSecondary);
    }

    // 入口卡片
    Rectangle cardsR = UILayoutNext(&lay, -1, 200);
    UILayout cl = UIBeginLayout(cardsR, UI_DIR_HORIZONTAL, 30, 0);
    struct { const char* t; const char* d; Color c; } cards[] = {
        {u8"计划管理", u8"创建、选择、删除\n学习计划", {70, 130, 180, 255}},
        {u8"学习进度", u8"查看每个单词的\n学习掌握情况", {60, 179, 113, 255}},
    };
    for (int i = 0; i < 2; i++) {
        Rectangle card = UILayoutNext(&cl, 320, -1);
        DrawRectangleRounded(card, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(card, 0.1f, 12, cards[i].c);
        Vector2 ts = MeasureTextAuto(cards[i].t, 32, 1);
        DrawTextAuto(cards[i].t, (Vector2){card.x + 20, card.y + 20}, 32, 1, cards[i].c);
        Rectangle da = {card.x + 20, card.y + 70, card.width - 40, card.height - 130};
        UIDrawTextRec(cards[i].d, da, 22, 1, true, STYLE->theme.textSecondary);
        Rectangle btn = {card.x + card.width/2 - 60, card.y + card.height - 70, 120, 50};
        if(UIButton(cards[i].t, btn, STYLE, UI_STATE, 700 + i) && i < CURRENT_MENU->childindex) {
            MENU* t = CURRENT_MENU->child[i];
            if(t) { StackPush(AppState_GetMenuStack(), CURRENT_MENU); CURRENT_MENU = t; }
        }
    }
}

void MenuPlanManager_Show(void) {
    if(!g_planForm.ready) {
        memset(&g_planForm, 0, sizeof(g_planForm));
        g_planForm.ready = true;
        g_planForm.daily = 10;
        g_planForm.days = 7;
        strcpy(g_planForm.nameState.buffer, u8"我的计划");
    }

    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_HORIZONTAL, 20, 25);

    // 左侧: 计划列表
    Rectangle lr = UILayoutNext(&lay, 320, -1);
    DrawRectangleRounded(lr, 0.1f, 8, STYLE->theme.panelBg);
    DrawTextAuto(u8"我的计划", (Vector2){lr.x + 15, lr.y + 15}, 24, 1, STYLE->theme.textPrimary);
    static float planListScroll = 0;

    Rectangle la = {lr.x, lr.y + 55, lr.width, lr.height - 55};
    UIScrollView sv = {0};
    sv.viewport = la;
    sv.scrollOffset.y = planListScroll;
    sv.contentSize = (Vector2){la.width - 20, PLAN_STATE.planCount * 60.0f + 10};
    UIBeginScrollView(&sv, la, sv.contentSize);

    for (int i = 0; i < PLAN_STATE.planCount; i++) {
        Rectangle item = {la.x + 10, la.y + 5 + i*60 - sv.scrollOffset.y, la.width - 20, 55};
        bool act = (i == Plan_GetActiveIndex());
        if(act) {
            DrawRectangleRounded(item, 0.08f, 6, Fade(STYLE->theme.primary, 0.15f));
            DrawRectangleRoundedLines(item, 0.08f, 6, STYLE->theme.primary);
            DrawRectangleRec((Rectangle){item.x, item.y, 4, item.height}, STYLE->theme.primary);
        }
        else if(CheckCollisionPointRec(UI_STATE->mousePos, item)) {
            DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.06f));
        }
        if(CheckCollisionPointRec(UI_STATE->mousePos, item) && UI_STATE->mouseReleased && !act) {
            Plan_SetActive(i);
        }

        float nx = item.x + 18;
        DrawTextAuto(PLAN_STATE.plans[i].name, (Vector2){nx, item.y + 4}, 22, 1,
            act ? STYLE->theme.primary : STYLE->theme.textPrimary);
        if(act) { DrawTextAuto(u8"> 当前使用", (Vector2){item.x+item.width-100, item.y+4}, 16, 1, STYLE->theme.primary); }

        LearningPlan* p = &PLAN_STATE.plans[i];
        float dp = p->totalDays > 0 ? (float)(p->currentDay+1)/p->totalDays : 0;
        if(dp > 1) { dp = 1; }
        char sub[128];
        snprintf(sub, sizeof(sub), u8"%d词/天 x %d天  进度 %d%%",
            p->dailyWordCount, p->totalDays, (int)(dp*100));
        DrawTextAuto(sub, (Vector2){nx, item.y + 29}, 14, 1, STYLE->theme.textSecondary);
        Rectangle bar = {nx, item.y + 48, item.width - nx + item.x - la.x - 50, 4};
        DrawRectangleRec(bar, Fade(STYLE->theme.textSecondary, 0.2f));
        DrawRectangleRec((Rectangle){bar.x, bar.y, bar.width*dp, bar.height},
            act ? STYLE->theme.primary : STYLE->theme.success);

        // 删除按钮
        Rectangle db = {item.x + item.width - 28, item.y + 4, 24, 22};
        bool dh = CheckCollisionPointRec(UI_STATE->mousePos, db);
        if(dh) { DrawRectangleRounded(db, 0.2f, 4, Fade(RED, 0.2f)); }
        DrawTextAuto(u8"x", (Vector2){db.x+6, db.y+2}, 16, 1,
            dh ? RED : STYLE->theme.textSecondary);
        if(dh && UI_STATE->mouseReleased) { Plan_Delete(i); }
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
    planListScroll = sv.scrollOffset.y;

    // 右侧: 创建表单
    Rectangle fr = UILayoutNext(&lay, -1, -1);
    DrawRectangleRounded(fr, 0.1f, 8, STYLE->theme.panelBg);
    UILayout fl = UIBeginLayout(fr, UI_DIR_VERTICAL, 15, 30);
    Rectangle ft = UILayoutNext(&fl, -1, 40);
    DrawTextAuto(u8"创建新计划", (Vector2){ft.x, ft.y}, 28, 1, STYLE->theme.primary);

    // 三个输入框
    DrawTextAuto(u8"计划名称", (Vector2){UILayoutNext(&fl, -1, 28).x, UILayoutNext(&fl, -1, 28).y},
        20, 1, STYLE->theme.textSecondary);
    UITextBox(&g_planForm.nameState, UILayoutNext(&fl, -1, 42), STYLE, UI_STATE, false);
    DrawTextAuto(u8"每日单词数", (Vector2){UILayoutNext(&fl, -1, 28).x, UILayoutNext(&fl, -1, 28).y},
        20, 1, STYLE->theme.textSecondary);
    UITextBox(&g_planForm.dailyState, UILayoutNext(&fl, -1, 42), STYLE, UI_STATE, false);
    DrawTextAuto(u8"总天数", (Vector2){UILayoutNext(&fl, -1, 28).x, UILayoutNext(&fl, -1, 28).y},
        20, 1, STYLE->theme.textSecondary);
    UITextBox(&g_planForm.daysState, UILayoutNext(&fl, -1, 42), STYLE, UI_STATE, false);

    if(strlen(g_planForm.msg) > 0) {
        Rectangle mr = UILayoutNext(&fl, -1, 30);
        DrawTextAuto(g_planForm.msg, (Vector2){mr.x, mr.y}, 18, 1,
            strstr(g_planForm.msg, "失败") ? STYLE->theme.error : STYLE->theme.success);
    }

    Rectangle btnR = UILayoutNext(&fl, -1, 50);
    UILayout bl = UIBeginLayout(btnR, UI_DIR_HORIZONTAL, 20, 0);
    if(UIButton(u8"创建计划", UILayoutNext(&bl, 150, 45), STYLE, UI_STATE, 710)) {
        const char* nm = g_planForm.nameState.buffer;
        int d = atoi(g_planForm.dailyState.buffer);
        int dd = atoi(g_planForm.daysState.buffer);
        if(strlen(nm) == 0) { strcpy(g_planForm.msg, u8"请输入计划名称"); }
        else if(d <= 0 || dd <= 0) { strcpy(g_planForm.msg, u8"请输入有效的数字"); }
        else if(Plan_Create(nm, d, dd)) {
            strcpy(g_planForm.msg, u8"计划创建成功！");
            memset(&g_planForm.nameState, 0, sizeof(UITextBoxState));
            memset(&g_planForm.dailyState, 0, sizeof(UITextBoxState));
            memset(&g_planForm.daysState, 0, sizeof(UITextBoxState));
        }
        else strcpy(g_planForm.msg, u8"创建失败：计划名已存在或已满");
    }
    if(UIButton(u8"使用默认", UILayoutNext(&bl, 150, 45), STYLE, UI_STATE, 711)) {
        Plan_AddDefaults();
        strcpy(g_planForm.msg, u8"已添加默认计划");
    }
}

void MenuProgress_Show(void) {
    UILayout lay = UIBeginLayout((Rectangle){250, 80, SCREEN_WIDTH-270, SCREEN_HEIGHT-100},
        UI_DIR_VERTICAL, 20, 25);

    Rectangle sc = UILayoutNext(&lay, -1, 130);
    DrawRectangleRounded(sc, 0.1f, 12, STYLE->theme.panelBg);

    int mastered = 0, learning = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if(g_words[i].progress.mastered) { mastered++; } else { learning++; }
    }

    UILayout sl = UIBeginLayout(sc, UI_DIR_HORIZONTAL, 0, 0);
    Rectangle s1 = UILayoutNext(&sl, sc.width/3, -1);
    Rectangle s2 = UILayoutNext(&sl, sc.width/3, -1);
    Rectangle s3 = UILayoutNext(&sl, sc.width/3, -1);
    char buf[64]; Vector2 sz;

    snprintf(buf, sizeof(buf), u8"总单词数\n%d", g_wordProgressCount);
    sz = MeasureTextAuto(buf, 30, 1);
    DrawTextAuto(buf, (Vector2){s1.x+s1.width/2-sz.x/2, s1.y+35}, 30, 1, STYLE->theme.textPrimary);
    snprintf(buf, sizeof(buf), u8"已掌握\n%d", mastered);
    sz = MeasureTextAuto(buf, 30, 1);
    DrawTextAuto(buf, (Vector2){s2.x+s2.width/2-sz.x/2, s2.y+35}, 30, 1, STYLE->theme.success);
    snprintf(buf, sizeof(buf), u8"学习中\n%d", learning);
    sz = MeasureTextAuto(buf, 30, 1);
    DrawTextAuto(buf, (Vector2){s3.x+s3.width/2-sz.x/2, s3.y+35}, 30, 1, (Color){255, 165, 0, 255});

    Rectangle rb = UILayoutNext(&lay, -1, 55);
    rb.x = SCREEN_WIDTH - 400;
    rb.width = 200;
    if(UIButton(u8"清除进度", rb, STYLE, UI_STATE, 5)) { clearProgress(); }

    // 详细列表
    Rectangle lc = UILayoutNext(&lay, -1, -1);
    DrawRectangleRounded(lc, 0.1f, 12, STYLE->theme.panelBg);
    UILayout ll = UIBeginLayout(lc, UI_DIR_VERTICAL, 6, 18);

    Rectangle hr = UILayoutNext(&ll, -1, 45);
    DrawTextAuto(u8"单词", (Vector2){hr.x+15, hr.y+10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"认识", (Vector2){hr.x+240, hr.y+10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"不认识", (Vector2){hr.x+350, hr.y+10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"状态", (Vector2){hr.x+500, hr.y+10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"上次复习", (Vector2){hr.x+620, hr.y+10}, 22, 1, STYLE->theme.textSecondary);
    DrawLineEx((Vector2){hr.x, hr.y+40}, (Vector2){hr.x+hr.width, hr.y+40}, 1, STYLE->theme.inputBorder);

    Rectangle sa = UILayoutNext(&ll, -1, -1);
    UIScrollView sv = {0};
    sv.viewport = sa;
    sv.contentSize = (Vector2){sa.width, g_wordProgressCount * 50.0f};
    UIBeginScrollView(&sv, sa, sv.contentSize);

    for (int i = 0; i < g_wordProgressCount; i++) {
        Rectangle rr = {sa.x, sa.y + i*50 - sv.scrollOffset.y, sa.width, 50};
        if(i % 2 == 0) { DrawRectangleRec(rr, Fade(STYLE->theme.inputBg, 0.5f)); }
        DrawTextAuto(g_words[i].entry.word, (Vector2){rr.x+15, rr.y+14}, 22, 1, STYLE->theme.textPrimary);
        char ct[32];
        snprintf(ct, sizeof(ct), "%d", g_words[i].progress.knownCount);
        DrawTextAuto(ct, (Vector2){rr.x+250, rr.y+14}, 22, 1, STYLE->theme.success);
        snprintf(ct, sizeof(ct), "%d", g_words[i].progress.unknownCount);
        DrawTextAuto(ct, (Vector2){rr.x+360, rr.y+14}, 22, 1, STYLE->theme.error);
        DrawTextAuto(g_words[i].progress.mastered ? u8"已掌握" : u8"学习中",
            (Vector2){rr.x+500, rr.y+14}, 22, 1,
            g_words[i].progress.mastered ? STYLE->theme.success : (Color){255, 165, 0, 255});
        DrawTextAuto(formatTime(g_words[i].progress.lastReview),
            (Vector2){rr.x+620, rr.y+14}, 20, 1, STYLE->theme.textSecondary);
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
}
