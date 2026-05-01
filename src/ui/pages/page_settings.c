// 设置 — 主题切换+关于信息

#include "pages.h"

void MenuSettings_Show(void) {
    Rectangle cr = {250, 80, SCREEN_WIDTH - 270, SCREEN_HEIGHT - 100};
    UILayout lay = UIBeginLayout(cr, UI_DIR_VERTICAL, 30, 50);

    Rectangle tr = UILayoutNext(&lay, -1, 60);
    Vector2 ts = MeasureTextAuto(u8"设置", 42, 1);
    DrawTextAuto(u8"设置", (Vector2){cr.x + 50, tr.y}, 42, 1, STYLE->theme.textPrimary);

    Rectangle pr = UILayoutNext(&lay, -1, -1);
    DrawRectangleRounded(pr, 0.1f, 12, STYLE->theme.panelBg);
    UILayout sl = UIBeginLayout(pr, UI_DIR_VERTICAL, 20, 30);

    // 主题设置
    Rectangle s1 = UILayoutNext(&sl, -1, 120);
    DrawTextAuto(u8"主题设置", (Vector2){s1.x + 30, s1.y + 10}, 28, 1, STYLE->theme.textPrimary);
    if(UICheckbox(u8"深色模式", (Rectangle){s1.x+30, s1.y+55, 300, 40},
        &g_app.isDarkMode, STYLE, UI_STATE)) {
        AppState_SetDarkMode(g_app.isDarkMode);
    }
    DrawTextAuto(AppState_IsDarkMode() ? u8"当前：深色模式" : u8"当前：浅色模式",
        (Vector2){s1.x + 30, s1.y + 100}, 18, 1, STYLE->theme.textSecondary);

    // 关于
    Rectangle s2 = UILayoutNext(&sl, -1, 120);
    DrawTextAuto(u8"关于", (Vector2){s2.x + 30, s2.y + 10}, 28, 1, STYLE->theme.textPrimary);
    UIDrawTextRec(u8"背单词软件 v5.0.0\n基于 raylib 构建，支持中英文混合显示",
        (Rectangle){s2.x + 30, s2.y + 50, 600, 60}, 18, 1, true, STYLE->theme.textSecondary);
}
