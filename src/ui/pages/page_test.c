// ============================================================================
// 测试模式页面 — 看单词选释义
// 功能：显示英文单词，从 4 个选项中选出正确的汉语释义
//       完成后显示正确率和评价
// ============================================================================

#include "pages.h"

/**
 * MenuTest_Show - 渲染测试模式页面
 *
 * 两种状态：
 *   1. 正在答题 — 显示题干（单词）→ 4个释义选项 → 进度
 *   2. 测试完成 — 显示结果面板（正确率、评价、再来一次按钮）
 *
 * 选项生成：正确答案随机放在 A/B/C/D 中一个位置，
 * 其余选项从其他单词中随机选取且不重复。
 */
void MenuTest_Show(void) {
    // ---- 测试完成：显示结果面板 ----
    if (TEST.testCount == 0 || TEST.currentTestIdx >= TEST.testCount) {
        Rectangle rr = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 180, 500, 360};
        DrawRectangleRounded(rr, 0.1f, 12, STYLE->theme.panelBg);

        Vector2 ts = MeasureTextAuto(u8"测试完成！", 46, 1);
        DrawTextAuto(u8"测试完成！", (Vector2){SCREEN_WIDTH/2 - ts.x/2, rr.y + 35}, 46, 1, STYLE->theme.primary);

        float score = TEST.testTotal > 0 ? (float)TEST.testCorrect / TEST.testTotal * 100 : 0;
        char st[128];
        snprintf(st, sizeof(st), u8"正确率: %.0f%% (%d/%d)", score, TEST.testCorrect, TEST.testTotal);
        Vector2 ss = MeasureTextAuto(st, 34, 1);
        Color sc = score >= 80 ? STYLE->theme.success : (score >= 60 ? (Color){255, 165, 0, 255} : STYLE->theme.error);
        DrawTextAuto(st, (Vector2){SCREEN_WIDTH/2 - ss.x/2, rr.y + 110}, 34, 1, sc);

        const char* cmt = score >= 90 ? u8"太棒了！继续保持！"
                        : score >= 70 ? u8"不错！再接再厉！"
                        : score >= 50 ? u8"需要多复习哦！"
                        : u8"建议先去学单词模式";
        Vector2 cs = MeasureTextAuto(cmt, 28, 1);
        DrawTextAuto(cmt, (Vector2){SCREEN_WIDTH/2 - cs.x/2, rr.y + 170}, 28, 1, STYLE->theme.textSecondary);

        Rectangle rb = {SCREEN_WIDTH/2 - 90, rr.y + 250, 180, 60};
        if (UIButton(u8"再来一次", rb, STYLE, UI_STATE, 3)) {
            for (int j = 0; j < g_wordProgressCount; j++) TEST.testIndices[j] = j;
            shuffleArray(TEST.testIndices, g_wordProgressCount);
            TEST.currentTestIdx = 0; TEST.testCorrect = 0; TEST.testTotal = 0;
            TEST.selectedAnswer = -1; TEST.answerResult = -1;
            memset(TEST.wrongOptionsUsed, 0, sizeof(TEST.wrongOptionsUsed));
        }
        return;
    }

    // ---- 正在答题 ----
    int wordIdx = TEST.testIndices[TEST.currentTestIdx];
    WordEntry* cw = &g_words[wordIdx].entry;

    // 题干
    Rectangle qr = {SCREEN_WIDTH/2 - 250, 100, 500, 110};
    DrawRectangleRounded(qr, 0.1f, 8, STYLE->theme.panelBg);
    char q[128]; snprintf(q, sizeof(q), u8"单词 \"%s\" 的正确释义是？", cw->word);
    Vector2 qs = MeasureTextAuto(q, 32, 1);
    DrawTextAuto(q, (Vector2){SCREEN_WIDTH/2 - qs.x/2, qr.y + 35}, 32, 1, STYLE->theme.textPrimary);

    // 生成 4 个选项（1 个正确 + 3 个干扰项）
    const char* opts[4] = {0};
    static int wrongIdx[3] = {0}, wrongCnt = 0, lastIdx = -1;
    if (lastIdx != TEST.currentTestIdx) {
        TEST.currentCorrectIdx = rand() % 4; wrongCnt = 0;
        for (int i = 0; i < g_wordProgressCount && wrongCnt < 3; i++)
            if (i != wordIdx && !TEST.wrongOptionsUsed[i])
                wrongIdx[wrongCnt++] = i, TEST.wrongOptionsUsed[i] = true;
        while (wrongCnt < 3) { int r = rand() % g_wordProgressCount; if (r != wordIdx) wrongIdx[wrongCnt++] = r; }
        lastIdx = TEST.currentTestIdx;
    }
    opts[TEST.currentCorrectIdx] = cw->definition;
    for (int i = 0, oi = 0; i < 4; i++)
        if (i != TEST.currentCorrectIdx && oi < wrongCnt)
            opts[i] = g_words[wrongIdx[oi++]].entry.definition;

    // 绘制 4 个选项
    UILayout ol = UIBeginLayout((Rectangle){SCREEN_WIDTH/2 - 250, 220, 500, 420}, UI_DIR_VERTICAL, 15, 0);
    for (int i = 0; i < 4; i++) {
        Rectangle or = UILayoutNext(&ol, -1, 85);
        Color bg = STYLE->theme.panelBg, bd = STYLE->theme.inputBorder;
        if (TEST.answerResult != -1) {
            if (i == TEST.currentCorrectIdx)       bg = Fade(STYLE->theme.success, 0.3f), bd = STYLE->theme.success;
            else if (i == TEST.selectedAnswer)     bg = Fade(STYLE->theme.error, 0.3f), bd = STYLE->theme.error;
        } else if (TEST.selectedAnswer == i)       bg = Fade(STYLE->theme.primary, 0.3f), bd = STYLE->theme.primary;
        DrawRectangleRounded(or, 0.1f, 8, bg);
        DrawRectangleRoundedLines(or, 0.1f, 8, bd);

        char lb[32]; snprintf(lb, sizeof(lb), "%c. ", 'A' + i);
        Vector2 lsz = MeasureTextAuto(lb, 24, 1);
        DrawTextAuto(lb, (Vector2){or.x + 18, or.y + 22}, 24, 1, STYLE->theme.primary);
        UIDrawTextRec(opts[i], (Rectangle){or.x + 18 + lsz.x, or.y + 18,
            or.width - 36 - lsz.x, or.height - 30}, 30, 1, true, STYLE->theme.textPrimary);

        if (TEST.answerResult == -1 && CheckCollisionPointRec(UI_STATE->mousePos, or) && UI_STATE->mouseReleased) {
            TEST.selectedAnswer = i; TEST.testTotal++;
            if (i == TEST.currentCorrectIdx) { TEST.testCorrect++; TEST.answerResult = 1; }
            else TEST.answerResult = 0;
        }
    }

    // 进度 + 下一题按钮
    Rectangle pr = {SCREEN_WIDTH/2 - 150, 660, 300, 40};
    int rate = TEST.testTotal > 0 ? (int)((float)TEST.testCorrect / TEST.testTotal * 100.0f) : 0;
    char pt[64]; snprintf(pt, sizeof(pt), u8"进度: %d / %d  正确率: %d%%",
        TEST.currentTestIdx + 1, TEST.testCount, rate);
    Vector2 psz = MeasureTextAuto(pt, 22, 1);
    DrawTextAuto(pt, (Vector2){SCREEN_WIDTH/2 - psz.x/2, pr.y + 8}, 22, 1, STYLE->theme.textSecondary);

    if (TEST.answerResult != -1) {
        Rectangle nb = {SCREEN_WIDTH/2 - 75, 710, 150, 55};
        if (UIButton(u8"下一题", nb, STYLE, UI_STATE, 4))
            { TEST.currentTestIdx++; TEST.selectedAnswer = -1; TEST.answerResult = -1; }
    }
}
