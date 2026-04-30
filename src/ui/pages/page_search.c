// ============================================================================
// 查找单词页面 — 通配符搜索 + 结果展示
// 功能：输入关键词，支持 ^/$/*/? 通配符搜索，滚动显示每个匹配单词的详情
// ============================================================================

#include "pages.h"

/**
 * MenuSearch_Show - 渲染查找单词页面
 *
 * 布局：
 *   1. 搜索栏（搜索输入框 + 搜索按钮）+ 语法提示
 *   2. 搜索结果区域（可滚动）
 *      每个结果项显示：单词（大号）、音标、释义标签+内容、例句标签+内容+翻译
 *
 * 搜索触发方式：
 *   - 点击"搜索"按钮
 *   - 输入框内容变化时自动搜索
 *
 * 支持的通配符语法：
 *   ^abc  以 abc 开头     abc$  以 abc 结尾
 *   a*c   任意字符匹配     ab?d  单字符匹配
 */
void MenuSearch_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 25, 30);

    // ---- 搜索栏区域 ----
    Rectangle sp = UILayoutNext(&layout, -1, 130);
    DrawRectangleRounded(sp, 0.1f, 12, STYLE->theme.panelBg);
    DrawTextAuto(u8"查找单词", (Vector2){sp.x + 25, sp.y + 15}, 28, 1, STYLE->theme.textPrimary);
    DrawTextAuto(u8"支持正则表达式，例如: ab.*  或  ^acc.*",
        (Vector2){sp.x + 25, sp.y + 55}, 18, 1, STYLE->theme.textSecondary);

    Rectangle inputRect = {sp.x + 25, sp.y + 85, sp.width - 150, 45};
    UISearchBar(&SEARCH.searchBar, inputRect, STYLE, UI_STATE);

    static int lastLen = 0;
    Rectangle btnR = {sp.x + sp.width - 115, sp.y + 85, 90, 45};

    // 按钮搜索
    if (UIButton(u8"搜索", btnR, STYLE, UI_STATE, 7)) {
        const char* q = SEARCH.searchBar.textState.buffer;
        if (strlen(q) > 0) {
            SEARCH.searchResultCount = searchWordsByRegex(q, SEARCH.searchResults, MAX_WORDS);
            if (SEARCH.searchResultCount == 0)
                SEARCH.searchResultCount = searchWordsSimple(q, SEARCH.searchResults, MAX_WORDS);
        }
    }
    // 自动搜索
    int curLen = strlen(SEARCH.searchBar.textState.buffer);
    if (curLen > 0 && curLen != lastLen) {
        const char* q = SEARCH.searchBar.textState.buffer;
        SEARCH.searchResultCount = searchWordsByRegex(q, SEARCH.searchResults, MAX_WORDS);
        if (SEARCH.searchResultCount == 0)
            SEARCH.searchResultCount = searchWordsSimple(q, SEARCH.searchResults, MAX_WORDS);
    }
    lastLen = curLen;

    // ---- 搜索结果区域 ----
    Rectangle rp = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(rp, 0.1f, 12, STYLE->theme.panelBg);

    if (SEARCH.searchResultCount == 0 && strlen(SEARCH.searchBar.textState.buffer) > 0) {
        Rectangle mr = {rp.x + 30, rp.y + 30, rp.width - 60, 60};
        DrawRectangleRounded(mr, 0.1f, 8, STYLE->theme.inputBg);
        Vector2 ms = MeasureTextAuto(u8"没有找到匹配的单词", 26, 1);
        DrawTextAuto(u8"没有找到匹配的单词",
            (Vector2){rp.x + rp.width/2 - ms.x/2, rp.y + rp.height/2 - ms.y/2}, 26, 1, STYLE->theme.textSecondary);
    } else if (SEARCH.searchResultCount > 0) {
        char ct[64]; snprintf(ct, sizeof(ct), u8"找到 %d 个结果", SEARCH.searchResultCount);
        DrawTextAuto(ct, (Vector2){rp.x + 25, rp.y + 15}, 22, 1, STYLE->theme.textSecondary);

        Rectangle lr = {rp.x, rp.y + 55, rp.width, rp.height - 70};
        UIScrollView sv = {0}; sv.viewport = lr;
        sv.contentSize = (Vector2){lr.width - 30, SEARCH.searchResultCount * 235.0f};
        UIBeginScrollView(&sv, lr, sv.contentSize);

        for (int i = 0; i < SEARCH.searchResultCount; i++) {
            WordEntry* e = &g_words[SEARCH.searchResults[i]].entry;
            Rectangle ir = {lr.x + 20, lr.y + i * 230 - sv.scrollOffset.y, lr.width - 40, 215};
            DrawRectangleRounded(ir, 0.1f, 8, STYLE->theme.inputBg);
            DrawTextAuto(e->word, (Vector2){ir.x + 20, ir.y + 15}, 36, 1, STYLE->theme.primary);
            if (e->phonetic && *e->phonetic)
                DrawTextAuto(e->phonetic, (Vector2){ir.x + 20, ir.y + 55}, 24, 1, STYLE->theme.textSecondary);
            DrawTextAuto(u8"释义", (Vector2){ir.x + 20, ir.y + 85}, 18, 1, STYLE->theme.textSecondary);
            UIDrawTextRec(e->definition, (Rectangle){ir.x + 20, ir.y + 105, ir.width - 40, 35}, 22, 1, true, STYLE->theme.textPrimary);
            if (e->example && *e->example) {
                DrawTextAuto(u8"例句", (Vector2){ir.x + 20, ir.y + 140}, 18, 1, STYLE->theme.textSecondary);
                UIDrawTextRec(e->example, (Rectangle){ir.x + 75, ir.y + 140, ir.width - 95, 35}, 20, 1, true, STYLE->theme.textSecondary);
                if (e->exampleTranslation && *e->exampleTranslation) {
                    DrawTextAuto(u8"翻译", (Vector2){ir.x + 20, ir.y + 175}, 18, 1, (Color){100, 149, 237, 255});
                    UIDrawTextRec(e->exampleTranslation, (Rectangle){ir.x + 75, ir.y + 175, ir.width - 95, 35}, 20, 1, true, STYLE->theme.textPrimary);
                }
            }
        }
        UIEndScrollView(&sv, STYLE, UI_STATE);
    } else {
        Rectangle mr = {rp.x + 30, rp.y + 30, rp.width - 60, 60};
        DrawRectangleRounded(mr, 0.1f, 8, STYLE->theme.inputBg);
        Vector2 ms = MeasureTextAuto(u8"请在上方输入要查找的单词", 24, 1);
        DrawTextAuto(u8"请在上方输入要查找的单词",
            (Vector2){rp.x + rp.width/2 - ms.x/2, rp.y + rp.height/2 - ms.y/2}, 24, 1, STYLE->theme.textSecondary);
    }
}
