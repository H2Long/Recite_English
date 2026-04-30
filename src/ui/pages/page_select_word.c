// ============================================================================
// 选词背单词页面 — 看释义选单词
// 功能：显示汉语释义作为题干，从 4 个英文选项中选正确的单词
//       得分按登录账号独立累计
// ============================================================================

#include "pages.h"

/**
 * MenuSelectWord_Show - 渲染选词背单词页面
 *
 * 两种状态：
 *   1. 正在答题 — 题干（汉语释义）→ 4个英文选项 → 进度 + 下一题
 *   2. 完成 — 结果面板：正确率、账号累计统计、评价、再来一次
 *
 * 选项生成逻辑与 MenuTest_Show 类似，但题干是汉语释义而选项是英文单词。
 * 每轮最多 15 题，按账号统计累计得分并保存到 accounts.txt。
 */
void MenuSelectWord_Show(void) {
    // ---- 完成或首次进入 ----
    if (SELECT_WORD.selectCount == 0 || SELECT_WORD.currentSelectIdx >= SELECT_WORD.selectCount) {
        if (SELECT_WORD.selectCount == 0) {
            SELECT_WORD.selectCount = (g_wordProgressCount < 15) ? g_wordProgressCount : 15;
            for (int j = 0; j < g_wordProgressCount; j++) SELECT_WORD.selectIndices[j] = j;
            shuffleArray(SELECT_WORD.selectIndices, g_wordProgressCount);
            SELECT_WORD.currentSelectIdx = 0; SELECT_WORD.selectCorrect = 0; SELECT_WORD.selectTotal = 0;
            SELECT_WORD.selectedAnswer = -1; SELECT_WORD.answerResult = -1;
        }
        if (SELECT_WORD.selectTotal > 0) {
            // 结果面板
            Rectangle rr = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 200, 500, 400};
            DrawRectangleRounded(rr, 0.1f, 12, STYLE->theme.panelBg);
            Vector2 ts = MeasureTextAuto(u8"练习完成！", 46, 1);
            DrawTextAuto(u8"练习完成！", (Vector2){SCREEN_WIDTH/2 - ts.x/2, rr.y + 35}, 46, 1, STYLE->theme.primary);

            float score = SELECT_WORD.selectTotal > 0
                ? (float)SELECT_WORD.selectCorrect / SELECT_WORD.selectTotal * 100 : 0;
            char st[128]; snprintf(st, sizeof(st), u8"正确率: %.0f%% (%d/%d)",
                score, SELECT_WORD.selectCorrect, SELECT_WORD.selectTotal);
            Vector2 ss = MeasureTextAuto(st, 34, 1);
            Color sc = score >= 80 ? STYLE->theme.success
                     : score >= 60 ? (Color){255, 165, 0, 255} : STYLE->theme.error;
            DrawTextAuto(st, (Vector2){SCREEN_WIDTH/2 - ss.x/2, rr.y + 110}, 34, 1, sc);

            // 更新账号累计统计
            if (Account_IsLoggedIn()) {
                int idx = Account_GetCurrentIndex();
                if (idx >= 0) {
                    ACCOUNT.users[idx].selectWordCorrect += SELECT_WORD.selectCorrect;
                    ACCOUNT.users[idx].selectWordTotal += SELECT_WORD.selectTotal;
                    Account_Save();
                }
            }

            const char* cmt = score >= 90 ? u8"太棒了！继续保持！"
                            : score >= 70 ? u8"不错！再接再厉！"
                            : score >= 50 ? u8"需要多复习哦！" : u8"建议先去学单词模式";
            Vector2 cs = MeasureTextAuto(cmt, 28, 1);
            DrawTextAuto(cmt, (Vector2){SCREEN_WIDTH/2 - cs.x/2, rr.y + 210}, 28, 1, STYLE->theme.textSecondary);

            Rectangle rb = {SCREEN_WIDTH/2 - 90, rr.y + 280, 180, 60};
            if (UIButton(u8"再来一次", rb, STYLE, UI_STATE, 8)) {
                SELECT_WORD.selectCount = 0; SELECT_WORD.currentSelectIdx = 0;
                SELECT_WORD.selectCorrect = 0; SELECT_WORD.selectTotal = 0;
                SELECT_WORD.selectedAnswer = -1; SELECT_WORD.answerResult = -1;
                memset(SELECT_WORD.wrongOptionsUsed, 0, sizeof(SELECT_WORD.wrongOptionsUsed));
            }
            return;
        }
    }

    // ---- 正在答题 ----
    int wordIdx = SELECT_WORD.selectIndices[SELECT_WORD.currentSelectIdx];
    WordEntry* cw = &g_words[wordIdx].entry;

    Rectangle qr = {SCREEN_WIDTH/2 - 250, 100, 500, 120};
    DrawRectangleRounded(qr, 0.1f, 8, STYLE->theme.panelBg);
    DrawTextAuto(u8"请选择正确的英文单词", (Vector2){SCREEN_WIDTH/2 - 160, qr.y + 12}, 20, 1, STYLE->theme.textSecondary);
    UIDrawTextRec(cw->definition, (Rectangle){qr.x + 25, qr.y + 40, qr.width - 50, 65}, 32, 1, true, STYLE->theme.textPrimary);

    // 生成选项
    const char* opts[4] = {0};
    static int wrongIdx[3] = {0}, wrongCnt = 0, lastIdx = -1;
    if (lastIdx != SELECT_WORD.currentSelectIdx) {
        SELECT_WORD.currentCorrectIdx = rand() % 4; wrongCnt = 0;
        for (int i = 0; i < g_wordProgressCount && wrongCnt < 3; i++)
            if (i != wordIdx && !SELECT_WORD.wrongOptionsUsed[i])
                wrongIdx[wrongCnt++] = i, SELECT_WORD.wrongOptionsUsed[i] = true;
        while (wrongCnt < 3) { int r = rand() % g_wordProgressCount; if (r != wordIdx) wrongIdx[wrongCnt++] = r; }
        lastIdx = SELECT_WORD.currentSelectIdx;
    }
    opts[SELECT_WORD.currentCorrectIdx] = cw->word;
    for (int i = 0, oi = 0; i < 4; i++)
        if (i != SELECT_WORD.currentCorrectIdx && oi < wrongCnt)
            opts[i] = g_words[wrongIdx[oi++]].entry.word;

    // 绘制选项
    UILayout ol = UIBeginLayout((Rectangle){SCREEN_WIDTH/2 - 250, 230, 500, 420}, UI_DIR_VERTICAL, 15, 0);
    for (int i = 0; i < 4; i++) {
        Rectangle or = UILayoutNext(&ol, -1, 85);
        Color bg = STYLE->theme.panelBg, bd = STYLE->theme.inputBorder;
        if (SELECT_WORD.answerResult != -1) {
            if (i == SELECT_WORD.currentCorrectIdx) bg = Fade(STYLE->theme.success, 0.3f), bd = STYLE->theme.success;
            else if (i == SELECT_WORD.selectedAnswer) bg = Fade(STYLE->theme.error, 0.3f), bd = STYLE->theme.error;
        } else if (SELECT_WORD.selectedAnswer == i) bg = Fade(STYLE->theme.primary, 0.3f), bd = STYLE->theme.primary;
        DrawRectangleRounded(or, 0.1f, 8, bg);
        DrawRectangleRoundedLines(or, 0.1f, 8, bd);
        char lb[32]; snprintf(lb, sizeof(lb), "%c. ", 'A' + i);
        Vector2 lsz = MeasureTextAuto(lb, 24, 1);
        DrawTextAuto(lb, (Vector2){or.x + 18, or.y + 22}, 24, 1, STYLE->theme.primary);
        UIDrawTextRec(opts[i], (Rectangle){or.x + 18 + lsz.x, or.y + 18,
            or.width - 36 - lsz.x, or.height - 30}, 30, 1, true, STYLE->theme.textPrimary);
        if (SELECT_WORD.answerResult == -1 && CheckCollisionPointRec(UI_STATE->mousePos, or) && UI_STATE->mouseReleased) {
            SELECT_WORD.selectedAnswer = i; SELECT_WORD.selectTotal++;
            if (i == SELECT_WORD.currentCorrectIdx) { SELECT_WORD.selectCorrect++; SELECT_WORD.answerResult = 1; }
            else SELECT_WORD.answerResult = 0;
        }
    }

    Rectangle pr = {SCREEN_WIDTH/2 - 150, 660, 300, 40};
    int rate = SELECT_WORD.selectTotal > 0 ? (int)((float)SELECT_WORD.selectCorrect / SELECT_WORD.selectTotal * 100.0f) : 0;
    char pt[64]; snprintf(pt, sizeof(pt), u8"进度: %d / %d  正确率: %d%%",
        SELECT_WORD.currentSelectIdx + 1, SELECT_WORD.selectCount, rate);
    Vector2 psz = MeasureTextAuto(pt, 22, 1);
    DrawTextAuto(pt, (Vector2){SCREEN_WIDTH/2 - psz.x/2, pr.y + 8}, 22, 1, STYLE->theme.textSecondary);

    if (SELECT_WORD.answerResult != -1) {
        Rectangle nb = {SCREEN_WIDTH/2 - 75, 710, 150, 55};
        if (UIButton(u8"下一题", nb, STYLE, UI_STATE, 9))
            { SELECT_WORD.currentSelectIdx++; SELECT_WORD.selectedAnswer = -1; SELECT_WORD.answerResult = -1; }
    }
}
