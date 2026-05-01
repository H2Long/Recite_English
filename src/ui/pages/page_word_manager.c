// 词库管理 — 搜索+CRUD

#include "pages.h"

static struct {
    int selIdx;
    int results[100];
    int resultCnt;
    char query[64];
    UITextBoxState searchBox;
    UITextBoxState fields[5];    // 0=单词 1=音标 2=释义 3=例句 4=翻译
    bool isAdding;
    char msg[128];
} g_wmState = {0};

static void wmDoSearch(void) {
    g_wmState.resultCnt = 0;
    const char* q = g_wmState.searchBox.buffer;
    if(strlen(q) == 0) {
        for (int i = 0; i < g_wordCount; i++) {
            g_wmState.results[g_wmState.resultCnt++] = i;
        }
        return ;
    }
    char ql[64];
    strcpy(ql, q);
    for (int j = 0; ql[j]; j++) ql[j] = tolower(ql[j]);
    for (int i = 0; i < g_wordCount && g_wmState.resultCnt < 100; i++) {
        char wl[256];
        strcpy(wl, g_wordLibrary[i].word);
        for (int j = 0; wl[j]; j++) wl[j] = tolower(wl[j]);
        if(strstr(wl, ql)) {
            g_wmState.results[g_wmState.resultCnt++] = i;
        }
    }
}

