#ifndef LIST_H
#define LIST_H

#include "const.h"
#include "utility.h"

typedef struct _ListNode
{
    struct _ListNode *next;
    struct _ListNode *prev;
} ListNode;

typedef ListNode List;

#define List_ForEach(list, pos) for (pos = (list)->next; !isEqual(list, pos); pos = pos->next)

#define List_Node(ptr, type, member) ContainerOf(ptr, type, member)

/* 链表初始化 */
void List_Init(List *list);

/* 链表头部增加节点 */
void List_Add(List *list, ListNode *node);

/* 链表尾部增加节点 */
void List_AddTail(List *list, ListNode *node);

/* 在某个节点前插入新节点 */
void List_AddBefore(ListNode *before, ListNode *node);

/* 在某个节点后插入新节点 */
void List_AddAfter(ListNode *after, ListNode *node);

/* 删除一个节点 */
void List_DelNode(ListNode *node);

/* 新节点替换旧节点 */
void ListReplace(ListNode *old, ListNode *node);

/* 节点是否是链表最后一个节点 */
int List_IsLast(List *list, ListNode *node);

/* 链表是否为空 */
int List_IsEmpty(List *list);

#endif // LIST_H