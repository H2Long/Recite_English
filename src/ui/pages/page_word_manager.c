// ============================================================================
// 词库管理页面
// 功能：搜索单词、查看详情、添加/编辑/删除单词
// 布局：搜索栏 → 左侧单词列表 → 右侧编辑表单
// ============================================================================

#include "pages.h"

/** g_wmState - 词库管理页面的所有交互状态 */
static struct {
    int selectedIdx;                /**< 当前选中的单词索引（-1=未选中） */
    int searchResults[100];         /**< 搜索结果在 g_wordLibrary 中的索引 */
    int searchCount;                /**< 当前搜索结果数量 */
    char searchQuery[64];           /**< 搜索关键词 */
    UITextBoxState searchBox;       /**< 搜索输入框状态 */
    UITextBoxState fieldStates[5];  /**< 编辑框：[0]单词 [1]音标 [2]释义 [3]例句 [4]例句翻译 */
    bool isAdding;                  /**< 是否处于"添加新单词"模式 */
    char msg[128];                  /**< 操作结果提示消息 */
    int msgTimer;                   /**< 消息显示计时 */
} g_wmState = {0};

/**
 * wmDoSearch - 词库管理页面内部搜索
 * 按搜索框内容对 g_wordLibrary 做不区分大小写的子串匹配。
 * 搜索框为空时列出所有单词。
 */
static void wmDoSearch(void) {
    g_wmState.searchCount = 0;
    const char* q = g_wmState.searchBox.buffer;
    if (strlen(q) == 0) {
        for (int i = 0; i < g_wordCount; i++) g_wmState.searchResults[g_wmState.searchCount++] = i;
        return;
    }
    char ql[64]; strncpy(ql, q, sizeof(ql)-1);
    for (int j = 0; ql[j]; j++) ql[j] = tolower(ql[j]);
    for (int i = 0; i < g_wordCount && g_wmState.searchCount < 100; i++) {
        char wl[256]; strncpy(wl, g_wordLibrary[i].word, sizeof(wl)-1);
        for (int j = 0; wl[j]; j++) wl[j] = tolower(wl[j]);
        if (strstr(wl, ql) != NULL) g_wmState.searchResults[g_wmState.searchCount++] = i;
    }
}

/**
 * MenuWordManager_Show - 词库管理页面
 *
 * 搜索栏 → 左侧列表（点击选中）→ 右侧编辑表单
 * 支持：编辑已有单词的 5 个字段、新增单词、删除单词
 * 所有修改自动保存到 words.txt
 * 注意：g_wmState 中的 static 状态变量跨帧保持
 */