void MenuWordManager_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_VERTICAL, 20, 25);

    DrawTextAuto(u8"词库管理", (Vector2){cr.x + 25, UILayoutNext(&lay, -1, 50).y},
        36, 1, STYLE->theme.primary);

    // 搜索栏
    Rectangle sr = UILayoutNext(&lay, -1, 45);
    UITextBox(&g_wmState.searchBox, (Rectangle){sr.x, sr.y, sr.width - 90, 40},
        STYLE, UI_STATE, false);
    if(UIButton(u8"搜索", (Rectangle){sr.x+sr.width-80, sr.y, 75, 40}, STYLE, UI_STATE, 600)) {
        wmDoSearch();
    }
    static int lastLen = -1;
    int curLen = strlen(g_wmState.searchBox.buffer);
    if(curLen != lastLen) { wmDoSearch(); lastLen = curLen; }

    // 主体: 左侧列表 + 右侧编辑
    Rectangle mr = UILayoutNext(&lay, -1, -1);
    UILayout ml = UIBeginLayout(mr, UI_DIR_HORIZONTAL, 15, 0);

    // 左侧列表
    Rectangle lr = UILayoutNext(&ml, 280, -1);
    DrawRectangleRounded(lr, 0.1f, 8, STYLE->theme.panelBg);
    static float listScroll = 0;
    UIScrollView sv = {0};
    sv.viewport = lr;
    sv.scrollOffset.y = listScroll;
    sv.contentSize = (Vector2){lr.width - 20, g_wmState.resultCnt * 40.0f + 10};
    UIBeginScrollView(&sv, lr, sv.contentSize);

    for (int i = 0; i < g_wmState.resultCnt; i++) {
        int idx = g_wmState.results[i];
        Rectangle item = {lr.x + 10, lr.y + 5 + i*40 - sv.scrollOffset.y, lr.width - 20, 36};
        if(idx == g_wmState.selIdx) {
            DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.25f));
        }
        if(CheckCollisionPointRec(UI_STATE->mousePos, item)) {
            if(idx != g_wmState.selIdx) { DrawRectangleRec(item, Fade(STYLE->theme.primary, 0.1f)); }
            if(UI_STATE->mouseReleased) {
                g_wmState.selIdx = idx;
                g_wmState.isAdding = false;
                for (int f = 0; f < 5; f++) memset(&g_wmState.fields[f], 0, sizeof(UITextBoxState));
                strcpy(g_wmState.fields[0].buffer, g_wordLibrary[idx].word ? g_wordLibrary[idx].word : "");
                strcpy(g_wmState.fields[1].buffer, g_wordLibrary[idx].phonetic ? g_wordLibrary[idx].phonetic : "");
                strcpy(g_wmState.fields[2].buffer, g_wordLibrary[idx].definition ? g_wordLibrary[idx].definition : "");
                strcpy(g_wmState.fields[3].buffer, g_wordLibrary[idx].example ? g_wordLibrary[idx].example : "");
                strcpy(g_wmState.fields[4].buffer, g_wordLibrary[idx].exampleTranslation ? g_wordLibrary[idx].exampleTranslation : "");
                g_wmState.msg[0] = '\0';
            }
        }
        DrawTextAuto(g_wordLibrary[idx].word, (Vector2){item.x + 8, item.y + 8},
            20, 1, STYLE->theme.textPrimary);
    }
    UIEndScrollView(&sv, STYLE, UI_STATE);
    listScroll = sv.scrollOffset.y;

    // 右侧编辑表单
    Rectangle er = UILayoutNext(&ml, -1, -1);
    DrawRectangleRounded(er, 0.1f, 8, STYLE->theme.panelBg);
    UILayout el = UIBeginLayout(er, UI_DIR_VERTICAL, 12, 20);

    Rectangle et = UILayoutNext(&el, -1, 35);
    DrawTextAuto(g_wmState.isAdding ? u8"添加新单词"
        : (g_wmState.selIdx >= 0 ? u8"编辑单词" : u8"请选择一个单词"),
        (Vector2){et.x, et.y}, 24, 1, STYLE->theme.primary);

    const char* lbl[5] = {u8"单词", u8"音标", u8"释义", u8"例句", u8"例句翻译"};
    for (int f = 0; f < 5; f++) {
        DrawTextAuto(lbl[f], (Vector2){UILayoutNext(&el, -1, 28).x, UILayoutNext(&el, -1, 28).y},
            18, 1, STYLE->theme.textSecondary);
        UITextBox(&g_wmState.fields[f], UILayoutNext(&el, -1, 38), STYLE, UI_STATE, false);
    }

    if(strlen(g_wmState.msg) > 0) {
        Rectangle mr2 = UILayoutNext(&el, -1, 30);
        DrawTextAuto(g_wmState.msg, (Vector2){mr2.x, mr2.y}, 18, 1,
            (strstr(g_wmState.msg, "失败")||strstr(g_wmState.msg, "错误"))
                ? STYLE->theme.error : STYLE->theme.success);
    }

    Rectangle br = UILayoutNext(&el, -1, 50);
    UILayout bl = UIBeginLayout(br, UI_DIR_HORIZONTAL, 10, 0);

    if(UIButton(u8"保存", UILayoutNext(&bl, 120, 42), STYLE, UI_STATE, 601)) {
        for (int f = 1; f < 5; f++) {
            if(strlen(g_wmState.fields[f].buffer) == 0) {
                strcpy(g_wmState.fields[f].buffer, u8"无");
            }
        }
        if(g_wmState.isAdding) {
            if(addWordToLibrary(g_wmState.fields[0].buffer, g_wmState.fields[1].buffer,
                g_wmState.fields[2].buffer, g_wmState.fields[3].buffer,
                g_wmState.fields[4].buffer)) {
                strcpy(g_wmState.msg, u8"添加成功！");
                g_wmState.isAdding = false;
                reloadWords();
                wmDoSearch();
            }
            else strcpy(g_wmState.msg, u8"添加失败：单词不能为空");
        }
        else if(g_wmState.selIdx >= 0) {
            if(editWordInLibrary(g_wmState.selIdx, g_wmState.fields[0].buffer,
                g_wmState.fields[1].buffer, g_wmState.fields[2].buffer,
                g_wmState.fields[3].buffer, g_wmState.fields[4].buffer)) {
                strcpy(g_wmState.msg, u8"保存成功！");
                reloadWords();
                wmDoSearch();
            }
            else strcpy(g_wmState.msg, u8"保存失败");
        }
    }
    if(UIButton(u8"新增", UILayoutNext(&bl, 120, 42), STYLE, UI_STATE, 602)) {
        g_wmState.isAdding = true;
        g_wmState.selIdx = -1;
        for (int f = 0; f < 5; f++) memset(&g_wmState.fields[f], 0, sizeof(UITextBoxState));
        g_wmState.msg[0] = '\0';
    }
    if(UIButton(u8"删除", UILayoutNext(&bl, 120, 42), STYLE, UI_STATE, 603)) {
        if(g_wmState.selIdx >= 0 && !g_wmState.isAdding) {
            if(deleteWordFromLibrary(g_wmState.selIdx)) {
                strcpy(g_wmState.msg, u8"删除成功！");
                g_wmState.selIdx = -1;
                reloadWords();
                wmDoSearch();
                for (int f = 0; f < 5; f++) memset(&g_wmState.fields[f], 0, sizeof(UITextBoxState));
            }
            else strcpy(g_wmState.msg, u8"删除失败");
        }
        else strcpy(g_wmState.msg, u8"请先选择一个单词");
    }
}
