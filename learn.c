//
// Created by hhlong on 2026/5/2.
//
//
// Created by hhlong on 2026/3/18.
//

/*#include "LinkList.h"


Node* LinkList_init() {
    Node * temp = (Node*)malloc(sizeof(Node));
    if (temp == NULL) {
        printf("fails to creat node");
    }
    else {
        temp->data = 0;
        temp->next = NULL;
    }
    return temp;
}

//头插法（每一次插入都在头节点后面）
result_t LinkList_insertHead(Node* head,ElemType data) {
    if(head == NULL) {
        return FAIL;
    }
    else {
        Node* temp =  (Node*)malloc(sizeof(Node));
        temp->data = data;

        temp->next = head->next;
        head->next = temp;
        return OK;
    }
}
//遍历
result_t LinkList_traverse(Node*head) {
    if(head == NULL) {
        return FAIL;
    }
    else {
        Node* temp = head->next;
        while (temp != NULL) {
            printf("%d",temp->data);
            temp = temp->next;
        }
        return OK;
    }
}
//得到长度函数
int LinkList_getLength(Node*head) {
    if(head == NULL) {
        printf("空指针");
        return 0;
    }
    else {
        int length = 0;
        Node* temp = head->next;
        while (temp != NULL) {
            length++;
            temp = temp->next;
        }
        return length;
    }
}
//辅助函数:找到特定位置的前驱,头节点算第0个数据
Node* LinkList_findFormer(Node*head,int pos) {
    if(head == NULL) {
        return NULL;
    }
    else {
        int length = LinkList_getLength(head);
        if (pos <= 0 || pos > length) {
            return NULL;
        }
        Node * temp = head;
        for (int i = 0 ; i < pos - 1;i++) {
            temp = temp->next;
        }
        return temp;
    }
}

//
//尾插法
result_t LinkList_insertTail(Node*head ,ElemType data) {
    if(head == NULL) {
        return FAIL;
    }
    else {
        int length = LinkList_getLength(head);
        if (length == 0) {

        }
        Node* tail = LinkList_findFormer(head,length)->next;
        Node* temp =  (Node*)malloc(sizeof(Node));
        temp->data = data;
        temp->next = NULL;
        tail->next = temp;
        return OK;
    }
}
result_t LinkList_insert(Node*head ,ElemType data,int pos) {
    if(head == NULL) {
        return FAIL;
    }
    else if (pos <= 0 || pos > LinkList_getLength(head)) {
        return FAIL;
    }
    else {
        //得到pos位置的前驱
        Node* former = LinkList_findFormer(head,pos);
        Node* temp =  (Node*)malloc(sizeof(Node));
        temp->data = data;
        temp->next = former->next;
        former->next = temp;
        return OK;
    }
}
result_t LinkList_delete(Node*head ,int pos) {
    if(head == NULL) {
        return FAIL;
    }
    else if (pos <= 0 || pos > LinkList_getLength(head)) {
        return FAIL;
    }
    else {
        //得到pos位置的前驱
        Node* former = LinkList_findFormer(head,pos);
        Node* temp = former->next;
        former->next = temp->next;
        free(temp);
        return OK;
    }
}
//
//
//
////
// Created by hhlong on 2026/3/13.
//
#include "SeqList.h"
#include "stdio.h"
#include <stddef.h>
#include <stdint.h>
#include  <stdlib.h>
result_t SeqList_Init(SeqList* list) {
    if (list == NULL) {
        return FAIL;
    }
    else {
        list->length = 0;
        return OK;
    }
}

result_t SeqList_Add(SeqList* list, ElemType data) {
    if (list == NULL) {
        return FAIL;
    }
    else if (list->length >= MAX_SIZE ) {
        return FAIL;
    }
    else {
        list->elem[list->length] = data;
        list->length++;
        return OK;
    }
}

result_t SeqList_Get(SeqList* list, ElemType *data,int pos) {
    if (list == NULL) {
        return FAIL;
    }
    else if ( pos < 1 || pos > list->length  ){
        return FAIL;
    }
    else {
        *data = list->elem[pos-1];
        return OK;
    }
}

result_t SeqList_Remove(SeqList* list,ElemType *data,int pos ) {
    if (list == NULL) {
        return FAIL;
    }
    else if (list->length <= 0) {
        return FAIL;
    }
    else if (pos < 1 || pos > list->length  ) {
        return FAIL;
    }
    else {
        *data = list->elem[pos-1];
        for (int i = pos - 1; i < list->length - 1;i++) {
            list->elem[i] = list->elem[i + 1];
        }
        list->length --;
        return OK;
    }
}

result_t SeqList_Change(SeqList* list ,ElemType data ,int pos) {
   if (list == NULL) {
        return FAIL;
    }
   else if (list->length <= 0) {
       return FAIL;
   }
    else if (pos < 1 || pos >list->length ) {
        return FAIL;
    }
    else {
        list->elem[pos - 1] = data;
        return OK;
    }
}

result_t SeqList_Insert(SeqList* list, ElemType data ,int pos) {
    if (list == NULL) {
        return FAIL;
    }
    //可以插入到最后一个位置
    else if (pos < 1 || pos >list->length+1 ) {
        return FAIL;
    }
    else if (list->length >= MAX_SIZE ){
        return FAIL;
    }
    else {
        //从最后一位开始移动
        for (int i = list->length  ;i > pos - 1;i--) {
             list->elem[i] = list->elem[i - 1];
        }
       list->length++;
        list->elem[pos - 1] = data;
        return OK;
    }
}
result_t SeqList_Find(SeqList* list , ElemType data) {
    uint8_t flag = 0;
    if (list == NULL) {
        return FAIL;
    }
    else if (list->length <= 0) {
        return FAIL;
    }
   else {
       for (int i = 0;i < list->length;i++) {
           if (list->elem[i] == data) {
               flag = 1;
               printf("Its position : %d",i+1);
               return OK;
           }
       }
       if (flag == 0) {
           printf("Not found");
       }
   }
}

result_t SeqList_Clear(SeqList* list) {
    if (list == NULL) {
        return FAIL;
    }
    else if (list->length <= 0) {
        return FAIL;
    }
    else {
        list->length = 0;
        return OK;
    }
}

/***********************************  devision line  *************************************************/
/*
Seq_List* Seq_List_Init() {
    Seq_List * temp = (Seq_List*)malloc(sizeof(Seq_List));
    if (temp == NULL) {
        return NULL;
    }
    else {
        temp->elem = (ElemType*)malloc(sizeof(ElemType) * MAX_SIZE);
        temp->length = 0;
        return temp;
    }
}
result_t Seq_List_Free(Seq_List* list) {
    if (list == NULL) {
        return FAIL;
    }
    else {
        free(list->elem);
        free(list);
        return OK;
    }
}




MENU* CreatMenuTreeNode(void(*fun)(),void (*show)())
{
    MENU* p = (MENU*)malloc(sizeof(MENU));               //分配空间
    p->fun = fun;
    p->show = show;
    p->childindex = 0;                                  //索引初始化为0；
    p->parent = NULL ;
    for (int i = 0; i < MAX_CHILD_NUM; i++)
    {
        p->child[i] = NULL;                              //指针指向NULL
    }
    return p;
}


int ConnectMenuTree(MENU*parentNode,MENU*childNode)
{
    if (parentNode == NULL || childNode == NULL|| parentNode->childindex >= MAX_CHILD_NUM)
    {
        return -1;                           //失败返回-1
    }
    parentNode->child[ parentNode->childindex++] = childNode;   //连接孩子节点
    childNode->parent = parentNode;                             //连接父母节点
    return 0;                               //连接成功返回0
}
void StackInit(MenuStack* MenuBack)
{
    if (MenuBack == NULL)
    {
        return ;
    }
    for (int i = 0; i < MAX_MENU_LEVEL; i++)
    {
        MenuBack->menuStack[i] = NULL;                     //栈内指针全部赋为0
    }
    MenuBack->Stacktop = -1;                               //栈顶赋为-1  表示栈空
}
​
​
MENU* StackPop(MenuStack* MenuBack)
{
    if (MenuBack == NULL)
    {
        printf("空指针");
        return NULL;
    }
    if (MenuBack->Stacktop == -1)
    {
        printf("栈空");
        return NULL;
    }
    MENU* temp = MenuBack->menuStack[MenuBack->Stacktop];  //   作为函数返回值
    MenuBack->menuStack[MenuBack->Stacktop] = NULL;        //   删除前一个菜单的记录
    MenuBack->Stacktop--;                                  //   栈顶-1
    return temp;
}


*/

//
//
//
//
//