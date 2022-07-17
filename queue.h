#ifndef QUEUE_H
#define QUEUE_H

#include "list.h"
typedef ListNode QueueNode;

typedef struct{
    QueueNode head;
    int length;
}Queue;

#define Queue_Node(ptr, type, member) ContainerOf(ptr, type, member)

/* 队列初始化 */
void Queue_Init(Queue* queue);

/* 队列是否为空 */
int Queue_ISEmpty(Queue* queue);

/* 队列是否包含某节点 */
int Queue_IsContained(Queue* queue, QueueNode* node);

/* 添加节点 */
void Queue_Add(Queue* queue, QueueNode* node);

/* 获取队列首节点 */
QueueNode* Queue_Front(Queue* queue);

/* 移除队列首节点 */
QueueNode* Queue_Remove(Queue* queue);

/* 获取队列长度 */
int Queue_Length(Queue* queue);

/* 将队列头部节点移到队尾 */
void Queue_Rotate(Queue* queue);

#endif  //QUEUE_H