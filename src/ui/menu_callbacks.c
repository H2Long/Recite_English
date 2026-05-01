// 菜单回调 — 树构建/导航栏

#include "menu_callbacks.h"
#include <stdlib.h>

#define STYLE    (AppState_GetStyle())
#define UI_STATE (AppState_GetUIState())
#define CURRENT_MENU (*AppState_GetCurrentMenu())

MENU* g_accountMenuNode = NULL;

void InitMenuTree(void)
{
    MENU* learn = CreatMenuTreeNode(NULL, MenuLearn_Show);
    MENU* search = CreatMenuTreeNode(NULL, MenuSearch_Show);
    MENU* planRoot = CreatMenuTreeNode(NULL, MenuPlanRoot_Show);
    MENU* planMgr = CreatMenuTreeNode(NULL, MenuPlanManager_Show);
    MENU* progress = CreatMenuTreeNode(NULL, MenuProgress_Show);
    ConnectMenuTree(planRoot, planMgr);
    ConnectMenuTree(planRoot, progress);
    MENU* settings = CreatMenuTreeNode(NULL, MenuSettings_Show);
    MENU* account = CreatMenuTreeNode(NULL, MenuAccount_Show);
    MENU* wordMgr = CreatMenuTreeNode(NULL, MenuWordManager_Show);
    MENU* reviewRoot = CreatMenuTreeNode(NULL, MenuReviewRoot_Show);
    MENU* cardReview = CreatMenuTreeNode(NULL, MenuCardReview_Show);
    MENU* selectWord = CreatMenuTreeNode(NULL, MenuSelectWord_Show);
    MENU* test = CreatMenuTreeNode(NULL, MenuTest_Show);
    ConnectMenuTree(reviewRoot, cardReview);
    ConnectMenuTree(reviewRoot, selectWord);
    ConnectMenuTree(reviewRoot, test);

    g_app.rootMenu = CreatMenuTreeNode(NULL, MenuHome_Show);
    ConnectMenuTree(g_app.rootMenu, learn);
    ConnectMenuTree(g_app.rootMenu, reviewRoot);
    ConnectMenuTree(g_app.rootMenu, search);
    ConnectMenuTree(g_app.rootMenu, planRoot);
    ConnectMenuTree(g_app.rootMenu, settings);
    ConnectMenuTree(g_app.rootMenu, account);
    ConnectMenuTree(g_app.rootMenu, wordMgr);
    g_accountMenuNode = account;

    StackInit(&g_app.menuStack);
    StackPush(&g_app.menuStack, g_app.rootMenu);
    g_app.currentMenu = g_app.rootMenu;
}

void FreeMenuTree(void) {
    if(g_app.rootMenu == NULL) { return ; }
    for (int i = 0; i < g_app.rootMenu->childindex; i++) {
        if(g_app.rootMenu->child[i]) { free(g_app.rootMenu->child[i]); }
    }
    free(g_app.rootMenu);
    g_app.rootMenu = NULL;
    g_app.currentMenu = NULL;
}

const char* GetMenuItemText(MENU* menu) {
    if(menu == NULL) { return ""; }
    if(menu == g_app.rootMenu) { return u8"主菜单"; }
    if(menu->show == MenuLearn_Show) { return u8"学单词"; }
    if(menu->show == MenuReviewRoot_Show) { return u8"背单词"; }
    if(menu->show == MenuCardReview_Show) { return u8"卡片背单词"; }
    if(menu->show == MenuSelectWord_Show) { return u8"选词背单词"; }
    if(menu->show == MenuTest_Show) { return u8"测试"; }
    if(menu->show == MenuSearch_Show) { return u8"查找单词"; }
    if(menu->show == MenuPlanRoot_Show) { return u8"学习计划"; }
    if(menu->show == MenuPlanManager_Show) { return u8"计划管理"; }
    if(menu->show == MenuProgress_Show) { return u8"学习进度"; }
    if(menu->show == MenuSettings_Show) { return u8"设置"; }
    if(menu->show == MenuAccount_Show) { return u8"账号管理"; }
    if(menu->show == MenuWordManager_Show) { return u8"词库管理"; }
    return "";
}

static float g_menuScrollOffset = 0.0f;

void DrawTreeMenu(Rectangle menuRect) {
    if(CURRENT_MENU == NULL) { return ; }
    DrawRectangleRounded(menuRect, 0.1f, 8, STYLE->theme.panelBg);
    UILayout lay = UIBeginLayout(menuRect, UI_DIR_VERTICAL, 12, 12);

    if(CURRENT_MENU != g_app.rootMenu) {
        Rectangle tr = UILayoutNext(&lay, -1, 45);
        const char* t = GetMenuItemText(CURRENT_MENU);
        Vector2 ts = MeasureTextAuto(t, 28, 1);
        DrawTextAuto(t, (Vector2){menuRect.x + menuRect.width/2 - ts.x/2, tr.y + 8},
            28, 1, STYLE->theme.primary);
        Rectangle br = UILayoutNext(&lay, -1, 50);
        if(UIButton(u8"返回", br, STYLE, UI_STATE, 6)) {
            MENU* prev = StackPop(AppState_GetMenuStack());
            if(prev != NULL && g_app.menuStack.Stacktop >= 0) {
                CURRENT_MENU = g_app.menuStack.menuStack[g_app.menuStack.Stacktop];
            }
            else { CURRENT_MENU = g_app.rootMenu; }
        }
        Rectangle lr = UILayoutNext(&lay, -1, 3);
        DrawLineEx((Vector2){lr.x,lr.y}, (Vector2){lr.x+lr.width,lr.y}, 1, STYLE->theme.inputBorder);
    }

    if(CURRENT_MENU->childindex > 0) {
        Rectangle ir = UILayoutNext(&lay, -1, -1);
        UIScrollView sv = {0};
        sv.viewport = ir;
        sv.scrollOffset.y = g_menuScrollOffset;
        sv.contentSize = (Vector2){ir.width, CURRENT_MENU->childindex * 55.0f};
        UIBeginScrollView(&sv, ir, sv.contentSize);
        for (int i = 0; i < CURRENT_MENU->childindex; i++) {
            MENU* c = CURRENT_MENU->child[i];
            if(c == NULL) { continue; }
            Rectangle item = {ir.x, ir.y + i*55 - sv.scrollOffset.y, ir.width, 55};
            if(UIButton(GetMenuItemText(c), item, STYLE, UI_STATE, 200 + i)) {
                if(c->show != NULL) {
                    StackPush(AppState_GetMenuStack(), c);
                    CURRENT_MENU = c;
                }
            }
        }
        UIEndScrollView(&sv, STYLE, UI_STATE);
        g_menuScrollOffset = sv.scrollOffset.y;
    }

    if(CURRENT_MENU == g_app.rootMenu) {
        Rectangle wr = UILayoutNext(&lay, -1, 50);
        Vector2 ws = MeasureTextAuto(u8"主菜单", 30, 1);
        DrawTextAuto(u8"主菜单", (Vector2){menuRect.x+menuRect.width/2-ws.x/2, wr.y+8},
            30, 1, STYLE->theme.primary);
        Rectangle lr = UILayoutNext(&lay, -1, 3);
        DrawLineEx((Vector2){lr.x,lr.y}, (Vector2){lr.x+lr.width,lr.y}, 1, STYLE->theme.inputBorder);
    }
}
