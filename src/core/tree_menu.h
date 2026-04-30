// ============================================================================
// 树形菜单系统头文件
// 功能：定义菜单树节点结构（MENU）、菜单栈结构（MenuStack）、相关操作函数
//
// 设计说明：
//   MENU 是一个双向树节点：通过 child[i] 数组指向子节点，
//   通过 parent 指针指向父节点。支持最多 8 个子节点和 10 层深度。
//   MenuStack 是一个固定深度的栈（最大 10 层），用于记录用户导航历史。
//   每次进入子页面时 StackPush，点击"返回"时 StackPop。
// ============================================================================

#ifndef MAIN_C_TREE_MENU_H
#define MAIN_C_TREE_MENU_H

#include "raylib.h"  /* Rectangle 类型，用于 DrawTreeMenu 的参数 */

// ============================================================================
// 常量定义
// ============================================================================

#define MAX_CHILD_NUM  8   /* 每个菜单节点最多可以连接的子节点数量 */
#define MAX_MENU_LEVEL 10  /* 菜单导航栈的最大深度（防止无限嵌套） */

// ============================================================================
// 数据结构
// ============================================================================

/**
 * MENU - 菜单节点结构体
 *
 * 树形菜单的核心数据结构。每个节点代表一个页面或菜单项。
 * 通过 child[] 数组组织子页面层次结构，parent 指针指向父页面。
 * show 函数指针指向页面渲染函数，fun 函数指针保留供未来扩展。
 */
typedef struct menu {
    /* show: 页面渲染函数指针。当菜单项被选中时调用此函数绘制页面内容。
     * 所有页面函数遵循 void func(void) 签名。
     * 例如 MenuHome_Show、MenuLearn_Show 等。 */
    void (*show)(void);

    /* fun: 页面操作函数指针（当前未使用，保留供未来扩展）。
     * 如果后续需要页面级别的键盘快捷键或定时器事件，
     * 可以在这里注册操作函数。 */
    void (*fun)(void);

    /* child[8]: 子节点指针数组。通过 ConnectMenuTree() 添加子节点。
     * childindex 跟踪当前已连接的子节点数量。 */
    struct menu* child[MAX_CHILD_NUM];

    /* parent: 父节点指针。用于从子页面回溯到父页面。 */
    struct menu* parent;

    int currentindex;   /* 当前节点在父节点 child 数组中的索引 */
    int childindex;     /* 已连接的子节点数量（0 ~ MAX_CHILD_NUM） */
} MENU;

/**
 * MenuStack - 菜单导航栈
 *
 * 用于实现"返回"功能的栈结构。
 * 用户每次进入子页面时，当前页面被压入栈中。
 * 点击"返回"时，从栈中弹出上一个页面。
 * 栈的最大深度为 MAX_MENU_LEVEL（10 层）。
 */
typedef struct MenuStack {
    MENU* menuStack[MAX_MENU_LEVEL];  /* 栈数组：存储菜单节点指针 */
    int Stacktop;                      /* 栈顶索引：-1 = 空栈 */
} MenuStack;

// ============================================================================
// 函数声明
// ============================================================================

/**
 * CreatMenuTreeNode - 创建菜单树节点
 * @param fun  操作函数指针（可为 NULL，当前未使用）
 * @param show 页面渲染函数指针
 * @return 新创建的节点，失败返回 NULL
 */
MENU* CreatMenuTreeNode(void (*fun)(void), void (*show)(void));

/**
 * ConnectMenuTree - 连接父子菜单节点
 * @param parentNode 父节点
 * @param childNode  子节点
 * @return 0 成功，-1 失败
 */
int ConnectMenuTree(MENU* parentNode, MENU* childNode);

/**
 * StackInit - 初始化菜单栈
 * @param MenuBack 要初始化的栈
 */
void StackInit(MenuStack* MenuBack);

/**
 * StackPush - 压入菜单到栈顶（记录访问历史）
 * @param MenuBack    目标栈
 * @param MenuCurrent 要压入的菜单
 */
void StackPush(MenuStack* MenuBack, MENU* MenuCurrent);

/**
 * StackPop - 从栈顶弹出菜单（返回上一个页面）
 * @param MenuBack 目标栈
 * @return 弹出的菜单节点，空栈返回 NULL
 */
MENU* StackPop(MenuStack* MenuBack);

#endif //MAIN_C_TREE_MENU_H
