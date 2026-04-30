// ============================================================================
// 树形菜单系统实现
// 功能：菜单节点的创建、双向连接、菜单栈（导航历史维护）
//
// 设计决策：
//   菜单系统使用"树 + 栈"数据结构：
//   - 树：MENU 节点通过 child/parent 指针构成树形结构，
//         根节点为主菜单，子节点为各功能页面
//   - 栈：MenuStack 记录用户访问历史，实现"返回"功能，
//         导航深度最多 10 层（MAX_MENU_LEVEL）
// ============================================================================

#include "tree_menu.h"  /* MENU / MenuStack / CreatMenuTreeNode 声明 */
#include <stdlib.h>     /* malloc / free */
#include <stdio.h>      /* printf 调试输出 */

/**
 * CreatMenuTreeNode - 创建新的菜单树节点
 *
 * 分配一个新的 MENU 结构体并初始化。
 * 初始化值：show=传入函数, fun=传入函数, childindex=0, parent=NULL,
 * 所有 child 指针为 NULL。
 *
 * @param fun  菜单操作函数指针（当前未使用，传 NULL）
 * @param show 菜单页面渲染函数指针（页面显示逻辑）
 * @return 新创建的菜单节点指针，失败返回 NULL
 */
MENU* CreatMenuTreeNode(void(*fun)(void), void (*show)(void)) {
    /* 分配内存，如果失败返回 NULL */
    MENU* p = (MENU*)malloc(sizeof(MENU));
    if (p == NULL) {
        return NULL;  /* 内存不足 */
    }

    /* 初始化节点字段 */
    p->fun = fun;              /* 操作函数 */
    p->show = show;            /* 显示函数 */
    p->childindex = 0;         /* 初始无子节点 */
    p->parent = NULL;          /* 初始无父节点 */

    /* 将所有子节点指针初始化为 NULL */
    for (int i = 0; i < MAX_CHILD_NUM; i++) {
        p->child[i] = NULL;
    }

    return p;
}

/**
 * ConnectMenuTree - 将子节点连接到父节点
 *
 * 建立父子双向链接：父节点指向子节点 + 子节点指向父节点。
 * 如果父节点的子节点数组已满（MAX_CHILD_NUM=8），返回失败。
 *
 * @param parentNode 父节点
 * @param childNode  子节点
 * @return 0 成功，-1 失败（参数无效或子节点已满）
 */
int ConnectMenuTree(MENU* parentNode, MENU* childNode) {
    /* 参数检查 */
    if (parentNode == NULL || childNode == NULL) {
        return -1;
    }
    /* 检查是否已满（单个父节点最多 8 个子节点） */
    if (parentNode->childindex >= MAX_CHILD_NUM) {
        return -1;
    }

    /* 建立双向链接 */
    parentNode->child[parentNode->childindex++] = childNode;  /* 父→子 */
    childNode->parent = parentNode;                            /* 子→父 */

    return 0;
}

/**
 * StackInit - 初始化菜单栈
 *
 * 将所有栈元素置为 NULL，栈顶设为 -1（空栈状态）。
 * @param MenuBack 要初始化的栈指针
 */
void StackInit(MenuStack* MenuBack) {
    if (MenuBack == NULL) return;

    /* 清空所有元素 */
    for (int i = 0; i < MAX_MENU_LEVEL; i++) {
        MenuBack->menuStack[i] = NULL;
    }
    MenuBack->Stacktop = -1;  /* -1 表示空栈 */
}

/**
 * StackPush - 将菜单压入栈中
 *
 * 记录用户访问历史。当用户点击进入某个页面时，
 * 将当前页面压入栈，然后切换到目标页面。
 * 后续"返回"操作时从栈中弹出，回到之前的位置。
 *
 * @param MenuBack    目标栈
 * @param MenuCurrent 要压入的菜单节点
 */
void StackPush(MenuStack* MenuBack, MENU* MenuCurrent) {
    if (MenuBack == NULL || MenuCurrent == NULL) {
        printf("错误: 空指针\n");
        return;
    }
    if (MenuBack->Stacktop >= MAX_MENU_LEVEL - 1) {
        printf("错误: 栈已满\n");
        return;
    }

    /* 压入栈顶 */
    MenuBack->Stacktop++;
    MenuBack->menuStack[MenuBack->Stacktop] = MenuCurrent;
}

/**
 * StackPop - 从栈中弹出菜单
 *
 * 返回上一个页面。弹出后栈顶下移。
 * 如果栈为空（Stacktop == -1），返回 NULL。
 *
 * @param MenuBack 目标栈
 * @return 弹出的菜单节点，栈空返回 NULL
 */
MENU* StackPop(MenuStack* MenuBack) {
    if (MenuBack == NULL) {
        printf("错误: 空指针\n");
        return NULL;
    }
    if (MenuBack->Stacktop == -1) {
        printf("错误: 栈为空\n");
        return NULL;
    }

    /* 取出栈顶元素 */
    MENU* temp = MenuBack->menuStack[MenuBack->Stacktop];
    MenuBack->menuStack[MenuBack->Stacktop] = NULL;  /* 清空当前位置 */
    MenuBack->Stacktop--;                             /* 栈顶下移 */

    return temp;
}
