// ============================================================================
// 树形菜单系统头文件
// 功能：定义菜单树节点结构和菜单导航功能
// ============================================================================

#ifndef MAIN_C_TREE_MENU_H
#define MAIN_C_TREE_MENU_H

// ============================================================================
// 常量定义
// ============================================================================

#define MAX_CHILD_NUM  5   // 每个菜单节点最多可以有多少个子菜单
#define MAX_MENU_LEVEL 10  // 菜单系统最大支持层级深度

// ============================================================================
// 数据结构
// ============================================================================

// 菜单节点结构
// 每个节点代表一个页面或菜单项
typedef struct menu {
    void (*show)();                        // 菜单页面的显示函数（渲染页面内容）
    void(*fun)();                          // 菜单页面的操作函数（处理交互）
    struct menu* child[MAX_CHILD_NUM];     // 子菜单指针数组
    struct menu* parent;                    // 父菜单指针
    int currentindex;                      // 当前菜单页面在父节点中的索引
    int childindex;                        // 已连接的子节点数量
} MENU;

// 菜单栈结构
// 用于实现"返回"功能，记录菜单访问历史
typedef struct MenuStack {
    MENU* menuStack[MAX_MENU_LEVEL];  // 菜单指针数组
    int Stacktop;                      // 栈顶索引（-1表示空栈）
} MenuStack;

// ============================================================================
// 函数声明
// ============================================================================

/**
 * 创建菜单树节点
 * @param fun 菜单操作函数（可为NULL）
 * @param show 菜单显示函数（页面渲染函数）
 * @return 新创建的菜单节点
 */
MENU* CreatMenuTreeNode(void(*fun)(), void (*show)());

/**
 * 连接两个菜单节点（将子节点添加到父节点）
 * @param parentNode 父节点
 * @param childNode 子节点
 * @return 0成功，-1失败
 */
int ConnectMenuTree(MENU* parentNode, MENU* childNode);

/**
 * 初始化菜单栈
 * @param MenuBack 要初始化的栈
 */
void StackInit(MenuStack* MenuBack);

/**
 * 将菜单压入栈中
 * @param MenuBack 目标栈
 * @param MenuCurrent 要压入的菜单
 */
void StackPush(MenuStack* MenuBack, MENU* MenuCurrent);

/**
 * 从栈中弹出菜单
 * @param MenuBack 目标栈
 * @return 弹出的菜单节点
 */
MENU* StackPop(MenuStack* MenuBack);

#endif //MAIN_C_TREE_MENU_H
