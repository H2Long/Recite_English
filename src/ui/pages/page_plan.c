// ============================================================================
// 学习计划页面（父页 + 计划管理 + 学习进度）
// ============================================================================

#include "pages.h"

// 计划管理表单状态
static struct {
    char name[64]; int daily; int days;
    UITextBoxState nameState, dailyState, daysState;
    bool isAdding; char msg[128]; bool initialized;
} g_planForm = {0};

void MenuPlanRoot_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);
    Rectangle titleR = UILayoutNext(&layout, -1, 60);
    DrawTextAuto(u8"学习计划", (Vector2){cr.x + 50, titleR.y}, 42, 1, STYLE->theme.primary);
    LearningPlan* active = Plan_GetActive();
    if (active != NULL) {
        Rectangle infoR = UILayoutNext(&layout, -1, 80);
        DrawRectangleRounded(infoR, 0.1f, 8, STYLE->theme.panelBg);
        char info[256]; snprintf(info, sizeof(info), u8"当前计划: %s  |  第 %d/%d 天  |  今日 %d/%d 词",
            active->name, active->currentDay + 1, active->totalDays, active->studiedToday, active->dailyWordCount);
        DrawTextAuto(info, (Vector2){infoR.x + 20, infoR.y + 25}, 22, 1, STYLE->theme.primary);
        float prog = (float)active->studiedToday / (active->dailyWordCount > 0 ? active->dailyWordCount : 1);
        if (prog > 1.0f) prog = 1.0f;
        Rectangle barBg = {infoR.x + 20, infoR.y + 55, infoR.width - 40, 12};
        DrawRectangleRounded(barBg, 0.5f, 4, STYLE->theme.inputBg);
        DrawRectangleRounded((Rectangle){barBg.x, barBg.y, barBg.width * prog, barBg.height}, 0.5f, 4, STYLE->theme.success);
    } else {
        Rectangle noPlanR = UILayoutNext(&layout, -1, 60);
        DrawTextAuto(u8"暂未选择学习计划，请先创建或选择一个计划", (Vector2){noPlanR.x, noPlanR.y}, 22, 1, STYLE->theme.textSecondary);
    }
    Rectangle cardsR = UILayoutNext(&layout, -1, 200);
    UILayout cardsL = UIBeginLayout(cardsR, UI_DIR_HORIZONTAL, 30, 0);
    struct { const char* t; const char* d; Color c; } cards[] = {
        {u8"计划管理", u8"创建、选择、删除\n学习计划", (Color){70, 130, 180, 255}},
        {u8"学习进度", u8"查看每个单词的\n学习掌握情况", (Color){60, 179, 113, 255}},
    };
    for (int i = 0; i < 2; i++) {
        Rectangle cardR = UILayoutNext(&cardsL, 320, -1);
        DrawRectangleRounded(cardR, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardR, 0.1f, 12, cards[i].c);
        Vector2 ts = MeasureTextAuto(cards[i].t, 32, 1);
        DrawTextAuto(cards[i].t, (Vector2){cardR.x + 20, cardR.y + 20}, 32, 1, cards[i].c);
        Rectangle descA = {cardR.x + 20, cardR.y + 70, cardR.width - 40, cardR.height - 130};
        UIDrawTextRec(cards[i].d, descA, 22, 1, true, STYLE->theme.textSecondary);
        Rectangle goBtn = {cardR.x + cardR.width/2 - 60, cardR.y + cardR.height - 70, 120, 50};
        if (UIButton(cards[i].t, goBtn, STYLE, UI_STATE, 700 + i) && i < g_app.currentMenu->childindex) {
            MENU* target = g_app.currentMenu->child[i];
            if (target) { StackPush(AppState_GetMenuStack(), g_app.currentMenu); CURRENT_MENU = target; }
        }
    }
}

