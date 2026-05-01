// 树形菜单系统 — 组合模式组织页面导航, 栈记录访问历史

#ifndef MAIN_C_TREE_MENU_H
#define MAIN_C_TREE_MENU_H

#include "raylib.h"

#define MAX_CHILD_NUM  8   // 每个节点最多子节点数
#define MAX_MENU_LEVEL 10  // 导航栈最大深度

// 菜单节点
typedef struct menu {
    void (*show)(void);              // 页面渲染函数
    void (*fun)(void);               // 预留操作函数
    struct menu* child[MAX_CHILD_NUM];
    struct menu* parent;
    int currentindex;                // 在父节点 child[] 中的索引
    int childindex;                  // 已连接的子节点数量
} MENU;

// 导航栈
typedef struct MenuStack {
    MENU* menuStack[MAX_MENU_LEVEL];
    int Stacktop;
} MenuStack;

MENU* CreatMenuTreeNode(void(*fun)(void), void(*show)(void));
int ConnectMenuTree(MENU* parentNode, MENU* childNode);
void StackInit(MenuStack* MenuBack);
void StackPush(MenuStack* MenuBack, MENU* MenuCurrent);
MENU* StackPop(MenuStack* MenuBack);

#endif
