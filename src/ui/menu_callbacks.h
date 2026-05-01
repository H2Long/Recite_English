// 菜单回调 — 树构建/导航栏绘制/名称获取

#ifndef MENU_CALLBACKS_H
#define MENU_CALLBACKS_H

#include "pages/pages.h"

void InitMenuTree(void);
void FreeMenuTree(void);
const char* GetMenuItemText(MENU* menu);
void DrawTreeMenu(Rectangle menuRect);

extern MENU* g_accountMenuNode;

#endif
