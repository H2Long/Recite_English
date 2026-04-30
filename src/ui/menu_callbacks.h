// ============================================================================
// 菜单回调函数头文件
// 功能：声明菜单系统辅助函数（InitMenuTree/DrawTreeMenu/GetMenuItemText）
//       页面函数声明统一放在 pages/pages.h 中
// ============================================================================

#ifndef MENU_CALLBACKS_H
#define MENU_CALLBACKS_H

/* 引入所有页面函数的声明和公共访问宏 */
#include "pages/pages.h"

// ============================================================================
// 菜单系统辅助函数
// 这些函数负责菜单树的创建、绘制和导航逻辑。
// 在 main.c 的主循环中调用，不属于任何具体页面。
// ============================================================================

/**
 * InitMenuTree - 初始化菜单树
 *
 * 创建所有 14 个页面节点并建立父子关系。
 * 树形结构：
 *   主菜单 ├── 学单词
 *         ├── 背单词 ├── 卡片背单词
 *         │          ├── 选词背单词
 *         │          └── 测试
 *         ├── 查找单词
 *         ├── 学习计划 ├── 计划管理
 *         │            └── 学习进度
 *         ├── 设置
 *         ├── 账号管理
 *         └── 词库管理
 *
 * 根节点（MenuHome_Show）通过 g_app.rootMenu 引用。
 * 账号节点通过 g_accountMenuNode 导出供 main.c 使用。
 */
void InitMenuTree(void);

/**
 * FreeMenuTree - 释放菜单树内存
 * 遍历根菜单的子节点，释放所有 malloc 分配的 MENU 节点。
 */
void FreeMenuTree(void);

/**
 * GetMenuItemText - 获取菜单项的中文显示名称
 * 根据菜单节点的 show 函数指针判断对应的页面名称。
 * @param menu 菜单节点
 * @return 中文名称字符串（如"学单词"、"背单词"等）
 */
const char* GetMenuItemText(MENU* menu);

/**
 * DrawTreeMenu - 绘制左侧树形导航菜单
 *
 * 在指定的矩形区域内绘制侧边导航栏。
 * - 非首页：显示当前页面标题 + "返回"按钮 + 子菜单列表（可滚动）
 * - 首页：显示"主菜单"标题 + 子菜单列表
 *
 * @param menuRect 菜单区域（位置和大小）
 */
void DrawTreeMenu(Rectangle menuRect);

/* g_accountMenuNode: 账号菜单节点引用，在 InitMenuTree() 中赋值。
 * 通过 extern 导出，供 main.c 在右上角用户信息点击时跳转使用。 */
extern MENU* g_accountMenuNode;

#endif // MENU_CALLBACKS_H
