// 学习计划 — 计划管理+进度追踪

#include "pages.h"

// 计划管理表单
static struct {
    char name[64]; int daily, days;
    UITextBoxState nameState, dailyState, daysState;
    bool isAdding; char msg[128]; bool ready;
    int selectedIdx;              // 右侧详情区选中的计划
} g_planForm = {0};

void MenuPlanRoot_Show(void) {
    Rectangle cr = {280, 80, SCREEN_WIDTH - 300, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);

    Rectangle tr = UILayoutNext(&lay, -1, 60);
    DrawTextAuto(u8"学习计划", (Vector2){cr.x + 50, tr.y}, 42, 1, STYLE->theme.primary);

    LearningPlan* active = Plan_GetActive();
    if(active != NULL) {
        Rectangle ir = UILayoutNext(&lay, -1, 80);
        UIDrawCard(ir, 0.08f, STYLE);
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
        UIDrawCard(card, 0.08f, STYLE);
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
        g_planForm.selectedIdx = Plan_GetActiveIndex();
    }

    Rectangle cr = {280, 80, SCREEN_WIDTH - 300, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_HORIZONTAL, 20, 25);

    // ===== 左侧: 计划列表 =====
    Rectangle lr = UILayoutNext(&lay, 300, -1);
    UIDrawCard(lr, 0.08f, STYLE);
    DrawTextAuto(u8"我的计划", (Vector2){lr.x + 15, lr.y + 12}, 22, 1, STYLE->theme.textPrimary);

    static float planListScroll = 0;
    Rectangle la = {lr.x, lr.y + 45, lr.width, lr.height - 45};
    UIScrollView sv = {0};
    sv.viewport = la;
    sv.scrollOffset.y = planListScroll;
    sv.contentSize = (Vector2){la.width, PLAN_STATE.planCount * 50.0f + 10};
    UIBeginScrollView(&sv, la, sv.contentSize);

    for (int i = 0; i < PLAN_STATE.planCount; i++) {
        Rectangle item = {la.x + 6, la.y + 4 + i*50 - sv.scrollOffset.y, la.width - 12, 46};
        bool act = (i == Plan_GetActiveIndex());
        bool sel = (i == g_planForm.selectedIdx);

        // 背景
        if(sel) {
            DrawRectangleRounded(item, 0.08f, 6, Fade(STYLE->theme.primary, 0.2f));
            DrawRectangleRoundedLines(item, 0.08f, 6, STYLE->theme.primary);
        }
        else if(act) {
            DrawRectangleRounded(item, 0.08f, 6, Fade(STYLE->theme.success, 0.1f));
        }
        else if(CheckCollisionPointRec(UI_STATE->mousePos, item)) {
            DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.06f));
        }

        // 点击选中
        if(CheckCollisionPointRec(UI_STATE->mousePos, item) && UI_STATE->mouseReleased) {
            g_planForm.selectedIdx = i;
        }

        // 计划名称
        float nx = item.x + 12;
        DrawTextAuto(PLAN_STATE.plans[i].name, (Vector2){nx, item.y + 4}, 20, 1,
            sel ? STYLE->theme.primary : STYLE->theme.textPrimary);

        // 简要信息
        char sub[64];
        snprintf(sub, sizeof(sub), u8"%d词/天  %d天",
            PLAN_STATE.plans[i].dailyWordCount, PLAN_STATE.plans[i].totalDays);
        DrawTextAuto(sub, (Vector2){nx, item.y + 26}, 14, 1, STYLE->theme.textSecondary);

        // 当前使用标记
        if(act) {
            DrawTextAuto(u8"●", (Vector2){item.x + item.width - 20, item.y + 16}, 14, 1, STYLE->theme.success);
        }
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
    planListScroll = sv.scrollOffset.y;

    // ===== 右侧 =====
    Rectangle rr = UILayoutNext(&lay, -1, -1);
    UILayout rl = UIBeginLayout(rr, UI_DIR_VERTICAL, 15, 0);

    // ---- 上半: 选中计划详情 ----
    Rectangle detailR = UILayoutNext(&rl, -1, 300);
    UIDrawCard(detailR, 0.08f, STYLE);
    UILayout dl = UIBeginLayout(detailR, UI_DIR_VERTICAL, 12, 25);

    if(g_planForm.selectedIdx >= 0 && g_planForm.selectedIdx < PLAN_STATE.planCount) {
        LearningPlan* p = &PLAN_STATE.plans[g_planForm.selectedIdx];
        bool isActive = (g_planForm.selectedIdx == Plan_GetActiveIndex());

        // 标题
        Rectangle dt = UILayoutNext(&dl, -1, 40);
        DrawTextAuto(p->name, (Vector2){dt.x, dt.y}, 28, 1, STYLE->theme.primary);
        if(isActive) {
            DrawTextAuto(u8"  [当前使用]", (Vector2){dt.x + MeasureTextAuto(p->name, 28, 1).x + 10, dt.y + 6},
                18, 1, STYLE->theme.success);
        }

        // 参数
        Rectangle dp = UILayoutNext(&dl, -1, 30);
        char param[128];
        snprintf(param, sizeof(param), u8"每日目标: %d 词    总天数: %d 天    总词量: %d 词",
            p->dailyWordCount, p->totalDays, p->dailyWordCount * p->totalDays);
        DrawTextAuto(param, (Vector2){dp.x, dp.y}, 20, 1, STYLE->theme.textSecondary);

        // 进度
        Rectangle pg = UILayoutNext(&dl, -1, 30);
        float prog = p->totalDays > 0 ? (float)(p->currentDay + 1) / p->totalDays : 0;
        if(prog > 1) { prog = 1; }
        char progText[128];
        snprintf(progText, sizeof(progText), u8"进度: 第 %d/%d 天  (%d%%)    今日已学: %d/%d 词",
            p->currentDay + 1, p->totalDays, (int)(prog * 100),
            p->studiedToday, p->dailyWordCount);
        DrawTextAuto(progText, (Vector2){pg.x, pg.y}, 20, 1, STYLE->theme.textPrimary);

        // 进度条
        Rectangle barBg = {pg.x, pg.y + 32, detailR.width - 50, 14};
        DrawRectangleRounded(barBg, 0.5f, 4, STYLE->theme.inputBg);
        DrawRectangleRounded((Rectangle){barBg.x, barBg.y, barBg.width * prog, barBg.height},
            0.5f, 4, isActive ? STYLE->theme.primary : STYLE->theme.success);

        // 创建时间
        Rectangle ct = UILayoutNext(&dl, -1, 25);
        char timeStr[64];
        if(p->createdAt > 0) {
            strftime(timeStr, sizeof(timeStr), u8"创建时间: %Y-%m-%d %H:%M", localtime(&p->createdAt));
        } else {
            strcpy(timeStr, u8"创建时间: 未知");
        }
        DrawTextAuto(timeStr, (Vector2){ct.x, ct.y}, 16, 1, STYLE->theme.textSecondary);

        // 操作按钮
        Rectangle btnRow = UILayoutNext(&dl, -1, 45);
        UILayout bl = UIBeginLayout(btnRow, UI_DIR_HORIZONTAL, 15, 0);
        if(!isActive) {
            if(UIButton(u8"设为当前", UILayoutNext(&bl, 130, 40), STYLE, UI_STATE, 730)) {
                Plan_SetActive(g_planForm.selectedIdx);
                RefreshReviewList();
            }
        }
        if(UIButtonDanger(u8"删除", UILayoutNext(&bl, 100, 40), STYLE, UI_STATE, 731)) {
            Plan_Delete(g_planForm.selectedIdx);
            if(g_planForm.selectedIdx >= PLAN_STATE.planCount) {
                g_planForm.selectedIdx = PLAN_STATE.planCount > 0 ? PLAN_STATE.planCount - 1 : -1;
            }
        }
    }
    else {
        Rectangle hint = UILayoutNext(&dl, -1, 60);
        DrawTextAuto(u8"← 请从左侧选择一个计划查看详情",
            (Vector2){hint.x + 20, hint.y + 15}, 22, 1, STYLE->theme.textSecondary);
    }

    // ---- 下半: 创建新计划 ----
    Rectangle createR = UILayoutNext(&rl, -1, -1);
    UIDrawCard(createR, 0.08f, STYLE);
    UILayout cl = UIBeginLayout(createR, UI_DIR_VERTICAL, 12, 25);

    Rectangle ct2 = UILayoutNext(&cl, -1, 35);
    DrawTextAuto(u8"创建新计划", (Vector2){ct2.x, ct2.y}, 24, 1, STYLE->theme.textPrimary);

    // 计划名称
    Rectangle lblName = UILayoutNext(&cl, -1, 24);
    DrawTextAuto(u8"计划名称", (Vector2){lblName.x, lblName.y}, 18, 1, STYLE->theme.textSecondary);
    UITextBox(&g_planForm.nameState, UILayoutNext(&cl, -1, 38), STYLE, UI_STATE, false);

    // 每日背单词数
    Rectangle lblDaily = UILayoutNext(&cl, -1, 24);
    DrawTextAuto(u8"每日背单词数", (Vector2){lblDaily.x, lblDaily.y}, 18, 1, STYLE->theme.textSecondary);
    UITextBox(&g_planForm.dailyState, UILayoutNext(&cl, -1, 38), STYLE, UI_STATE, false);

    // 计划天数
    Rectangle lblDays = UILayoutNext(&cl, -1, 24);
    DrawTextAuto(u8"计划天数", (Vector2){lblDays.x, lblDays.y}, 18, 1, STYLE->theme.textSecondary);
    UITextBox(&g_planForm.daysState, UILayoutNext(&cl, -1, 38), STYLE, UI_STATE, false);

    // 提示消息
    if(strlen(g_planForm.msg) > 0) {
        Rectangle mr = UILayoutNext(&cl, -1, 25);
        DrawTextAuto(g_planForm.msg, (Vector2){mr.x, mr.y}, 18, 1,
            strstr(g_planForm.msg, "失败") ? STYLE->theme.error : STYLE->theme.success);
    }

    // 按钮行
    Rectangle btnRow = UILayoutNext(&cl, -1, 42);
    UILayout bl = UIBeginLayout(btnRow, UI_DIR_HORIZONTAL, 15, 0);
    if(UIButton(u8"创建计划", UILayoutNext(&bl, 130, 38), STYLE, UI_STATE, 710)) {
        const char* nm = g_planForm.nameState.buffer;
        int d = atoi(g_planForm.dailyState.buffer);
        int dd = atoi(g_planForm.daysState.buffer);
        if(strlen(nm) == 0) { strcpy(g_planForm.msg, u8"请输入计划名称"); }
        else if(d <= 0 || dd <= 0) { strcpy(g_planForm.msg, u8"请输入有效的数字"); }
        else if(Plan_Create(nm, d, dd)) {
            strcpy(g_planForm.msg, u8"创建成功！");
            memset(&g_planForm.nameState, 0, sizeof(UITextBoxState));
            memset(&g_planForm.dailyState, 0, sizeof(UITextBoxState));
            memset(&g_planForm.daysState, 0, sizeof(UITextBoxState));
        }
        else strcpy(g_planForm.msg, u8"创建失败：名称已存在或已满");
    }
    if(UIButton(u8"使用默认", UILayoutNext(&bl, 130, 38), STYLE, UI_STATE, 711)) {
        Plan_AddDefaults();
        strcpy(g_planForm.msg, u8"已添加默认计划");
    }
}

void MenuProgress_Show(void) {
    UILayout lay = UIBeginLayout((Rectangle){280, 80, SCREEN_WIDTH-300, SCREEN_HEIGHT-100},
        UI_DIR_VERTICAL, 20, 25);

    Rectangle sc = UILayoutNext(&lay, -1, 130);
    UIDrawCard(sc, 0.08f, STYLE);

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
    if(UIButtonDanger(u8"清除进度", rb, STYLE, UI_STATE, 5)) { clear_progress(); }

    // 详细列表
    Rectangle lc = UILayoutNext(&lay, -1, -1);
    UIDrawCard(lc, 0.08f, STYLE);
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
