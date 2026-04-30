// ============================================================================
// 主菜单页面
// 功能：显示欢迎信息、学习统计卡片、三个快捷功能入口
// ============================================================================

#include "pages.h"

/**
 * MenuHome_Show - 渲染主菜单页面
 *
 * 布局从上到下：
 *   1. 欢迎标题 + 副标题（居中）
 *   2. 统计卡片：总单词数、已掌握、学习中（水平排列）
 *   3. 三个功能卡片：学单词 / 背单词 / 查找单词
 *      每个卡片包含标题、描述和"开始"按钮
 *
 * 点击"开始"按钮后，通过菜单栈导航到对应的子页面。
 * 菜单节点引用从 g_app.rootMenu->child[0~2] 获取。
 */
void MenuHome_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    // ---- 欢迎信息 ----
    Rectangle welcomeRect = UILayoutNext(&layout, -1, 90);
    Vector2 wSize = MeasureTextAuto(u8"欢迎使用背单词软件！", 52, 1);
    DrawTextAuto(u8"欢迎使用背单词软件！",
        (Vector2){SCREEN_WIDTH/2 - wSize.x/2, welcomeRect.y}, 52, 1, STYLE->theme.primary);

    Rectangle descRect = UILayoutNext(&layout, -1, 60);
    Vector2 dSize = MeasureTextAuto(u8"选择左侧导航栏中的模式开始学习", 28, 1);
    DrawTextAuto(u8"选择左侧导航栏中的模式开始学习",
        (Vector2){SCREEN_WIDTH/2 - dSize.x/2, descRect.y}, 28, 1, STYLE->theme.textSecondary);

    // ---- 统计卡片：遍历 g_words 统计已掌握数 ----
    Rectangle statsRect = UILayoutNext(&layout, -1, 100);
    DrawRectangleRounded(statsRect, 0.1f, 12, STYLE->theme.panelBg);

    int mastered = 0;
    for (int i = 0; i < g_wordProgressCount; i++)
        if (g_words[i].progress.mastered) mastered++;

    char statsText[256];
    snprintf(statsText, sizeof(statsText), u8"总单词数: %d  |  已掌握: %d  |  学习中: %d",
        g_wordProgressCount, mastered, g_wordProgressCount - mastered);

    UILayout statsLayout = UIBeginLayout(statsRect, UI_DIR_HORIZONTAL, 0, 25);
    Rectangle statsTextRect = UILayoutNext(&statsLayout, -1, -1);
    DrawTextAuto(statsText, (Vector2){statsTextRect.x, statsTextRect.y + 28}, 22, 1, STYLE->theme.textPrimary);

    // ---- 三个功能入口卡片 ----
    Rectangle cardsRect = UILayoutNext(&layout, -1, 160);
    UILayout cardsLayout = UIBeginLayout(cardsRect, UI_DIR_HORIZONTAL, 30, 0);

    // 卡片配置：标题、描述、对应菜单节点（从根菜单的子节点获取）、边框颜色
    struct { const char* title; const char* desc; MENU* target; Color color; } modes[] = {
        {u8"学单词", u8"详细学习每个单词的释义和用法", NULL, (Color){70, 130, 180, 255}},
        {u8"背单词", u8"包含卡片背单词、选词背单词、测试三种模式", NULL, (Color){60, 179, 113, 255}},
        {u8"查找单词", u8"正则表达式快速查找", NULL, (Color){138, 43, 226, 255}}
    };
    for (int i = 0; i < g_app.rootMenu->childindex && i < 3; i++)
        modes[i].target = g_app.rootMenu->child[i];

    for (int i = 0; i < 3; i++) {
        Rectangle cardRect = UILayoutNext(&cardsLayout, 240, -1);
        DrawRectangleRounded(cardRect, 0.1f, 12, STYLE->theme.panelBg);
        DrawRectangleRoundedLines(cardRect, 0.1f, 12, modes[i].color);
        DrawTextAuto(modes[i].title,
            (Vector2){cardRect.x + 15, cardRect.y + 15}, 32, 1, modes[i].color);

        Rectangle descArea = {cardRect.x + 15, cardRect.y + 55,
                              cardRect.width - 30, cardRect.height - 80};
        UIDrawTextRec(modes[i].desc, descArea, 18, 1, true, STYLE->theme.textSecondary);

        Rectangle goBtn = {cardRect.x + cardRect.width - 100,
                           cardRect.y + cardRect.height - 55, 85, 45};
        if (UIButton(u8"开始", goBtn, STYLE, UI_STATE, 100 + i) && modes[i].target != NULL) {
            StackPush(AppState_GetMenuStack(), modes[i].target);
            CURRENT_MENU = modes[i].target;
        }
    }
}
