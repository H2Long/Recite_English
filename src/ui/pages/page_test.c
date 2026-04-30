// ============================================================================
// 测试模式页面 — 看单词选释义
// ============================================================================

#include "pages.h"

void MenuTest_Show(void) {
    if (TEST.testCount == 0 || TEST.currentTestIdx >= TEST.testCount) {
        Rectangle resultRect = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 180, 500, 360};
        DrawRectangleRounded(resultRect, 0.1f, 12, STYLE->theme.panelBg);
        Vector2 tSize = MeasureTextAuto(u8"测试完成！", 46, 1);
        DrawTextAuto(u8"测试完成！", (Vector2){SCREEN_WIDTH/2 - tSize.x/2, resultRect.y + 35}, 46, 1, STYLE->theme.primary);
        float score = TEST.testTotal > 0 ? (float)TEST.testCorrect / TEST.testTotal * 100 : 0;
        char scoreText[128]; snprintf(scoreText, sizeof(scoreText), u8"正确率: %.0f%% (%d/%d)", score, TEST.testCorrect, TEST.testTotal);
        Vector2 sSize = MeasureTextAuto(scoreText, 34, 1);
        Color scoreColor = score >= 80 ? STYLE->theme.success : (score >= 60 ? (Color){255, 165, 0, 255} : STYLE->theme.error);
        DrawTextAuto(scoreText, (Vector2){SCREEN_WIDTH/2 - sSize.x/2, resultRect.y + 110}, 34, 1, scoreColor);
        const char* comment = score >= 90 ? u8"太棒了！继续保持！" : score >= 70 ? u8"不错！再接再厉！" : score >= 50 ? u8"需要多复习哦！" : u8"建议先去学单词模式";
        Vector2 cSize = MeasureTextAuto(comment, 28, 1);
        DrawTextAuto(comment, (Vector2){SCREEN_WIDTH/2 - cSize.x/2, resultRect.y + 170}, 28, 1, STYLE->theme.textSecondary);
        Rectangle restartBtn = {SCREEN_WIDTH/2 - 90, resultRect.y + 250, 180, 60};
        if (UIButton(u8"再来一次", restartBtn, STYLE, UI_STATE, 3)) {
            for (int j = 0; j < g_wordProgressCount; j++) TEST.testIndices[j] = j;
            shuffleArray(TEST.testIndices, g_wordProgressCount);
            TEST.currentTestIdx = 0; TEST.testCorrect = 0; TEST.testTotal = 0;
            TEST.selectedAnswer = -1; TEST.answerResult = -1;
            memset(TEST.wrongOptionsUsed, 0, sizeof(TEST.wrongOptionsUsed));
        }
        return;
    }
    int wordIdx = TEST.testIndices[TEST.currentTestIdx];
    WordEntry* currentWord = &g_words[wordIdx].entry;
    Rectangle qRect = {SCREEN_WIDTH/2 - 250, 100, 500, 110};
    DrawRectangleRounded(qRect, 0.1f, 8, STYLE->theme.panelBg);
    char question[128]; snprintf(question, sizeof(question), u8"单词 \"%s\" 的正确释义是？", currentWord->word);
    Vector2 qSize = MeasureTextAuto(question, 32, 1);
    DrawTextAuto(question, (Vector2){SCREEN_WIDTH/2 - qSize.x/2, qRect.y + 35}, 32, 1, STYLE->theme.textPrimary);
    const char* options[4] = {0};
    static int wrongOptions[3] = {0}; static int wrongCount = 0; static int lastTestIdx = -1;
    if (lastTestIdx != TEST.currentTestIdx) {
        TEST.currentCorrectIdx = rand() % 4; wrongCount = 0;
        for (int i = 0; i < g_wordProgressCount && wrongCount < 3; i++)
            if (i != wordIdx && !TEST.wrongOptionsUsed[i]) { wrongOptions[wrongCount++] = i; TEST.wrongOptionsUsed[i] = true; }
        while (wrongCount < 3) { int idx = rand() % g_wordProgressCount; if (idx != wordIdx) wrongOptions[wrongCount++] = idx; }
        lastTestIdx = TEST.currentTestIdx;
    }
    options[TEST.currentCorrectIdx] = currentWord->definition;
    int optIdx = 0;
    for (int i = 0; i < 4; i++) if (i != TEST.currentCorrectIdx && optIdx < wrongCount) options[i] = g_words[wrongOptions[optIdx++]].entry.definition;
    UILayout optLayout = UIBeginLayout((Rectangle){SCREEN_WIDTH/2 - 250, 220, 500, 420}, UI_DIR_VERTICAL, 15, 0);
    for (int i = 0; i < 4; i++) {
        Rectangle optRect = UILayoutNext(&optLayout, -1, 85);
        Color bgColor = STYLE->theme.panelBg, borderColor = STYLE->theme.inputBorder;
        if (TEST.answerResult != -1) {
            if (i == TEST.currentCorrectIdx) { bgColor = Fade(STYLE->theme.success, 0.3f); borderColor = STYLE->theme.success; }
            else if (i == TEST.selectedAnswer) { bgColor = Fade(STYLE->theme.error, 0.3f); borderColor = STYLE->theme.error; }
        } else if (TEST.selectedAnswer == i) { bgColor = Fade(STYLE->theme.primary, 0.3f); borderColor = STYLE->theme.primary; }
        DrawRectangleRounded(optRect, 0.1f, 8, bgColor);
        DrawRectangleRoundedLines(optRect, 0.1f, 8, borderColor);
        char optLabel[32]; snprintf(optLabel, sizeof(optLabel), "%c. ", 'A' + i);
        Vector2 labelSize = MeasureTextAuto(optLabel, 24, 1);
        DrawTextAuto(optLabel, (Vector2){optRect.x + 18, optRect.y + 22}, 24, 1, STYLE->theme.primary);
        Rectangle textRect = {optRect.x + 18 + labelSize.x, optRect.y + 18, optRect.width - 36 - labelSize.x, optRect.height - 30};
        UIDrawTextRec(options[i], textRect, 30, 1, true, STYLE->theme.textPrimary);
        if (TEST.answerResult == -1 && CheckCollisionPointRec(UI_STATE->mousePos, optRect) && UI_STATE->mouseReleased) {
            TEST.selectedAnswer = i; TEST.testTotal++;
            if (i == TEST.currentCorrectIdx) { TEST.testCorrect++; TEST.answerResult = 1; } else TEST.answerResult = 0;
        }
    }
    Rectangle progressRect = {SCREEN_WIDTH/2 - 150, 660, 300, 40};
    int testRate = TEST.testTotal > 0 ? (int)((float)TEST.testCorrect / TEST.testTotal * 100.0f) : 0;
    char progressText[64];
    snprintf(progressText, sizeof(progressText), u8"进度: %d / %d  正确率: %d%%", TEST.currentTestIdx + 1, TEST.testCount, testRate);
    Vector2 pSize = MeasureTextAuto(progressText, 22, 1);
    DrawTextAuto(progressText, (Vector2){SCREEN_WIDTH/2 - pSize.x/2, progressRect.y + 8}, 22, 1, STYLE->theme.textSecondary);
    if (TEST.answerResult != -1) {
        Rectangle nextBtn = {SCREEN_WIDTH/2 - 75, 710, 150, 55};
        if (UIButton(u8"下一题", nextBtn, STYLE, UI_STATE, 4)) { TEST.currentTestIdx++; TEST.selectedAnswer = -1; TEST.answerResult = -1; }
    }
}
