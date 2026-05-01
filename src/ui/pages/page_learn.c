// 学单词 — 左侧列表+右侧详情

#include "pages.h"

void MenuLearn_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_HORIZONTAL, 25, 0);

    // ---- 左侧: 单词列表 ----
    Rectangle la = UILayoutNext(&lay, 350, -1);
    DrawRectangleRounded(la, 0.1f, 8, STYLE->theme.panelBg);

    UICheckbox(u8"只显示未掌握", (Rectangle){la.x + 15, la.y + 15, 220, 35},
        &LEARN.learnFilterUnknown, STYLE, UI_STATE);

    Rectangle svR = {la.x, la.y + 60, la.width, la.height - 60};
    UIScrollView sv = {0};
    sv.scrollOffset.y = LEARN.learnScrollOffset;

    int mastered = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if(g_words[i].progress.mastered) { mastered++; }
    }
    int vis = LEARN.learnFilterUnknown ? g_wordProgressCount - mastered : g_wordProgressCount;
    sv.contentSize = (Vector2){svR.width, vis * 60.0f};
    UIBeginScrollView(&sv, svR, sv.contentSize);

    int li = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if(LEARN.learnFilterUnknown && g_words[i].progress.mastered) { continue; }
        Rectangle ir = {svR.x, svR.y + li*60 - sv.scrollOffset.y, svR.width, 60};
        float rate = (float)g_words[i].progress.knownCount
            / (g_words[i].progress.knownCount + g_words[i].progress.unknownCount + 1);
        Color ind = getMasteryColor(rate);
        DrawRectangleRec((Rectangle){ir.x + 5, ir.y + 12, 5, ir.height - 24}, ind);
        if(i == LEARN.learnIndex) { DrawRectangleRec(ir, Fade(STYLE->theme.primary, 0.2f)); }
        if(UIListItem(g_words[i].entry.word, ir, STYLE, UI_STATE)) { LEARN.learnIndex = i; }
        li++;
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
    LEARN.learnScrollOffset = sv.scrollOffset.y;

    // ---- 右侧: 单词详情 ----
    Rectangle da = UILayoutNext(&lay, -1, -1);
    WordEntry* e = &g_words[LEARN.learnIndex].entry;
    WordProgress* p = &g_words[LEARN.learnIndex].progress;
    DrawRectangleRounded(da, 0.1f, 12, STYLE->theme.panelBg);
    UILayout dl = UIBeginLayout(da, UI_DIR_VERTICAL, 20, 30);

    // 单词
    Rectangle wr = UILayoutNext(&dl, -1, 70);
    DrawTextAuto(e->word, (Vector2){wr.x, wr.y}, 56, 1, STYLE->theme.primary);

    // 音标
    if(e->phonetic && *e->phonetic) {
        Rectangle pr = UILayoutNext(&dl, -1, 45);
        DrawTextAuto(e->phonetic, (Vector2){wr.x, pr.y}, 32, 1, STYLE->theme.textSecondary);
    }

    // 分隔线
    Rectangle lr = UILayoutNext(&dl, -1, 3);
    DrawLineEx((Vector2){lr.x, lr.y}, (Vector2){lr.x + lr.width, lr.y}, 1, STYLE->theme.inputBorder);

    // 释义
    Rectangle dlr = UILayoutNext(&dl, -1, 40);
    DrawTextAuto(u8"释义", (Vector2){dlr.x, dlr.y}, 28, 1, STYLE->theme.textSecondary);
    Rectangle dfr = UILayoutNext(&dl, -1, 80);
    UIDrawTextRec(e->definition, dfr, 30, 1, true, STYLE->theme.textPrimary);

    // 例句
    Rectangle elr = UILayoutNext(&dl, -1, 40);
    DrawTextAuto(u8"例句", (Vector2){elr.x, elr.y}, 28, 1, STYLE->theme.textSecondary);
    if(e->example && *e->example) {
        Rectangle er = UILayoutNext(&dl, -1, 90);
        char buf[512];
        snprintf(buf, sizeof(buf), u8"例: %s", e->example);
        UIDrawTextRec(buf, er, 26, 1, true, STYLE->theme.textSecondary);
    }

    // 学习统计
    Rectangle sr = UILayoutNext(&dl, -1, 90);
    DrawRectangleRounded(sr, 0.1f, 8, STYLE->theme.inputBg);
    UILayout sl = UIBeginLayout(sr, UI_DIR_HORIZONTAL, 40, 20);
    Rectangle si = UILayoutNext(&sl, -1, -1);
    char stt[256];
    snprintf(stt, sizeof(stt), u8"认识: %d次  |  不认识: %d次  |  状态: %s  |  上次复习: %s",
        p->knownCount, p->unknownCount,
        p->mastered ? u8"已掌握" : u8"学习中", formatTime(p->lastReview));
    UIDrawTextRec(stt, (Rectangle){si.x, si.y + 20, si.width, si.height}, 22, 1, true, STYLE->theme.textPrimary);

    // 导航按钮
    Rectangle nr = UILayoutNext(&dl, -1, 60);
    UILayout nl = UIBeginLayout(nr, UI_DIR_HORIZONTAL, 25, 0);
    if(UIButton(u8"上一个", UILayoutNext(&nl, 120, 50), STYLE, UI_STATE, 1) && LEARN.learnIndex > 0) {
        LEARN.learnIndex--;
    }
    if(UIButton(u8"下一个", UILayoutNext(&nl, 120, 50), STYLE, UI_STATE, 2)
        && LEARN.learnIndex < g_wordProgressCount - 1) {
        LEARN.learnIndex++;
    }
}
