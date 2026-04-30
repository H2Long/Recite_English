// ============================================================================
// 词库管理页面 — 搜索、添加、编辑、删除单词
// ============================================================================

#include "pages.h"

/** g_wmState - 词库管理页面状态 */
static struct {
    int selectedIdx; int searchResults[100]; int searchCount;
    char searchQuery[64]; UITextBoxState searchBox; UITextBoxState fieldStates[5];
    bool isAdding; char msg[128]; int msgTimer;
} g_wmState = {0};

/** wmDoSearch - 词库管理页面的内部搜索函数 */
static void wmDoSearch(void) {
    g_wmState.searchCount = 0;
    const char* q = g_wmState.searchBox.buffer;
    if (strlen(q) == 0) { for (int i = 0; i < g_wordCount; i++) g_wmState.searchResults[g_wmState.searchCount++] = i; return; }
    char qLower[64]; strncpy(qLower, q, sizeof(qLower)-1);
    for (int j = 0; qLower[j]; j++) qLower[j] = tolower(qLower[j]);
    for (int i = 0; i < g_wordCount && g_wmState.searchCount < 100; i++) {
        char wLower[256]; strncpy(wLower, g_wordLibrary[i].word, sizeof(wLower)-1);
        for (int j = 0; wLower[j]; j++) wLower[j] = tolower(wLower[j]);
        if (strstr(wLower, qLower) != NULL) g_wmState.searchResults[g_wmState.searchCount++] = i;
    }
}

