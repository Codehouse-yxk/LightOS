/*
 * @Author: yangxingkun
 * @Date: 2022-07-16 15:22:30
 * @FilePath: \LightOS\list.c
 * @Description: 双向循环链表
 * @Github: https://github.com/Codehouse-yxk
 */

#include "list.h"

void List_Init(List *list)
{
    list->next = list;
    list->prev = list;
}

static void _List_Add(ListNode *node, ListNode *prev, ListNode *next)
{
    node->next = next;
    next->prev = node;
    node->prev = prev;
    prev->next = node;
}

static void _List_Del(ListNode *prev, ListNode *next)
{
    prev->next = next;
    next->prev = prev;
}

void List_Add(List *list, ListNode *node)
{
    _List_Add(node, list, list->next);
}

void List_AddTail(List *list, ListNode *node)
{
    _List_Add(node, list->prev, list);
}

void List_AddBefore(ListNode *before, ListNode *node)
{
    _List_Add(node, before->prev, before);
}

void List_AddAfter(ListNode *after, ListNode *node)
{
    _List_Add(node, after, after->next);
}

void List_DelNode(ListNode *node)
{
    _List_Del(node->prev, node->next);
    node->next = NULL;
    node->prev = NULL;
}

void ListReplace(ListNode *old, ListNode *node)
{
    node->next = old->next;
    node->next->prev = node;

    node->prev = old->prev;
    node->prev->next = node;

    old->prev = NULL;
    old->next = NULL;
}

int List_IsLast(List *list, ListNode *node)
{
    return (isEqual(node->next, list));
}

int List_IsEmpty(List *list)
{
    return (isEqual(list->next, list) && isEqual(list->prev, list));
}