void MenuPlanManager_Show(void) {
    if (!g_planForm.initialized) { memset(&g_planForm, 0, sizeof(g_planForm)); g_planForm.initialized = true; g_planForm.daily = 10; g_planForm.days = 7; snprintf(g_planForm.nameState.buffer, 1023, "%s", u8"我的计划"); }
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_HORIZONTAL, 20, 25);
    Rectangle listR = UILayoutNext(&layout, 320, -1);
    DrawRectangleRounded(listR, 0.1f, 8, STYLE->theme.panelBg);
    Rectangle listTitle = {listR.x + 15, listR.y + 15, listR.width - 30, 35};
    DrawTextAuto(u8"我的计划", (Vector2){listTitle.x, listTitle.y}, 24, 1, STYLE->theme.textPrimary);
    static float planListScroll = 0.0f;
    float totalH = PLAN_STATE.planCount * 60.0f + 10;
    UIScrollView sv = {0}; Rectangle listArea = {listR.x, listR.y + 55, listR.width, listR.height - 55};
    sv.viewport = listArea; sv.scrollOffset.y = planListScroll; sv.contentSize = (Vector2){listArea.width - 20, totalH};
    UIBeginScrollView(&sv, listArea, sv.contentSize);
    for (int i = 0; i < PLAN_STATE.planCount; i++) {
        Rectangle item = {listArea.x + 10, listArea.y + 5 + i * 60 - sv.scrollOffset.y, listArea.width - 20, 55};
        bool isActive = (i == Plan_GetActiveIndex());
        if (isActive) {
            DrawRectangleRounded(item, 0.08f, 6, Fade(STYLE->theme.primary, 0.15f));
            DrawRectangleRoundedLines(item, 0.08f, 6, STYLE->theme.primary);
            DrawRectangleRec((Rectangle){item.x, item.y, 4, item.height}, STYLE->theme.primary);
        } else if (CheckCollisionPointRec(UI_STATE->mousePos, item)) DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.06f));
        if (CheckCollisionPointRec(UI_STATE->mousePos, item) && UI_STATE->mouseReleased && !isActive) Plan_SetActive(i);
        float nameX = item.x + 18;
        DrawTextAuto(PLAN_STATE.plans[i].name, (Vector2){nameX, item.y + 4}, 22, 1, isActive ? STYLE->theme.primary : STYLE->theme.textPrimary);
        if (isActive) DrawTextAuto(u8"▶ 当前使用", (Vector2){item.x + item.width - 100, item.y + 4}, 16, 1, STYLE->theme.primary);
        LearningPlan* p = &PLAN_STATE.plans[i];
        float dayProg = p->totalDays > 0 ? (float)(p->currentDay + 1) / p->totalDays : 0;
        if (dayProg > 1.0f) dayProg = 1.0f;
        char sub[128]; snprintf(sub, sizeof(sub), u8"%d词/天 × %d天  进度 %d%%", p->dailyWordCount, p->totalDays, (int)(dayProg * 100));
        DrawTextAuto(sub, (Vector2){nameX, item.y + 29}, 14, 1, STYLE->theme.textSecondary);
        Rectangle bar = {nameX, item.y + 48, item.width - nameX + item.x - listArea.x - 50, 4};
        DrawRectangleRec(bar, Fade(STYLE->theme.textSecondary, 0.2f));
        DrawRectangleRec((Rectangle){bar.x, bar.y, bar.width * dayProg, bar.height}, isActive ? STYLE->theme.primary : STYLE->theme.success);
        Rectangle delBtn = {item.x + item.width - 28, item.y + 4, 24, 22};
        bool delHover = CheckCollisionPointRec(UI_STATE->mousePos, delBtn);
        if (delHover) DrawRectangleRounded(delBtn, 0.2f, 4, Fade(RED, 0.2f));
        DrawTextAuto(u8"✕", (Vector2){delBtn.x + 6, delBtn.y + 2}, 16, 1, delHover ? RED : STYLE->theme.textSecondary);
        if (delHover && UI_STATE->mouseReleased) Plan_Delete(i);
    }
    UIEndScrollView(&sv, STYLE, UI_STATE); planListScroll = sv.scrollOffset.y;
    Rectangle formR = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(formR, 0.1f, 8, STYLE->theme.panelBg);
    UILayout fl = UIBeginLayout(formR, UI_DIR_VERTICAL, 15, 30);
    Rectangle ft = UILayoutNext(&fl, -1, 40); DrawTextAuto(u8"创建新计划", (Vector2){ft.x, ft.y}, 28, 1, STYLE->theme.primary);
    Rectangle l1 = UILayoutNext(&fl, -1, 28); DrawTextAuto(u8"计划名称", (Vector2){l1.x, l1.y}, 20, 1, STYLE->theme.textSecondary);
    Rectangle i1 = UILayoutNext(&fl, -1, 42); UITextBox(&g_planForm.nameState, i1, STYLE, UI_STATE, false);
    Rectangle l2 = UILayoutNext(&fl, -1, 28); DrawTextAuto(u8"每日单词数", (Vector2){l2.x, l2.y}, 20, 1, STYLE->theme.textSecondary);
    Rectangle i2 = UILayoutNext(&fl, -1, 42); UITextBox(&g_planForm.dailyState, i2, STYLE, UI_STATE, false);
    Rectangle l3 = UILayoutNext(&fl, -1, 28); DrawTextAuto(u8"总天数", (Vector2){l3.x, l3.y}, 20, 1, STYLE->theme.textSecondary);
    Rectangle i3 = UILayoutNext(&fl, -1, 42); UITextBox(&g_planForm.daysState, i3, STYLE, UI_STATE, false);
    if (strlen(g_planForm.msg) > 0) {
        Rectangle mr = UILayoutNext(&fl, -1, 30);
        DrawTextAuto(g_planForm.msg, (Vector2){mr.x, mr.y}, 18, 1, strstr(g_planForm.msg, "失败") ? STYLE->theme.error : STYLE->theme.success);
    }
    Rectangle btnR = UILayoutNext(&fl, -1, 50);
    UILayout bl = UIBeginLayout(btnR, UI_DIR_HORIZONTAL, 20, 0);
    Rectangle createBtn = UILayoutNext(&bl, 150, 45);
    if (UIButton(u8"创建计划", createBtn, STYLE, UI_STATE, 710)) {
        const char* name = g_planForm.nameState.buffer; int daily = atoi(g_planForm.dailyState.buffer); int days = atoi(g_planForm.daysState.buffer);
        if (strlen(name) == 0) snprintf(g_planForm.msg, sizeof(g_planForm.msg), "%s", u8"请输入计划名称");
        else if (daily <= 0 || days <= 0) snprintf(g_planForm.msg, sizeof(g_planForm.msg), "%s", u8"请输入有效的数字");
        else if (Plan_Create(name, daily, days)) { snprintf(g_planForm.msg, sizeof(g_planForm.msg), u8"计划创建成功！"); memset(&g_planForm.nameState, 0, sizeof(UITextBoxState)); memset(&g_planForm.dailyState, 0, sizeof(UITextBoxState)); memset(&g_planForm.daysState, 0, sizeof(UITextBoxState)); }
        else snprintf(g_planForm.msg, sizeof(g_planForm.msg), u8"创建失败：计划名已存在或已满");
    }
    Rectangle useBtn = UILayoutNext(&bl, 150, 45);
    if (UIButton(u8"使用默认", useBtn, STYLE, UI_STATE, 711)) { Plan_AddDefaults(); snprintf(g_planForm.msg, sizeof(g_planForm.msg), u8"已添加默认计划"); }
}

