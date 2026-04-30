// ============================================================================
// 卡片背单词页面 — 闪卡翻转记忆
// ============================================================================

#include "pages.h"

void MenuCardReview_Show(void) {
    if (REVIEW.reviewCount == 0) {
        Rectangle msgRect = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 60, 500, 120};
        DrawRectangleRounded(msgRect, 0.1f, 12, STYLE->theme.panelBg);
        const char* msg = u8"恭喜！所有单词都已掌握！";
        Vector2 mSize = MeasureTextAuto(msg, 34, 1);
        DrawTextAuto(msg, (Vector2){SCREEN_WIDTH/2 - mSize.x/2, SCREEN_HEIGHT/2 - 20}, 34, 1, STYLE->theme.success);
        return;
    }

    // 进度条
    Rectangle progressBar = {SCREEN_WIDTH/2 - 250, 100, 500, 30};
    DrawRectangleRounded(progressBar, 0.2f, 5, STYLE->theme.inputBg);
    float progress = (float)REVIEW.currentReviewIdx / REVIEW.reviewCount;
    DrawRectangleRounded((Rectangle){progressBar.x, progressBar.y, progressBar.width * progress, progressBar.height}, 0.2f, 5, STYLE->theme.primary);
    char progressText[64];
    snprintf(progressText, sizeof(progressText), u8"%d / %d", REVIEW.currentReviewIdx + 1, REVIEW.reviewCount);
    Vector2 pSize = MeasureTextAuto(progressText, 22, 1);
    DrawTextAuto(progressText, (Vector2){SCREEN_WIDTH/2 - pSize.x/2, progressBar.y + 35}, 22, 1, STYLE->theme.textSecondary);

    // 闪卡
    Rectangle cardRect = {SCREEN_WIDTH/2 - 220, 160, 440, 264};
    int action = UIFlashCard(&g_words[REVIEW.reviewIndices[REVIEW.currentReviewIdx]].entry, cardRect,
                            &REVIEW.flashcardFace, STYLE, UI_STATE, &REVIEW.flashcardAnimTime);

    if (action == 1) {
        int idx = REVIEW.reviewIndices[REVIEW.currentReviewIdx];
        g_words[idx].progress.knownCount++;
        g_words[idx].progress.lastReview = time(NULL);
        if (g_words[idx].progress.knownCount >= 3) g_words[idx].progress.mastered = true;
        REVIEW.knownInSession++;
        REVIEW.flashcardFace = CARD_FRONT;
        REVIEW.flashcardAnimTime = 0.0f;
        REVIEW.currentReviewIdx++;
        if (REVIEW.currentReviewIdx >= REVIEW.reviewCount) REVIEW.currentReviewIdx = REVIEW.reviewCount - 1;
        saveProgress();
    } else if (action == 2) {
        int idx = REVIEW.reviewIndices[REVIEW.currentReviewIdx];
        g_words[idx].progress.unknownCount++;
        g_words[idx].progress.lastReview = time(NULL);
        g_words[idx].progress.mastered = false;
        REVIEW.unknownInSession++;
        REVIEW.flashcardFace = CARD_FRONT;
        REVIEW.flashcardAnimTime = 0.0f;
        REVIEW.currentReviewIdx++;
        if (REVIEW.currentReviewIdx >= REVIEW.reviewCount) REVIEW.currentReviewIdx = REVIEW.reviewCount - 1;
        saveProgress();
    }

    // 本轮统计
    Rectangle statsRect = {SCREEN_WIDTH/2 - 180, 470, 360, 65};
    DrawRectangleRounded(statsRect, 0.1f, 8, STYLE->theme.panelBg);
    char sessionText[128];
    snprintf(sessionText, sizeof(sessionText), u8"本轮认识: %d  |  不认识: %d", REVIEW.knownInSession, REVIEW.unknownInSession);
    Vector2 sSize = MeasureTextAuto(sessionText, 26, 1);
    DrawTextAuto(sessionText, (Vector2){SCREEN_WIDTH/2 - sSize.x/2, statsRect.y + 18}, 26, 1, STYLE->theme.textPrimary);
}
