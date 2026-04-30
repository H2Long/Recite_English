// ============================================================================
// 学单词页面
// 左侧：可滚动的单词列表；右侧：选中单词的详细信息
// ============================================================================

#include "pages.h"

void MenuLearn_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_HORIZONTAL, 25, 0);

    // 左侧：单词列表
    Rectangle listArea = UILayoutNext(&layout, 350, -1);
    DrawRectangleRounded(listArea, 0.1f, 8, STYLE->theme.panelBg);

    Rectangle filterRect = {listArea.x + 15, listArea.y + 15, 220, 35};
    UICheckbox(u8"只显示未掌握", filterRect, &LEARN.learnFilterUnknown, STYLE, UI_STATE);

    Rectangle scrollView = {listArea.x, listArea.y + 60, listArea.width, listArea.height - 60};
    UIScrollView sv = {0};
    sv.scrollOffset.y = LEARN.learnScrollOffset;

    int masteredCount = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if (g_words[i].progress.mastered) masteredCount++;
    }
    int visibleCount = LEARN.learnFilterUnknown ?
        (g_wordProgressCount - masteredCount) : g_wordProgressCount;

    sv.contentSize = (Vector2){scrollView.width, visibleCount * 60.0f};
    UIBeginScrollView(&sv, scrollView, sv.contentSize);

    int listIndex = 0;
    for (int i = 0; i < g_wordProgressCount; i++) {
        if (LEARN.learnFilterUnknown && g_words[i].progress.mastered) continue;

        Rectangle itemRect = {scrollView.x, scrollView.y + listIndex * 60 - sv.scrollOffset.y, scrollView.width, 60};
        Color indicatorColor = getMasteryColor(
            (float)g_words[i].progress.knownCount /
            (g_words[i].progress.knownCount + g_words[i].progress.unknownCount + 1));
        DrawRectangleRec((Rectangle){itemRect.x + 5, itemRect.y + 12, 5, itemRect.height - 24}, indicatorColor);

        if (i == LEARN.learnIndex) DrawRectangleRec(itemRect, Fade(STYLE->theme.primary, 0.2f));

        if (UIListItem(g_words[i].entry.word, itemRect, STYLE, UI_STATE)) LEARN.learnIndex = i;
        listIndex++;
    }

    UIEndScrollView(&sv, STYLE, UI_STATE);
    LEARN.learnScrollOffset = sv.scrollOffset.y;

    // 右侧：单词详情
    Rectangle detailArea = UILayoutNext(&layout, -1, -1);
    WordEntry* entry = &g_words[LEARN.learnIndex].entry;
    WordProgress* progress = &g_words[LEARN.learnIndex].progress;

    DrawRectangleRounded(detailArea, 0.1f, 12, STYLE->theme.panelBg);
    UILayout detailLayout = UIBeginLayout(detailArea, UI_DIR_VERTICAL, 20, 30);

    Rectangle wordRect = UILayoutNext(&detailLayout, -1, 70);
    DrawTextAuto(entry->word, (Vector2){wordRect.x, wordRect.y}, 56, 1, STYLE->theme.primary);

    if (entry->phonetic && *entry->phonetic) {
        Rectangle phoRect = UILayoutNext(&detailLayout, -1, 45);
        DrawTextAuto(entry->phonetic, (Vector2){wordRect.x, phoRect.y}, 32, 1, STYLE->theme.textSecondary);
    }

    Rectangle lineRect = UILayoutNext(&detailLayout, -1, 3);
    DrawLineEx((Vector2){lineRect.x, lineRect.y}, (Vector2){lineRect.x + lineRect.width, lineRect.y}, 1, STYLE->theme.inputBorder);

    Rectangle defLabelRect = UILayoutNext(&detailLayout, -1, 40);
    DrawTextAuto(u8"释义", (Vector2){defLabelRect.x, defLabelRect.y}, 28, 1, STYLE->theme.textSecondary);

    Rectangle defRect = UILayoutNext(&detailLayout, -1, 80);
    UIDrawTextRec(entry->definition, defRect, 30, 1, true, STYLE->theme.textPrimary);

    Rectangle exLabelRect = UILayoutNext(&detailLayout, -1, 40);
    DrawTextAuto(u8"例句", (Vector2){exLabelRect.x, exLabelRect.y}, 28, 1, STYLE->theme.textSecondary);

    if (entry->example && *entry->example) {
        Rectangle exRect = UILayoutNext(&detailLayout, -1, 90);
        char exampleFull[512];
        snprintf(exampleFull, sizeof(exampleFull), u8"例: %s", entry->example);
        UIDrawTextRec(exampleFull, exRect, 26, 1, true, STYLE->theme.textSecondary);
    }

    Rectangle stateRect = UILayoutNext(&detailLayout, -1, 90);
    DrawRectangleRounded(stateRect, 0.1f, 8, STYLE->theme.inputBg);
    UILayout stateLayout = UIBeginLayout(stateRect, UI_DIR_HORIZONTAL, 40, 20);
    Rectangle stateItem = UILayoutNext(&stateLayout, -1, -1);

    char stateText[256];
    snprintf(stateText, sizeof(stateText),
        u8"认识: %d次  |  不认识: %d次  |  状态: %s  |  上次复习: %s",
        progress->knownCount, progress->unknownCount,
        progress->mastered ? u8"已掌握" : u8"学习中",
        formatTime(progress->lastReview));
    UIDrawTextRec(stateText, (Rectangle){stateItem.x, stateItem.y + 20, stateItem.width, stateItem.height}, 22, 1, true, STYLE->theme.textPrimary);

    Rectangle navRect = UILayoutNext(&detailLayout, -1, 60);
    UILayout navLayout = UIBeginLayout(navRect, UI_DIR_HORIZONTAL, 25, 0);
    Rectangle prevBtn = UILayoutNext(&navLayout, 120, 50);
    Rectangle nextBtn = UILayoutNext(&navLayout, 120, 50);

    if (UIButton(u8"上一个", prevBtn, STYLE, UI_STATE, 1) && LEARN.learnIndex > 0) LEARN.learnIndex--;
    if (UIButton(u8"下一个", nextBtn, STYLE, UI_STATE, 2) && LEARN.learnIndex < g_wordProgressCount - 1) LEARN.learnIndex++;
}