void MenuProgress_Show(void) {
    UILayout layout = UIBeginLayout((Rectangle){250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100}, UI_DIR_VERTICAL, 20, 25);
    Rectangle statsCard = UILayoutNext(&layout, -1, 130);
    DrawRectangleRounded(statsCard, 0.1f, 12, STYLE->theme.panelBg);
    int mastered = 0, learning = 0;
    for (int i = 0; i < g_wordProgressCount; i++) { if (g_words[i].progress.mastered) mastered++; else learning++; }
    UILayout statsLayout = UIBeginLayout(statsCard, UI_DIR_HORIZONTAL, 0, 0);
    Rectangle stat1 = UILayoutNext(&statsLayout, statsCard.width / 3, -1);
    Rectangle stat2 = UILayoutNext(&statsLayout, statsCard.width / 3, -1);
    Rectangle stat3 = UILayoutNext(&statsLayout, statsCard.width / 3, -1);
    char statText[64]; Vector2 sSize;
    snprintf(statText, sizeof(statText), u8"总单词数\n%d", g_wordProgressCount);
    sSize = MeasureTextAuto(statText, 30, 1); DrawTextAuto(statText, (Vector2){stat1.x + stat1.width/2 - sSize.x/2, stat1.y + 35}, 30, 1, STYLE->theme.textPrimary);
    snprintf(statText, sizeof(statText), u8"已掌握\n%d", mastered);
    sSize = MeasureTextAuto(statText, 30, 1); DrawTextAuto(statText, (Vector2){stat2.x + stat2.width/2 - sSize.x/2, stat2.y + 35}, 30, 1, STYLE->theme.success);
    snprintf(statText, sizeof(statText), u8"学习中\n%d", learning);
    sSize = MeasureTextAuto(statText, 30, 1); DrawTextAuto(statText, (Vector2){stat3.x + stat3.width/2 - sSize.x/2, stat3.y + 35}, 30, 1, (Color){255, 165, 0, 255});
    Rectangle resetBtn = UILayoutNext(&layout, -1, 55); resetBtn.x = SCREEN_WIDTH - 400; resetBtn.width = 200;
    if (UIButton(u8"清除进度", resetBtn, STYLE, UI_STATE, 5)) clearProgress();
    Rectangle listCard = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(listCard, 0.1f, 12, STYLE->theme.panelBg);
    UILayout listLayout = UIBeginLayout(listCard, UI_DIR_VERTICAL, 6, 18);
    Rectangle headerRect = UILayoutNext(&listLayout, -1, 45);
    DrawTextAuto(u8"单词", (Vector2){headerRect.x + 15, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"认识", (Vector2){headerRect.x + 240, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"不认识", (Vector2){headerRect.x + 350, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"状态", (Vector2){headerRect.x + 500, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawTextAuto(u8"上次复习", (Vector2){headerRect.x + 620, headerRect.y + 10}, 22, 1, STYLE->theme.textSecondary);
    DrawLineEx((Vector2){headerRect.x, headerRect.y + 40}, (Vector2){headerRect.x + headerRect.width, headerRect.y + 40}, 1, STYLE->theme.inputBorder);
    Rectangle scrollArea = UILayoutNext(&listLayout, -1, -1);
    UIScrollView sv = {0}; sv.viewport = scrollArea; sv.contentSize = (Vector2){scrollArea.width, g_wordProgressCount * 50.0f};
    UIBeginScrollView(&sv, scrollArea, sv.contentSize);
    for (int i = 0; i < g_wordProgressCount; i++) {
        Rectangle rowRect = {scrollArea.x, scrollArea.y + i * 50 - sv.scrollOffset.y, scrollArea.width, 50};
        if (i % 2 == 0) DrawRectangleRec(rowRect, Fade(STYLE->theme.inputBg, 0.5f));
        DrawTextAuto(g_words[i].entry.word, (Vector2){rowRect.x + 15, rowRect.y + 14}, 22, 1, STYLE->theme.textPrimary);
        char countText[32]; snprintf(countText, sizeof(countText), "%d", g_words[i].progress.knownCount);
        DrawTextAuto(countText, (Vector2){rowRect.x + 250, rowRect.y + 14}, 22, 1, STYLE->theme.success);
        snprintf(countText, sizeof(countText), "%d", g_words[i].progress.unknownCount);
        DrawTextAuto(countText, (Vector2){rowRect.x + 360, rowRect.y + 14}, 22, 1, STYLE->theme.error);
        const char* status = g_words[i].progress.mastered ? u8"已掌握" : u8"学习中";
        Color statusColor = g_words[i].progress.mastered ? STYLE->theme.success : (Color){255, 165, 0, 255};
        DrawTextAuto(status, (Vector2){rowRect.x + 500, rowRect.y + 14}, 22, 1, statusColor);
        DrawTextAuto(formatTime(g_words[i].progress.lastReview), (Vector2){rowRect.x + 620, rowRect.y + 14}, 20, 1, STYLE->theme.textSecondary);
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
}
