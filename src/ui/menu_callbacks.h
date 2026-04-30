// ============================================================================
// 菜单回调函数头文件
// 功能：声明菜单系统辅助函数，页面函数统一放在 pages/pages.h 中
// ============================================================================

#ifndef MENU_CALLBACKS_H
#define MENU_CALLBACKS_H

#include "pages/pages.h"

// ============================================================================
// 菜单系统辅助函数
// ============================================================================

/**
 * 初始化菜单树
 * 创建所有菜单节点并建立父子关系
 */
void InitMenuTree(void);

/**
 * 释放菜单树内存
 */
void FreeMenuTree(void);

/**
 * 获取菜单项文本
 * 根据菜单的显示函数确定菜单名称
 * @param menu 菜单节点
 * @return 菜单显示名称
 */
const char* GetMenuItemText(MENU* menu);

/**
 * 绘制树形菜单（左侧导航栏）
 * @param menuRect 菜单区域
 */
void DrawTreeMenu(Rectangle menuRect);

// 账号菜单节点（供 main.c 右上角点击跳转使用）
extern MENU* g_accountMenuNode;

#endif // MENU_CALLBACKS_H
