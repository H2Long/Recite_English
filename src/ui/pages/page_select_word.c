// ============================================================================
// 选词背单词页面 — 看释义选单词
// ============================================================================

#include "pages.h"

void MenuSelectWord_Show(void) {
    if (SELECT_WORD.selectCount == 0 || SELECT_WORD.currentSelectIdx >= SELECT_WORD.selectCount) {
        if (SELECT_WORD.selectCount == 0) {
            SELECT_WORD.selectCount = (g_wordProgressCount < 15) ? g_wordProgressCount : 15;
            for (int j = 0; j < g_wordProgressCount; j++) SELECT_WORD.selectIndices[j] = j;
            shuffleArray(SELECT_WORD.selectIndices, g_wordProgressCount);
            SELECT_WORD.currentSelectIdx = 0; SELECT_WORD.selectCorrect = 0; SELECT_WORD.selectTotal = 0;
            SELECT_WORD.selectedAnswer = -1; SELECT_WORD.answerResult = -1;
        }
        if (SELECT_WORD.selectTotal > 0) {
            Rectangle resultRect = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 200, 500, 400};
            DrawRectangleRounded(resultRect, 0.1f, 12, STYLE->theme.panelBg);
            Vector2 tSize = MeasureTextAuto(u8"练习完成！", 46, 1);
            DrawTextAuto(u8"练习完成！", (Vector2){SCREEN_WIDTH/2 - tSize.x/2, resultRect.y + 35}, 46, 1, STYLE->theme.primary);
            float score = SELECT_WORD.selectTotal > 0 ? (float)SELECT_WORD.selectCorrect / SELECT_WORD.selectTotal * 100 : 0;
            char scoreText[128];
            snprintf(scoreText, sizeof(scoreText), u8"正确率: %.0f%% (%d/%d)", score, SELECT_WORD.selectCorrect, SELECT_WORD.selectTotal);
            Vector2 sSize = MeasureTextAuto(scoreText, 34, 1);
            Color scoreColor = score >= 80 ? STYLE->theme.success : (score >= 60 ? (Color){255, 165, 0, 255} : STYLE->theme.error);
            DrawTextAuto(scoreText, (Vector2){SCREEN_WIDTH/2 - sSize.x/2, resultRect.y + 110}, 34, 1, scoreColor);
            if (Account_IsLoggedIn()) {
                int idx = Account_GetCurrentIndex();
                if (idx >= 0) { ACCOUNT.users[idx].selectWordCorrect += SELECT_WORD.selectCorrect; ACCOUNT.users[idx].selectWordTotal += SELECT_WORD.selectTotal; Account_Save(); }
                char accountStats[128];
                snprintf(accountStats, sizeof(accountStats), u8"账号累计: %d/%d (%.0f%%)",
                    ACCOUNT.users[idx].selectWordCorrect, ACCOUNT.users[idx].selectWordTotal,
                    ACCOUNT.users[idx].selectWordTotal > 0 ? (float)ACCOUNT.users[idx].selectWordCorrect / ACCOUNT.users[idx].selectWordTotal * 100 : 0.0f);
                Vector2 aSize = MeasureTextAuto(accountStats, 22, 1);
                DrawTextAuto(accountStats, (Vector2){SCREEN_WIDTH/2 - aSize.x/2, resultRect.y + 165}, 22, 1, STYLE->theme.textSecondary);
            }
            const char* comment = score >= 90 ? u8"太棒了！继续保持！" : score >= 70 ? u8"不错！再接再厉！" : score >= 50 ? u8"需要多复习哦！" : u8"建议先去学单词模式";
            Vector2 cSize = MeasureTextAuto(comment, 28, 1);
            DrawTextAuto(comment, (Vector2){SCREEN_WIDTH/2 - cSize.x/2, resultRect.y + 210}, 28, 1, STYLE->theme.textSecondary);
            Rectangle restartBtn = {SCREEN_WIDTH/2 - 90, resultRect.y + 280, 180, 60};
            if (UIButton(u8"再来一次", restartBtn, STYLE, UI_STATE, 8)) {
                SELECT_WORD.selectCount = 0; SELECT_WORD.currentSelectIdx = 0; SELECT_WORD.selectCorrect = 0;
                SELECT_WORD.selectTotal = 0; SELECT_WORD.selectedAnswer = -1; SELECT_WORD.answerResult = -1;
                memset(SELECT_WORD.wrongOptionsUsed, 0, sizeof(SELECT_WORD.wrongOptionsUsed));
            }
            return;
        }
    }
    int wordIdx = SELECT_WORD.selectIndices[SELECT_WORD.currentSelectIdx];
    WordEntry* currentWord = &g_words[wordIdx].entry;
    Rectangle qRect = {SCREEN_WIDTH/2 - 250, 100, 500, 120};
    DrawRectangleRounded(qRect, 0.1f, 8, STYLE->theme.panelBg);
    DrawTextAuto(u8"请选择正确的英文单词", (Vector2){SCREEN_WIDTH/2 - 160, qRect.y + 12}, 20, 1, STYLE->theme.textSecondary);
    Rectangle defRect = {qRect.x + 25, qRect.y + 40, qRect.width - 50, 65};
    UIDrawTextRec(currentWord->definition, defRect, 32, 1, true, STYLE->theme.textPrimary);
    const char* options[4] = {0};
    static int wrongOptions[3] = {0}; static int wrongCount = 0; static int lastSelectIdx = -1;
    if (lastSelectIdx != SELECT_WORD.currentSelectIdx) {
        SELECT_WORD.currentCorrectIdx = rand() % 4; wrongCount = 0;
        for (int i = 0; i < g_wordProgressCount && wrongCount < 3; i++)
            if (i != wordIdx && !SELECT_WORD.wrongOptionsUsed[i]) { wrongOptions[wrongCount++] = i; SELECT_WORD.wrongOptionsUsed[i] = true; }
        while (wrongCount < 3) { int idx = rand() % g_wordProgressCount; if (idx != wordIdx) wrongOptions[wrongCount++] = idx; }
        lastSelectIdx = SELECT_WORD.currentSelectIdx;
    }
    options[SELECT_WORD.currentCorrectIdx] = currentWord->word;
    int optIdx = 0;
    for (int i = 0; i < 4; i++) if (i != SELECT_WORD.currentCorrectIdx && optIdx < wrongCount) options[i] = g_words[wrongOptions[optIdx++]].entry.word;
    UILayout optLayout = UIBeginLayout((Rectangle){SCREEN_WIDTH/2 - 250, 230, 500, 420}, UI_DIR_VERTICAL, 15, 0);
    for (int i = 0; i < 4; i++) {
        Rectangle optRect = UILayoutNext(&optLayout, -1, 85);
        Color bgColor = STYLE->theme.panelBg; Color borderColor = STYLE->theme.inputBorder;
        if (SELECT_WORD.answerResult != -1) {
            if (i == SELECT_WORD.currentCorrectIdx) { bgColor = Fade(STYLE->theme.success, 0.3f); borderColor = STYLE->theme.success; }
            else if (i == SELECT_WORD.selectedAnswer) { bgColor = Fade(STYLE->theme.error, 0.3f); borderColor = STYLE->theme.error; }
        } else if (SELECT_WORD.selectedAnswer == i) { bgColor = Fade(STYLE->theme.primary, 0.3f); borderColor = STYLE->theme.primary; }
        DrawRectangleRounded(optRect, 0.1f, 8, bgColor);
        DrawRectangleRoundedLines(optRect, 0.1f, 8, borderColor);
        char optLabel[32]; snprintf(optLabel, sizeof(optLabel), "%c. ", 'A' + i);
        Vector2 labelSize = MeasureTextAuto(optLabel, 24, 1);
        DrawTextAuto(optLabel, (Vector2){optRect.x + 18, optRect.y + 22}, 24, 1, STYLE->theme.primary);
        Rectangle textRect = {optRect.x + 18 + labelSize.x, optRect.y + 18, optRect.width - 36 - labelSize.x, optRect.height - 30};
        UIDrawTextRec(options[i], textRect, 30, 1, true, STYLE->theme.textPrimary);
        if (SELECT_WORD.answerResult == -1 && CheckCollisionPointRec(UI_STATE->mousePos, optRect) && UI_STATE->mouseReleased) {
            SELECT_WORD.selectedAnswer = i; SELECT_WORD.selectTotal++;
            if (i == SELECT_WORD.currentCorrectIdx) { SELECT_WORD.selectCorrect++; SELECT_WORD.answerResult = 1; }
            else { SELECT_WORD.answerResult = 0; }
        }
    }
    Rectangle progressRect = {SCREEN_WIDTH/2 - 150, 660, 300, 40};
    int selectRate = SELECT_WORD.selectTotal > 0 ? (int)((float)SELECT_WORD.selectCorrect / SELECT_WORD.selectTotal * 100.0f) : 0;
    char progressText[64];
    snprintf(progressText, sizeof(progressText), u8"进度: %d / %d  正确率: %d%%", SELECT_WORD.currentSelectIdx + 1, SELECT_WORD.selectCount, selectRate);
    Vector2 pSize = MeasureTextAuto(progressText, 22, 1);
    DrawTextAuto(progressText, (Vector2){SCREEN_WIDTH/2 - pSize.x/2, progressRect.y + 8}, 22, 1, STYLE->theme.textSecondary);
    if (SELECT_WORD.answerResult != -1) {
        Rectangle nextBtn = {SCREEN_WIDTH/2 - 75, 710, 150, 55};
        if (UIButton(u8"下一题", nextBtn, STYLE, UI_STATE, 9)) { SELECT_WORD.currentSelectIdx++; SELECT_WORD.selectedAnswer = -1; SELECT_WORD.answerResult = -1; }
    }
}
