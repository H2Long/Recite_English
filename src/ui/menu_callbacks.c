// ============================================================================
// 菜单系统辅助函数实现
// 功能：菜单树初始化、销毁、名称获取、侧边栏绘制
// ============================================================================

#include "menu_callbacks.h"
#include <stdlib.h>

// ============================================================================
// 宏定义：简化访问器调用
// ============================================================================

#define STYLE    (AppState_GetStyle())
#define UI_STATE (AppState_GetUIState())
#define CURRENT_MENU (*AppState_GetCurrentMenu())

// 账号菜单节点（供 main.c 右上角点击跳转使用）
MENU* g_accountMenuNode = NULL;

// ============================================================================
// 菜单系统辅助函数
// ============================================================================

void InitMenuTree(void) {
    // 创建子菜单
    MENU* menuLearn = CreatMenuTreeNode(NULL, MenuLearn_Show);
    MENU* menuSearch = CreatMenuTreeNode(NULL, MenuSearch_Show);
    // 学习计划（父节点 + 子页面）
    MENU* menuPlanRoot = CreatMenuTreeNode(NULL, MenuPlanRoot_Show);
    MENU* menuPlanManager = CreatMenuTreeNode(NULL, MenuPlanManager_Show);
    MENU* menuProgress = CreatMenuTreeNode(NULL, MenuProgress_Show);
    ConnectMenuTree(menuPlanRoot, menuPlanManager);
    ConnectMenuTree(menuPlanRoot, menuProgress);

    MENU* menuSettings = CreatMenuTreeNode(NULL, MenuSettings_Show);
    MENU* menuAccount = CreatMenuTreeNode(NULL, MenuAccount_Show);
    MENU* menuWordManager = CreatMenuTreeNode(NULL, MenuWordManager_Show);

    // 背单词模式（父节点 + 三个子模式）
    MENU* menuReviewRoot = CreatMenuTreeNode(NULL, MenuReviewRoot_Show);
    MENU* menuCardReview = CreatMenuTreeNode(NULL, MenuCardReview_Show);
    MENU* menuSelectWord = CreatMenuTreeNode(NULL, MenuSelectWord_Show);
    MENU* menuTest = CreatMenuTreeNode(NULL, MenuTest_Show);
    ConnectMenuTree(menuReviewRoot, menuCardReview);
    ConnectMenuTree(menuReviewRoot, menuSelectWord);
    ConnectMenuTree(menuReviewRoot, menuTest);

    // 创建根菜单（主页面）
    g_app.rootMenu = CreatMenuTreeNode(NULL, MenuHome_Show);

    // 连接子菜单到根菜单
    ConnectMenuTree(g_app.rootMenu, menuLearn);
    ConnectMenuTree(g_app.rootMenu, menuReviewRoot);
    ConnectMenuTree(g_app.rootMenu, menuSearch);
    ConnectMenuTree(g_app.rootMenu, menuPlanRoot);
    ConnectMenuTree(g_app.rootMenu, menuSettings);
    ConnectMenuTree(g_app.rootMenu, menuAccount);
    ConnectMenuTree(g_app.rootMenu, menuWordManager);
    // 保存账号菜单节点引用（供右上角点击跳转使用）
    g_accountMenuNode = menuAccount;

    // 初始化菜单栈
    StackInit(&g_app.menuStack);
    StackPush(&g_app.menuStack, g_app.rootMenu);
    g_app.currentMenu = g_app.rootMenu;
}

void FreeMenuTree(void) {
    if (g_app.rootMenu == NULL) return;
    for (int i = 0; i < g_app.rootMenu->childindex; i++) {
        if (g_app.rootMenu->child[i]) free(g_app.rootMenu->child[i]);
    }
    free(g_app.rootMenu);
    g_app.rootMenu = NULL;
    g_app.currentMenu = NULL;
}

