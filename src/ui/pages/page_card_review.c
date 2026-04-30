// ============================================================================
// 卡片背单词页面 — 闪卡翻转记忆
// 功能：显示闪卡（正面单词 → 点击翻转 → 背面释义），
//       点击"认识"/"不认识"记录进度，认识 3 次标记已掌握
// ============================================================================

#include "pages.h"

/**
 * MenuCardReview_Show - 渲染卡片背单词页面
 *
 * 页面结构（从上到下）：
 *   1. 进度条 — 当前卡片索引 / 总数
 *   2. 闪卡组件 — 点击翻转，背面显示"认识"/"不认识"按钮
 *   3. 本轮统计 — 本轮已认识的 / 不认识的数量
 *
 * 学习逻辑：
 *   - 点击"认识" → knownCount++，连续 3 次 → mastered=true
 *   - 点击"不认识" → unknownCount++，重置 mastered=false
 *   - 每次操作后自动保存进度（saveProgress）
 */
void MenuCardReview_Show(void) {
    // 所有单词都已掌握 — 显示恭喜信息
    if (REVIEW.reviewCount == 0) {
        Rectangle r = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 60, 500, 120};
        DrawRectangleRounded(r, 0.1f, 12, STYLE->theme.panelBg);
        Vector2 ms = MeasureTextAuto(u8"恭喜！所有单词都已掌握！", 34, 1);
        DrawTextAuto(u8"恭喜！所有单词都已掌握！",
            (Vector2){SCREEN_WIDTH/2 - ms.x/2, SCREEN_HEIGHT/2 - 20}, 34, 1, STYLE->theme.success);
        return;
    }

    // ---- 进度条 ----
    Rectangle pb = {SCREEN_WIDTH/2 - 250, 100, 500, 30};
    DrawRectangleRounded(pb, 0.2f, 5, STYLE->theme.inputBg);
    float prog = (float)REVIEW.currentReviewIdx / REVIEW.reviewCount;
    DrawRectangleRounded((Rectangle){pb.x, pb.y, pb.width * prog, pb.height},
                         0.2f, 5, STYLE->theme.primary);
    char progText[64];
    snprintf(progText, sizeof(progText), u8"%d / %d", REVIEW.currentReviewIdx + 1, REVIEW.reviewCount);
    Vector2 ps = MeasureTextAuto(progText, 22, 1);
    DrawTextAuto(progText, (Vector2){SCREEN_WIDTH/2 - ps.x/2, pb.y + 35}, 22, 1, STYLE->theme.textSecondary);

    // ---- 闪卡 ----
    Rectangle cr = {SCREEN_WIDTH/2 - 220, 160, 440, 264};
    int idx = REVIEW.reviewIndices[REVIEW.currentReviewIdx];
    int action = UIFlashCard(&g_words[idx].entry, cr,
                             &REVIEW.flashcardFace, STYLE, UI_STATE, &REVIEW.flashcardAnimTime);

    // 处理"认识"按钮（action==1）
    if (action == 1) {
        g_words[idx].progress.knownCount++;
        g_words[idx].progress.lastReview = time(NULL);
        if (g_words[idx].progress.knownCount >= 3)
            g_words[idx].progress.mastered = true;
        REVIEW.knownInSession++;
        REVIEW.flashcardFace = CARD_FRONT;
        REVIEW.flashcardAnimTime = 0.0f;
        if (++REVIEW.currentReviewIdx >= REVIEW.reviewCount)
            REVIEW.currentReviewIdx = REVIEW.reviewCount - 1;
        saveProgress();
    }
    // 处理"不认识"按钮（action==2）
    else if (action == 2) {
        g_words[idx].progress.unknownCount++;
        g_words[idx].progress.lastReview = time(NULL);
        g_words[idx].progress.mastered = false;
        REVIEW.unknownInSession++;
        REVIEW.flashcardFace = CARD_FRONT;
        REVIEW.flashcardAnimTime = 0.0f;
        if (++REVIEW.currentReviewIdx >= REVIEW.reviewCount)
            REVIEW.currentReviewIdx = REVIEW.reviewCount - 1;
        saveProgress();
    }

    // ---- 本轮统计 ----
    Rectangle sr = {SCREEN_WIDTH/2 - 180, 470, 360, 65};
    DrawRectangleRounded(sr, 0.1f, 8, STYLE->theme.panelBg);
    char st[128];
    snprintf(st, sizeof(st), u8"本轮认识: %d  |  不认识: %d",
             REVIEW.knownInSession, REVIEW.unknownInSession);
    Vector2 ss = MeasureTextAuto(st, 26, 1);
    DrawTextAuto(st, (Vector2){SCREEN_WIDTH/2 - ss.x/2, sr.y + 18}, 26, 1, STYLE->theme.textPrimary);
}
