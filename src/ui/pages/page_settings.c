// ============================================================================
// 设置页面 — 深色模式切换 + 关于信息
// ============================================================================

#include "pages.h"

void MenuSettings_Show(void) {
    Rectangle contentRect = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout layout = UIBeginLayout(contentRect, UI_DIR_VERTICAL, 30, 50);

    Rectangle titleRect = UILayoutNext(&layout, -1, 60);
    Vector2 tSize = MeasureTextAuto(u8"设置", 42, 1);
    DrawTextAuto(u8"设置", (Vector2){contentRect.x + 50, titleRect.y}, 42, 1, STYLE->theme.textPrimary);

    Rectangle panelRect = UILayoutNext(&layout, -1, -1);
    DrawRectangleRounded(panelRect, 0.1f, 12, STYLE->theme.panelBg);
    UILayout settingsLayout = UIBeginLayout(panelRect, UI_DIR_VERTICAL, 20, 30);

    Rectangle themeSection = UILayoutNext(&settingsLayout, -1, 120);
    DrawTextAuto(u8"主题设置", (Vector2){themeSection.x + 30, themeSection.y + 10}, 28, 1, STYLE->theme.textPrimary);
    Rectangle checkboxRect = {themeSection.x + 30, themeSection.y + 55, 300, 40};
    if (UICheckbox(u8"深色模式", checkboxRect, &g_app.isDarkMode, STYLE, UI_STATE)) AppState_SetDarkMode(g_app.isDarkMode);
    Rectangle hintRect = {themeSection.x + 30, themeSection.y + 100, 500, 30};
    DrawTextAuto(AppState_IsDarkMode() ? u8"当前：深色模式" : u8"当前：浅色模式", (Vector2){hintRect.x, hintRect.y}, 18, 1, STYLE->theme.textSecondary);

    Rectangle aboutSection = UILayoutNext(&settingsLayout, -1, 120);
    DrawTextAuto(u8"关于", (Vector2){aboutSection.x + 30, aboutSection.y + 10}, 28, 1, STYLE->theme.textPrimary);
    Rectangle aboutContent = {aboutSection.x + 30, aboutSection.y + 50, 600, 60};
    UIDrawTextRec(u8"背单词软件 v5.0.0\n基于 raylib 构建，支持中英文混合显示", aboutContent, 18, 1, true, STYLE->theme.textSecondary);
}
