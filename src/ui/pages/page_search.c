// ============================================================================
// 查找单词页面 — 支持通配符搜索
// ============================================================================

#include "pages.h"

void MenuSearch_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 25, 30);

    Rectangle searchPanel = UILayoutNext(&layout, -1, 130);
    DrawRectangleRounded(searchPanel, 0.1f, 12, STYLE->theme.panelBg);

    Rectangle titleRect = {searchPanel.x + 25, searchPanel.y + 15, 200, 40};
    DrawTextAuto(u8"查找单词", (Vector2){titleRect.x, titleRect.y}, 28, 1, STYLE->theme.textPrimary);

    Rectangle hintRect = {searchPanel.x + 25, searchPanel.y + 55, 500, 30};
    DrawTextAuto(u8"支持正则表达式，例如: ab.*  或  ^acc.*", (Vector2){hintRect.x, hintRect.y}, 18, 1, STYLE->theme.textSecondary);

    Rectangle inputRect = {searchPanel.x + 25, searchPanel.y + 85, searchPanel.width - 150, 45};
    UISearchBar(&SEARCH.searchBar, inputRect, STYLE, UI_STATE);

    Rectangle btnRect = {searchPanel.x + searchPanel.width - 115, searchPanel.y + 85, 90, 45};
    static int lastSearchLen = 0;
    if (UIButton(u8"搜索", btnRect, STYLE, UI_STATE, 7)) {
        const char* query = SEARCH.searchBar.textState.buffer;
        if (strlen(query) > 0) {
            SEARCH.searchResultCount = searchWordsByRegex(query, SEARCH.searchResults, MAX_WORDS);
            if (SEARCH.searchResultCount == 0) SEARCH.searchResultCount = searchWordsSimple(query, SEARCH.searchResults, MAX_WORDS);
        }
    }
    int currentLen = strlen(SEARCH.searchBar.textState.buffer);
    if (currentLen > 0 && currentLen != lastSearchLen) {
        const char* query = SEARCH.searchBar.textState.buffer;
        SEARCH.searchResultCount = searchWordsByRegex(query, SEARCH.searchResults, MAX_WORDS);
        if (SEARCH.searchResultCount == 0) SEARCH.searchResultCount = searchWordsSimple(query, SEARCH.searchResults, MAX_WORDS);
    }
    lastSearchLen = currentLen;

    Rectangle resultPanel = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(resultPanel, 0.1f, 12, STYLE->theme.panelBg);

    if (SEARCH.searchResultCount == 0 && strlen(SEARCH.searchBar.textState.buffer) > 0) {
        Rectangle msgRect = {resultPanel.x + 30, resultPanel.y + 30, resultPanel.width - 60, 60};
        DrawRectangleRounded(msgRect, 0.1f, 8, STYLE->theme.inputBg);
        const char* msg = u8"没有找到匹配的单词";
        Vector2 msgSize = MeasureTextAuto(msg, 26, 1);
        DrawTextAuto(msg, (Vector2){resultPanel.x + resultPanel.width/2 - msgSize.x/2, resultPanel.y + resultPanel.height/2 - msgSize.y/2}, 26, 1, STYLE->theme.textSecondary);
    } else if (SEARCH.searchResultCount > 0) {
        Rectangle countRect = {resultPanel.x + 25, resultPanel.y + 15, 300, 35};
        char countText[64]; snprintf(countText, sizeof(countText), u8"找到 %d 个结果", SEARCH.searchResultCount);
        DrawTextAuto(countText, (Vector2){countRect.x, countRect.y}, 22, 1, STYLE->theme.textSecondary);
        Rectangle listRect = {resultPanel.x, resultPanel.y + 55, resultPanel.width, resultPanel.height - 70};
        UIScrollView sv = {0}; sv.viewport = listRect;
        sv.contentSize = (Vector2){listRect.width - 30, SEARCH.searchResultCount * 235.0f};
        UIBeginScrollView(&sv, listRect, sv.contentSize);
        for (int i = 0; i < SEARCH.searchResultCount; i++) {
            int wordIdx = SEARCH.searchResults[i];
            WordEntry* entry = &g_words[wordIdx].entry;
            Rectangle itemRect = {listRect.x + 20, listRect.y + i * 230 - sv.scrollOffset.y, listRect.width - 40, 215};
            DrawRectangleRounded(itemRect, 0.1f, 8, STYLE->theme.inputBg);
            DrawTextAuto(entry->word, (Vector2){itemRect.x + 20, itemRect.y + 15}, 36, 1, STYLE->theme.primary);
            if (entry->phonetic && *entry->phonetic) DrawTextAuto(entry->phonetic, (Vector2){itemRect.x + 20, itemRect.y + 55}, 24, 1, STYLE->theme.textSecondary);
            DrawTextAuto(u8"释义", (Vector2){itemRect.x + 20, itemRect.y + 85}, 18, 1, STYLE->theme.textSecondary);
            Rectangle dRect = {itemRect.x + 20, itemRect.y + 105, itemRect.width - 40, 35};
            UIDrawTextRec(entry->definition, dRect, 22, 1, true, STYLE->theme.textPrimary);
            if (entry->example && *entry->example) {
                DrawTextAuto(u8"例句", (Vector2){itemRect.x + 20, itemRect.y + 140}, 18, 1, STYLE->theme.textSecondary);
                Rectangle eRect = {itemRect.x + 75, itemRect.y + 140, itemRect.width - 95, 35};
                UIDrawTextRec(entry->example, eRect, 20, 1, true, STYLE->theme.textSecondary);
                if (entry->exampleTranslation && *entry->exampleTranslation) {
                    DrawTextAuto(u8"翻译", (Vector2){itemRect.x + 20, itemRect.y + 175}, 18, 1, (Color){100, 149, 237, 255});
                    Rectangle tRect = {itemRect.x + 75, itemRect.y + 175, itemRect.width - 95, 35};
                    UIDrawTextRec(entry->exampleTranslation, tRect, 20, 1, true, STYLE->theme.textPrimary);
                }
            }
        }
        UIEndScrollView(&sv, STYLE, UI_STATE);
    } else {
        Rectangle msgRect = {resultPanel.x + 30, resultPanel.y + 30, resultPanel.width - 60, 60};
        DrawRectangleRounded(msgRect, 0.1f, 8, STYLE->theme.inputBg);
        const char* msg = u8"请在上方输入要查找的单词";
        Vector2 msgSize = MeasureTextAuto(msg, 24, 1);
        DrawTextAuto(msg, (Vector2){resultPanel.x + resultPanel.width/2 - msgSize.x/2, resultPanel.y + resultPanel.height/2 - msgSize.y/2}, 24, 1, STYLE->theme.textSecondary);
    }
}
