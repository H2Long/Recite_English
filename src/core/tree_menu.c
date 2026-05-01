// 树形菜单系统 — 实现

#include "tree_menu.h"
#include <stdlib.h>
#include <stdio.h>

MENU* CreatMenuTreeNode(void(*fun)(void), void(*show)(void))
{
    MENU* p = (MENU*)malloc(sizeof(MENU));               //分配空间
    if(p == NULL) {
        return NULL;
    }
    p->fun = fun;
    p->show = show;
    p->childindex = 0;                                  //索引初始化为0
    p->parent = NULL ;
    for (int i = 0; i < MAX_CHILD_NUM; i++) {
        p->child[i] = NULL;                              //指针指向NULL
    }
    return p;
}

int ConnectMenuTree(MENU*parentNode, MENU*childNode)
{
    if(parentNode == NULL || childNode == NULL || parentNode->childindex >= MAX_CHILD_NUM) {
        return -1;                           //失败返回-1
    }
    parentNode->child[parentNode->childindex++] = childNode;   //连接孩子节点
    childNode->parent = parentNode;                             //连接父母节点
    return 0;                               //连接成功返回0
}

void StackInit(MenuStack* MenuBack)
{
    if(MenuBack == NULL) {
        return ;
    }
    for (int i = 0; i < MAX_MENU_LEVEL; i++) {
        MenuBack->menuStack[i] = NULL;                     //栈内指针全部赋为0
    }
    MenuBack->Stacktop = -1;                               //栈顶赋为-1 表示栈空
}

void StackPush(MenuStack* MenuBack, MENU* MenuCurrent)
{
    if(MenuBack == NULL || MenuCurrent == NULL) {
        printf("空指针");
        return ;
    }
    if(MenuBack->Stacktop >= MAX_MENU_LEVEL - 1) {
        printf("栈满");
        return ;
    }
    MenuBack->Stacktop++;
    MenuBack->menuStack[MenuBack->Stacktop] = MenuCurrent;  //压入栈顶
}

MENU* StackPop(MenuStack* MenuBack)
{
    if(MenuBack == NULL) {
        printf("空指针");
        return NULL;
    }
    if(MenuBack->Stacktop == -1) {
        printf("栈空");
        return NULL;
    }
    MENU* temp = MenuBack->menuStack[MenuBack->Stacktop];  //   作为函数返回值
    MenuBack->menuStack[MenuBack->Stacktop] = NULL;        //   删除前一个菜单的记录
    MenuBack->Stacktop--;                                  //   栈顶-1
    return temp;
}