void MenuWordManager_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 20, 25);
    Rectangle titleR = UILayoutNext(&layout, -1, 50);
    DrawTextAuto(u8"词库管理", (Vector2){cr.x + 25, titleR.y}, 36, 1, STYLE->theme.primary);
    Rectangle searchR = UILayoutNext(&layout, -1, 45);
    Rectangle searchInput = {searchR.x, searchR.y, searchR.width - 90, 40};
    UITextBox(&g_wmState.searchBox, searchInput, STYLE, UI_STATE, false);
    Rectangle searchBtn = {searchR.x + searchR.width - 80, searchR.y, 75, 40};
    if (UIButton(u8"搜索", searchBtn, STYLE, UI_STATE, 600)) wmDoSearch();
    static int lastSearchLen = -1;
    int curLen = strlen(g_wmState.searchBox.buffer);
    if (curLen != lastSearchLen) { wmDoSearch(); lastSearchLen = curLen; }
    Rectangle mainR = UILayoutNext(&layout, -1, -1);
    UILayout mainL = UIBeginLayout(mainR, UI_DIR_HORIZONTAL, 15, 0);
    Rectangle listR = UILayoutNext(&mainL, 280, -1);
    DrawRectangleRounded(listR, 0.1f, 8, STYLE->theme.panelBg);
    static float g_wmListScroll = 0.0f;
    float listH = g_wmState.searchCount * 40.0f + 10;
    UIScrollView sv = {0}; sv.viewport = listR; sv.scrollOffset.y = g_wmListScroll;
    sv.contentSize = (Vector2){listR.width - 20, listH};
    UIBeginScrollView(&sv, listR, sv.contentSize);
    for (int i = 0; i < g_wmState.searchCount; i++) {
        int idx = g_wmState.searchResults[i];
        Rectangle item = {listR.x + 10, listR.y + 5 + i * 40 - sv.scrollOffset.y, listR.width - 20, 36};
        if (idx == g_wmState.selectedIdx) DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.25f));
        if (CheckCollisionPointRec(UI_STATE->mousePos, item)) {
            if (!(idx == g_wmState.selectedIdx)) DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.1f));
            if (UI_STATE->mouseReleased) {
                g_wmState.selectedIdx = idx; g_wmState.isAdding = false;
                for (int f = 0; f < 5; f++) memset(&g_wmState.fieldStates[f], 0, sizeof(UITextBoxState));
                strncpy(g_wmState.fieldStates[0].buffer, g_wordLibrary[idx].word ? g_wordLibrary[idx].word : "", 1023);
                strncpy(g_wmState.fieldStates[1].buffer, g_wordLibrary[idx].phonetic ? g_wordLibrary[idx].phonetic : "", 1023);
                strncpy(g_wmState.fieldStates[2].buffer, g_wordLibrary[idx].definition ? g_wordLibrary[idx].definition : "", 1023);
                strncpy(g_wmState.fieldStates[3].buffer, g_wordLibrary[idx].example ? g_wordLibrary[idx].example : "", 1023);
                strncpy(g_wmState.fieldStates[4].buffer, g_wordLibrary[idx].exampleTranslation ? g_wordLibrary[idx].exampleTranslation : "", 1023);
                snprintf(g_wmState.msg, sizeof(g_wmState.msg), "%s", "");
            }
        }
        DrawTextAuto(g_wordLibrary[idx].word, (Vector2){item.x + 8, item.y + 8}, 20, 1, STYLE->theme.textPrimary);
    }
    UIEndScrollView(&sv, STYLE, UI_STATE); g_wmListScroll = sv.scrollOffset.y;
    Rectangle editR = UILayoutNext(&mainL, -1, -1);
    DrawRectangleRounded(editR, 0.1f, 8, STYLE->theme.panelBg);
    UILayout editL = UIBeginLayout(editR, UI_DIR_VERTICAL, 12, 20);
    Rectangle editTitle = UILayoutNext(&editL, -1, 35);
    DrawTextAuto(g_wmState.isAdding ? u8"添加新单词" : (g_wmState.selectedIdx >= 0 ? u8"编辑单词" : u8"请选择一个单词"), (Vector2){editTitle.x, editTitle.y}, 24, 1, STYLE->theme.primary);
    const char* labels[5] = {u8"单词", u8"音标", u8"释义", u8"例句", u8"例句翻译"};
    for (int f = 0; f < 5; f++) {
        Rectangle lbR = UILayoutNext(&editL, -1, 28); DrawTextAuto(labels[f], (Vector2){lbR.x, lbR.y}, 18, 1, STYLE->theme.textSecondary);
        Rectangle inpR = UILayoutNext(&editL, -1, 38); UITextBox(&g_wmState.fieldStates[f], inpR, STYLE, UI_STATE, false);
    }
    Rectangle btnR = UILayoutNext(&editL, -1, 50);
    UILayout btnL = UIBeginLayout(btnR, UI_DIR_HORIZONTAL, 10, 0);
    if (strlen(g_wmState.msg) > 0) {
        Rectangle msgR = UILayoutNext(&editL, -1, 30);
        DrawTextAuto(g_wmState.msg, (Vector2){msgR.x, msgR.y}, 18, 1, (strstr(g_wmState.msg, "失败") || strstr(g_wmState.msg, "错误")) ? STYLE->theme.error : STYLE->theme.success);
    }
    Rectangle saveBtn = UILayoutNext(&btnL, 120, 42);
    if (UIButton(u8"保存", saveBtn, STYLE, UI_STATE, 601)) {
        for (int f = 1; f < 5; f++) if (strlen(g_wmState.fieldStates[f].buffer) == 0) strncpy(g_wmState.fieldStates[f].buffer, u8"无", 1023);
        if (g_wmState.isAdding) {
            if (addWordToLibrary(g_wmState.fieldStates[0].buffer, g_wmState.fieldStates[1].buffer, g_wmState.fieldStates[2].buffer, g_wmState.fieldStates[3].buffer, g_wmState.fieldStates[4].buffer)) {
                snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"添加成功！"); g_wmState.isAdding = false; reloadWords(); wmDoSearch();
            } else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"添加失败：单词不能为空");
        } else if (g_wmState.selectedIdx >= 0) {
            if (editWordInLibrary(g_wmState.selectedIdx, g_wmState.fieldStates[0].buffer, g_wmState.fieldStates[1].buffer, g_wmState.fieldStates[2].buffer, g_wmState.fieldStates[3].buffer, g_wmState.fieldStates[4].buffer)) {
                snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"保存成功！"); reloadWords(); wmDoSearch();
            } else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"保存失败");
        }
    }
    Rectangle addBtn = UILayoutNext(&btnL, 120, 42);
    if (UIButton(u8"新增", addBtn, STYLE, UI_STATE, 602)) { g_wmState.isAdding = true; g_wmState.selectedIdx = -1; for (int f = 0; f < 5; f++) memset(&g_wmState.fieldStates[f], 0, sizeof(UITextBoxState)); snprintf(g_wmState.msg, sizeof(g_wmState.msg), "%s", ""); }
    Rectangle delBtn = UILayoutNext(&btnL, 120, 42);
    if (UIButton(u8"删除", delBtn, STYLE, UI_STATE, 603)) {
        if (g_wmState.selectedIdx >= 0 && !g_wmState.isAdding) {
            if (deleteWordFromLibrary(g_wmState.selectedIdx)) { snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"删除成功！"); g_wmState.selectedIdx = -1; reloadWords(); wmDoSearch(); for (int f = 0; f < 5; f++) memset(&g_wmState.fieldStates[f], 0, sizeof(UITextBoxState)); }
            else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"删除失败");
        } else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"请先选择一个单词");
    }
}