const char* GetMenuItemText(MENU* menu) {
    if (menu == NULL) return "";
    if (menu == g_app.rootMenu) return u8"主菜单";
    if (menu->show == MenuLearn_Show) return u8"学单词";
    if (menu->show == MenuReviewRoot_Show) return u8"背单词";
    if (menu->show == MenuCardReview_Show) return u8"卡片背单词";
    if (menu->show == MenuSelectWord_Show) return u8"选词背单词";
    if (menu->show == MenuTest_Show) return u8"测试";
    if (menu->show == MenuSearch_Show) return u8"查找单词";
    if (menu->show == MenuPlanRoot_Show) return u8"学习计划";
    if (menu->show == MenuPlanManager_Show) return u8"计划管理";
    if (menu->show == MenuProgress_Show) return u8"学习进度";
    if (menu->show == MenuSettings_Show) return u8"设置";
    if (menu->show == MenuAccount_Show) return u8"账号管理";
    if (menu->show == MenuWordManager_Show) return u8"词库管理";
    return "";
}

// 侧边菜单滚动位置（跨帧保存）
static float g_menuScrollOffset = 0.0f;

void DrawTreeMenu(Rectangle menuRect) {
    if (g_app.currentMenu == NULL) return;

    DrawRectangleRounded(menuRect, 0.1f, 8, STYLE->theme.panelBg);
    UILayout layout = UIBeginLayout(menuRect, UI_DIR_VERTICAL, 12, 12);

    // 非根菜单显示标题和返回按钮
    if (g_app.currentMenu != g_app.rootMenu) {
        Rectangle titleRect = UILayoutNext(&layout, -1, 45);
        const char* title = GetMenuItemText(g_app.currentMenu);
        Vector2 tSize = MeasureTextAuto(title, 28, 1);
        DrawTextAuto(title, (Vector2){menuRect.x + menuRect.width/2 - tSize.x/2, titleRect.y + 8}, 28, 1, STYLE->theme.primary);

        Rectangle backRect = UILayoutNext(&layout, -1, 50);
        if (UIButton(u8"返回", backRect, STYLE, UI_STATE, 6)) {
            MENU* prev = StackPop(AppState_GetMenuStack());
            if (prev != NULL && g_app.menuStack.Stacktop >= 0)
                g_app.currentMenu = g_app.menuStack.menuStack[g_app.menuStack.Stacktop];
            else
                g_app.currentMenu = g_app.rootMenu;
        }

        Rectangle lineRect = UILayoutNext(&layout, -1, 3);
        DrawLineEx((Vector2){lineRect.x, lineRect.y}, (Vector2){lineRect.x + lineRect.width, lineRect.y}, 1, STYLE->theme.inputBorder);
    }

    // 绘制子菜单项
    if (g_app.currentMenu->childindex > 0) {
        Rectangle itemsRect = UILayoutNext(&layout, -1, -1);
        UIScrollView sv = {0};
        sv.viewport = itemsRect;
        sv.scrollOffset.y = g_menuScrollOffset;
        float totalH = g_app.currentMenu->childindex * 55.0f;
        sv.contentSize = (Vector2){itemsRect.width, totalH};
        UIBeginScrollView(&sv, itemsRect, sv.contentSize);

        for (int i = 0; i < g_app.currentMenu->childindex; i++) {
            MENU* child = g_app.currentMenu->child[i];
            if (child == NULL) continue;
            Rectangle itemRect = {itemsRect.x, itemsRect.y + i * 55 - sv.scrollOffset.y, itemsRect.width, 55};
            if (UIButton(GetMenuItemText(child), itemRect, STYLE, UI_STATE, 200 + i)) {
                if (child->show != NULL) {
                    StackPush(AppState_GetMenuStack(), child);
                    g_app.currentMenu = child;
                }
            }
        }
        UIEndScrollView(&sv, STYLE, UI_STATE);
        g_menuScrollOffset = sv.scrollOffset.y;
    }

    // 根菜单显示标题
    if (g_app.currentMenu == g_app.rootMenu) {
        Rectangle welcomeRect = UILayoutNext(&layout, -1, 50);
        Vector2 wSize = MeasureTextAuto(u8"主菜单", 30, 1);
        DrawTextAuto(u8"主菜单", (Vector2){menuRect.x + menuRect.width/2 - wSize.x/2, welcomeRect.y + 8}, 30, 1, STYLE->theme.primary);
        Rectangle lineRect = UILayoutNext(&layout, -1, 3);
        DrawLineEx((Vector2){lineRect.x, lineRect.y}, (Vector2){lineRect.x + lineRect.width, lineRect.y}, 1, STYLE->theme.inputBorder);
    }
}
