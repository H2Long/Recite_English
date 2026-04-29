// ============================================================================
// 树形菜单系统实现
// 功能：菜单节点的创建、连接和菜单栈操作
// ============================================================================

#include "tree_menu.h"
#include <stdlib.h>
#include <stdio.h>

// ============================================================================
// 菜单节点操作
// ============================================================================

/**
 * 创建新的菜单树节点
 * @param fun 菜单操作函数（可为NULL）
 * @param show 菜单显示函数
 * @return 新创建的菜单节点，失败返回NULL
 */
MENU* CreatMenuTreeNode(void(*fun)(), void (*show)()) {
    // 分配内存空间
    MENU* p = (MENU*)malloc(sizeof(MENU));
    if (p == NULL) {
        return NULL;
    }

    // 初始化节点属性
    p->fun = fun;                    // 设置操作函数
    p->show = show;                  // 设置显示函数
    p->childindex = 0;                // 初始没有子节点
    p->parent = NULL;                 // 初始没有父节点

    // 将所有子节点指针初始化为NULL
    for (int i = 0; i < MAX_CHILD_NUM; i++) {
        p->child[i] = NULL;
    }

    return p;
}

/**
 * 将子节点连接到父节点
 * @param parentNode 父节点
 * @param childNode 子节点
 * @return 0成功，-1失败
 */
int ConnectMenuTree(MENU* parentNode, MENU* childNode) {
    // 参数检查
    if (parentNode == NULL || childNode == NULL) {
        return -1;
    }
    // 检查是否已满
    if (parentNode->childindex >= MAX_CHILD_NUM) {
        return -1;
    }

    // 建立双向连接
    parentNode->child[parentNode->childindex++] = childNode;  // 父节点指向子节点
    childNode->parent = parentNode;                            // 子节点指向父节点

    return 0;
}

// ============================================================================
// 菜单栈操作（用于"返回"功能）
// ============================================================================

/**
 * 初始化菜单栈
 * @param MenuBack 要初始化的栈指针
 */
void StackInit(MenuStack* MenuBack) {
    if (MenuBack == NULL) {
        return;
    }

    // 将所有栈元素置为空
    for (int i = 0; i < MAX_MENU_LEVEL; i++) {
        MenuBack->menuStack[i] = NULL;
    }
    // 栈顶设为-1表示空栈
    MenuBack->Stacktop = -1;
}

/**
 * 将菜单压入栈中（记录访问历史）
 * @param MenuBack 目标栈
 * @param MenuCurrent 要压入的菜单
 */
void StackPush(MenuStack* MenuBack, MENU* MenuCurrent) {
    // 参数检查
    if (MenuBack == NULL || MenuCurrent == NULL) {
        printf("错误: 空指针\n");
        return;
    }

    // 检查栈是否已满
    if (MenuBack->Stacktop >= MAX_MENU_LEVEL - 1) {
        printf("错误: 栈已满\n");
        return;
    }

    // 将菜单压入栈顶
    MenuBack->Stacktop++;
    MenuBack->menuStack[MenuBack->Stacktop] = MenuCurrent;
}

/**
 * 从栈中弹出菜单（用于返回上一个页面）
 * @param MenuBack 目标栈
 * @return 弹出的菜单节点，如果栈空返回NULL
 */
MENU* StackPop(MenuStack* MenuBack) {
    // 参数检查
    if (MenuBack == NULL) {
        printf("错误: 空指针\n");
        return NULL;
    }

    // 检查栈是否为空
    if (MenuBack->Stacktop == -1) {
        printf("错误: 栈为空\n");
        return NULL;
    }

    // 取出栈顶元素
    MENU* temp = MenuBack->menuStack[MenuBack->Stacktop];
    MenuBack->menuStack[MenuBack->Stacktop] = NULL;  // 清空当前位置
    MenuBack->Stacktop--;                             // 栈顶下移

    return temp;
}
