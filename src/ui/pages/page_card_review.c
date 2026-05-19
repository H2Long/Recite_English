// 卡片背单词 — 闪卡翻转记忆

#include "pages.h"

void MenuCardReview_Show(void) {
    if(REVIEW.reviewCount == 0) {
        Rectangle r = {SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 - 60, 500, 120};
        DrawRectangleRounded(r, 0.1f, 12, STYLE->theme.panelBg);
        Vector2 ms = MeasureTextAuto(u8"恭喜！所有单词都已掌握！", 34, 1);
        DrawTextAuto(u8"恭喜！所有单词都已掌握！",
            (Vector2){SCREEN_WIDTH/2 - ms.x/2, SCREEN_HEIGHT/2 - 20}, 34, 1, STYLE->theme.success);
        return ;
    }

    // 本轮已完成
    if(REVIEW.knownInSession + REVIEW.unknownInSession >= REVIEW.reviewCount) {
        Rectangle r = {SCREEN_WIDTH/2 - 280, 140, 560, 200};
        UIDrawCard(r, 0.08f, STYLE);
        DrawRectangleRoundedLines(r, 0.08f, 12, STYLE->theme.success);
        Vector2 m1 = MeasureTextAuto(u8"本轮复习完成！", 36, 1);
        DrawTextAuto(u8"本轮复习完成！", (Vector2){SCREEN_WIDTH/2 - m1.x/2, r.y + 25}, 36, 1, STYLE->theme.success);
        char msg[128];
        snprintf(msg, sizeof(msg), u8"认识: %d  |  不认识: %d  |  共 %d 词",
            REVIEW.knownInSession, REVIEW.unknownInSession, REVIEW.reviewCount);
        Vector2 m2 = MeasureTextAuto(msg, 22, 1);
        DrawTextAuto(msg, (Vector2){SCREEN_WIDTH/2 - m2.x/2, r.y + 80}, 22, 1, STYLE->theme.textPrimary);
        int remaining = Plan_GetRemainingToday();
        char rt[64];
        if(remaining > 0) {
            snprintf(rt, sizeof(rt), u8"今日剩余: %d 词", remaining);
        } else {
            snprintf(rt, sizeof(rt), u8"今日目标已完成！");
        }
        Vector2 m3 = MeasureTextAuto(rt, 20, 1);
        DrawTextAuto(rt, (Vector2){SCREEN_WIDTH/2 - m3.x/2, r.y + 115}, 20, 1,
            remaining > 0 ? STYLE->theme.textSecondary : STYLE->theme.success);
        Rectangle btn = {SCREEN_WIDTH/2 - 80, r.y + 150, 160, 40};
        if(UIButton(u8"重新开始", btn, STYLE, UI_STATE, 900)) {
            RefreshReviewList();
        }
        return ;
    }

    // 进度条
    Rectangle pb = {SCREEN_WIDTH/2 - 250, 100, 500, 30};
    DrawRectangleRounded(pb, 0.2f, 5, STYLE->theme.inputBg);
    float prog = REVIEW.reviewCount > 0 ? (float)(REVIEW.currentReviewIdx + 1) / REVIEW.reviewCount : 0;
    DrawRectangleRounded((Rectangle){pb.x, pb.y, pb.width*prog, pb.height}, 0.2f, 5, STYLE->theme.primary);
    char pt[64];
    snprintf(pt, sizeof(pt), u8"%d / %d", REVIEW.currentReviewIdx + 1, REVIEW.reviewCount);
    Vector2 ps = MeasureTextAuto(pt, 22, 1);
    DrawTextAuto(pt, (Vector2){SCREEN_WIDTH/2 - ps.x/2, pb.y + 35}, 22, 1, STYLE->theme.textSecondary);

    // 闪卡
    Rectangle cr = {SCREEN_WIDTH/2 - 220, 140, 440, 260};
    int idx = REVIEW.reviewIndices[REVIEW.currentReviewIdx];
    int action = UIFlashCard(&g_words[idx].entry, cr,
        &REVIEW.flashcardFace, STYLE, UI_STATE, &REVIEW.flashcardAnimTime);

    // 认识
    if(action == 1) {
        g_words[idx].progress.knownCount++;
        g_words[idx].progress.lastReview = time(NULL);
        if(g_words[idx].progress.knownCount >= MASTERED_THRESHOLD) {
            g_words[idx].progress.mastered = true;
        }
        REVIEW.knownInSession++;
        REVIEW.flashcardFace = CARD_FRONT;
        REVIEW.flashcardAnimTime = 0.0f;
        REVIEW.currentReviewIdx++;
        if(REVIEW.currentReviewIdx >= REVIEW.reviewCount) {
            REVIEW.currentReviewIdx = REVIEW.reviewCount - 1;
        }
        Plan_AddStudiedToday(1);
        save_progress();
    }
    // 不认识
    else if(action == 2) {
        g_words[idx].progress.unknownCount++;
        g_words[idx].progress.lastReview = time(NULL);
        g_words[idx].progress.mastered = false;
        REVIEW.unknownInSession++;
        REVIEW.flashcardFace = CARD_FRONT;
        REVIEW.flashcardAnimTime = 0.0f;
        REVIEW.currentReviewIdx++;
        if(REVIEW.currentReviewIdx >= REVIEW.reviewCount) {
            REVIEW.currentReviewIdx = REVIEW.reviewCount - 1;
        }
        Plan_AddStudiedToday(1);
        save_progress();
    }

    // 本轮统计
    Rectangle sr = {SCREEN_WIDTH/2 - 220, 470, 440, 85};
    UIDrawCard(sr, 0.08f, STYLE);
    char st[128];
    snprintf(st, sizeof(st), u8"本轮认识: %d  |  不认识: %d",
        REVIEW.knownInSession, REVIEW.unknownInSession);
    Vector2 ss = MeasureTextAuto(st, 26, 1);
    DrawTextAuto(st, (Vector2){SCREEN_WIDTH/2 - ss.x/2, sr.y + 12}, 26, 1, STYLE->theme.textPrimary);

    int remaining = Plan_GetRemainingToday();
    if(remaining > 0) {
        char rt[64];
        snprintf(rt, sizeof(rt), u8"今日剩余: %d 词", remaining);
        Vector2 rs = MeasureTextAuto(rt, 20, 1);
        DrawTextAuto(rt, (Vector2){SCREEN_WIDTH/2 - rs.x/2, sr.y + 48}, 20, 1, STYLE->theme.textSecondary);
    } else {
        Vector2 rs = MeasureTextAuto(u8"今日目标已完成！", 20, 1);
        DrawTextAuto(u8"今日目标已完成！", (Vector2){SCREEN_WIDTH/2 - rs.x/2, sr.y + 48}, 20, 1, STYLE->theme.success);
    }
}