void MenuWordManager_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(cr, UI_DIR_VERTICAL, 20, 25);
    DrawTextAuto(u8"词库管理", (Vector2){cr.x + 25, UILayoutNext(&layout, -1, 50).y}, 36, 1, STYLE->theme.primary);

    // 搜索栏
    Rectangle sr = UILayoutNext(&layout, -1, 45);
    UITextBox(&g_wmState.searchBox, (Rectangle){sr.x, sr.y, sr.width - 90, 40}, STYLE, UI_STATE, false);
    if (UIButton(u8"搜索", (Rectangle){sr.x + sr.width - 80, sr.y, 75, 40}, STYLE, UI_STATE, 600)) wmDoSearch();

    static int lastLen = -1;
    int curLen = strlen(g_wmState.searchBox.buffer);
    if (curLen != lastLen) { wmDoSearch(); lastLen = curLen; }

    // 左侧列表 + 右侧编辑
    Rectangle mr = UILayoutNext(&layout, -1, -1);
    UILayout ml = UIBeginLayout(mr, UI_DIR_HORIZONTAL, 15, 0);
    Rectangle lr = UILayoutNext(&ml, 280, -1);
    DrawRectangleRounded(lr, 0.1f, 8, STYLE->theme.panelBg);

    static float listScroll = 0.0f;
    UIScrollView sv = {0}; sv.viewport = lr; sv.scrollOffset.y = listScroll;
    sv.contentSize = (Vector2){lr.width - 20, g_wmState.searchCount * 40.0f + 10};
    UIBeginScrollView(&sv, lr, sv.contentSize);

    for (int i = 0; i < g_wmState.searchCount; i++) {
        int idx = g_wmState.searchResults[i];
        Rectangle item = {lr.x + 10, lr.y + 5 + i * 40 - sv.scrollOffset.y, lr.width - 20, 36};
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
    UIEndScrollView(&sv, STYLE, UI_STATE); listScroll = sv.scrollOffset.y;

    // 右侧编辑表单
    Rectangle er2 = UILayoutNext(&ml, -1, -1);
    DrawRectangleRounded(er2, 0.1f, 8, STYLE->theme.panelBg);
    UILayout el2 = UIBeginLayout(er2, UI_DIR_VERTICAL, 12, 20);
    Rectangle et = UILayoutNext(&el2, -1, 35);
    DrawTextAuto(g_wmState.isAdding ? u8"添加新单词"
        : (g_wmState.selectedIdx >= 0 ? u8"编辑单词" : u8"请选择一个单词"),
        (Vector2){et.x, et.y}, 24, 1, STYLE->theme.primary);

    const char* lbl[5] = {u8"单词", u8"音标", u8"释义", u8"例句", u8"例句翻译"};
    for (int f = 0; f < 5; f++) {
        DrawTextAuto(lbl[f], (Vector2){(UILayoutNext(&el2, -1, 28)).x, (UILayoutNext(&el2, -1, 28)).y}, 18, 1, STYLE->theme.textSecondary);
        UITextBox(&g_wmState.fieldStates[f], UILayoutNext(&el2, -1, 38), STYLE, UI_STATE, false);
    }
    Rectangle br2 = UILayoutNext(&el2, -1, 50);
    UILayout bl2 = UIBeginLayout(br2, UI_DIR_HORIZONTAL, 10, 0);
    if (strlen(g_wmState.msg) > 0) {
        Rectangle mr2 = UILayoutNext(&el2, -1, 30);
        DrawTextAuto(g_wmState.msg, (Vector2){mr2.x, mr2.y}, 18, 1,
            (strstr(g_wmState.msg, "失败") || strstr(g_wmState.msg, "错误")) ? STYLE->theme.error : STYLE->theme.success);
    }

    if (UIButton(u8"保存", UILayoutNext(&bl2, 120, 42), STYLE, UI_STATE, 601)) {
        for (int f = 1; f < 5; f++)
            if (strlen(g_wmState.fieldStates[f].buffer) == 0)
                strncpy(g_wmState.fieldStates[f].buffer, u8"无", 1023);
        if (g_wmState.isAdding) {
            if (addWordToLibrary(g_wmState.fieldStates[0].buffer, g_wmState.fieldStates[1].buffer,
                g_wmState.fieldStates[2].buffer, g_wmState.fieldStates[3].buffer, g_wmState.fieldStates[4].buffer))
                { snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"添加成功！"); g_wmState.isAdding = false; reloadWords(); wmDoSearch(); }
            else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"添加失败：单词不能为空");
        } else if (g_wmState.selectedIdx >= 0) {
            if (editWordInLibrary(g_wmState.selectedIdx, g_wmState.fieldStates[0].buffer,
                g_wmState.fieldStates[1].buffer, g_wmState.fieldStates[2].buffer,
                g_wmState.fieldStates[3].buffer, g_wmState.fieldStates[4].buffer))
                { snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"保存成功！"); reloadWords(); wmDoSearch(); }
            else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"保存失败");
        }
    }
    if (UIButton(u8"新增", UILayoutNext(&bl2, 120, 42), STYLE, UI_STATE, 602))
        { g_wmState.isAdding = true; g_wmState.selectedIdx = -1; for (int f = 0; f < 5; f++) memset(&g_wmState.fieldStates[f], 0, sizeof(UITextBoxState)); snprintf(g_wmState.msg, sizeof(g_wmState.msg), "%s", ""); }
    if (UIButton(u8"删除", UILayoutNext(&bl2, 120, 42), STYLE, UI_STATE, 603)) {
        if (g_wmState.selectedIdx >= 0 && !g_wmState.isAdding) {
            if (deleteWordFromLibrary(g_wmState.selectedIdx))
                { snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"删除成功！"); g_wmState.selectedIdx = -1; reloadWords(); wmDoSearch(); for (int f = 0; f < 5; f++) memset(&g_wmState.fieldStates[f], 0, sizeof(UITextBoxState)); }
            else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"删除失败");
        } else snprintf(g_wmState.msg, sizeof(g_wmState.msg), u8"请先选择一个单词");
    }
}
