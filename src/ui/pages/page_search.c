// 查找单词 — 通配符搜索

#include "pages.h"

void MenuSearch_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_VERTICAL, 25, 30);

    // 搜索区域
    Rectangle sp = UILayoutNext(&lay, -1, 130);
    DrawRectangleRounded(sp, 0.1f, 12, STYLE->theme.panelBg);
    DrawTextAuto(u8"查找单词", (Vector2){sp.x + 25, sp.y + 15}, 28, 1, STYLE->theme.textPrimary);
    DrawTextAuto(u8"支持正则表达式，例如: ab.*  或  ^acc.*",
        (Vector2){sp.x + 25, sp.y + 55}, 18, 1, STYLE->theme.textSecondary);

    Rectangle ir = {sp.x + 25, sp.y + 85, sp.width - 150, 45};
    UISearchBar(&SEARCH.searchBar, ir, STYLE, UI_STATE);

    static int lastLen = 0;
    Rectangle br = {sp.x + sp.width - 115, sp.y + 85, 90, 45};
    if(UIButton(u8"搜索", br, STYLE, UI_STATE, 7)) {
        const char* q = SEARCH.searchBar.textState.buffer;
        if(strlen(q) > 0) {
            SEARCH.searchResultCount = searchWordsByRegex(q, SEARCH.searchResults, MAX_WORDS);
            if(SEARCH.searchResultCount == 0) {
                SEARCH.searchResultCount = searchWordsSimple(q, SEARCH.searchResults, MAX_WORDS);
            }
        }
    }
    int curLen = strlen(SEARCH.searchBar.textState.buffer);
    if(curLen > 0 && curLen != lastLen) {
        const char* q = SEARCH.searchBar.textState.buffer;
        SEARCH.searchResultCount = searchWordsByRegex(q, SEARCH.searchResults, MAX_WORDS);
        if(SEARCH.searchResultCount == 0) {
            SEARCH.searchResultCount = searchWordsSimple(q, SEARCH.searchResults, MAX_WORDS);
        }
    }
    lastLen = curLen;

    // 搜索结果
    Rectangle rp = UILayoutNext(&lay, -1, -1);
    DrawRectangleRounded(rp, 0.1f, 12, STYLE->theme.panelBg);

    if(SEARCH.searchResultCount == 0 && strlen(SEARCH.searchBar.textState.buffer) > 0) {
        Vector2 ms = MeasureTextAuto(u8"没有找到匹配的单词", 26, 1);
        DrawTextAuto(u8"没有找到匹配的单词",
            (Vector2){rp.x + rp.width/2 - ms.x/2, rp.y + rp.height/2 - ms.y/2},
            26, 1, STYLE->theme.textSecondary);
    }
    else if(SEARCH.searchResultCount > 0) {
        char ct[64];
        snprintf(ct, sizeof(ct), u8"找到 %d 个结果", SEARCH.searchResultCount);
        DrawTextAuto(ct, (Vector2){rp.x + 25, rp.y + 15}, 22, 1, STYLE->theme.textSecondary);

        Rectangle lr = {rp.x, rp.y + 55, rp.width, rp.height - 70};
        UIScrollView sv = {0};
        sv.viewport = lr;
        sv.contentSize = (Vector2){lr.width - 30, SEARCH.searchResultCount * 235.0f};
        UIBeginScrollView(&sv, lr, sv.contentSize);

        for (int i = 0; i < SEARCH.searchResultCount; i++) {
            WordEntry* e = &g_words[SEARCH.searchResults[i]].entry;
            Rectangle ir2 = {lr.x + 20, lr.y + i*230 - sv.scrollOffset.y, lr.width - 40, 215};
            DrawRectangleRounded(ir2, 0.1f, 8, STYLE->theme.inputBg);
            DrawTextAuto(e->word, (Vector2){ir2.x + 20, ir2.y + 15}, 36, 1, STYLE->theme.primary);
            if(e->phonetic && *e->phonetic) {
                DrawTextAuto(e->phonetic, (Vector2){ir2.x + 20, ir2.y + 55}, 24, 1, STYLE->theme.textSecondary);
            }
            DrawTextAuto(u8"释义", (Vector2){ir2.x + 20, ir2.y + 85}, 18, 1, STYLE->theme.textSecondary);
            UIDrawTextRec(e->definition, (Rectangle){ir2.x+20, ir2.y+105, ir2.width-40, 35},
                22, 1, true, STYLE->theme.textPrimary);
            if(e->example && *e->example) {
                DrawTextAuto(u8"例句", (Vector2){ir2.x + 20, ir2.y + 140}, 18, 1, STYLE->theme.textSecondary);
                UIDrawTextRec(e->example, (Rectangle){ir2.x+75, ir2.y+140, ir2.width-95, 35},
                    20, 1, true, STYLE->theme.textSecondary);
                if(e->exampleTranslation && *e->exampleTranslation) {
                    DrawTextAuto(u8"翻译", (Vector2){ir2.x+20, ir2.y+175}, 18, 1, (Color){100, 149, 237, 255});
                    UIDrawTextRec(e->exampleTranslation, (Rectangle){ir2.x+75, ir2.y+175, ir2.width-95, 35},
                        20, 1, true, STYLE->theme.textPrimary);
                }
            }
        }
        UIEndScrollView(&sv, STYLE, UI_STATE);
    }
    else {
        Vector2 ms = MeasureTextAuto(u8"请在上方输入要查找的单词", 24, 1);
        DrawTextAuto(u8"请在上方输入要查找的单词",
            (Vector2){rp.x + rp.width/2 - ms.x/2, rp.y + rp.height/2 - ms.y/2},
            24, 1, STYLE->theme.textSecondary);
    }
}
