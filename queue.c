/**
 *  双向队列
 **/
#include "queue.h"

void Queue_Init(Queue *queue)
{
    List_Init(&queue->head);
    queue->length = 0;
}

int Queue_ISEmpty(Queue *queue)
{
    return List_IsEmpty((List *)&queue->head);
}

int Queue_IsContained(Queue *queue, QueueNode *node)
{
    ListNode *pos = NULL;
    int ret = 0;
    List_ForEach((List *)&queue->head, pos)
    {
        if (ret = isEqual(pos, node))
        {
            break;
        }
    }
    return ret;
}

void Queue_Add(Queue *queue, QueueNode *node)
{
    List_AddTail((List *)&queue->head, node);
    queue->length++;
}

QueueNode *Queue_Front(Queue *queue)
{
    QueueNode *ret = NULL;

    if (queue->length > 0)
    {
        ret = queue->head.next;
    }

    return ret;
}

QueueNode *Queue_Remove(Queue *queue)
{
    QueueNode *ret = Queue_Front(queue);

    if (ret)
    {
        List_DelNode(ret);
        queue->length--;
    }

    return ret;
}

int Queue_Length(Queue *queue)
{
    return queue->length;
}

void Queue_Rotate(Queue *queue)
{
    QueueNode *node = Queue_Remove(queue);
    if (node)
    {
        Queue_Add(queue, node);
    }
}
