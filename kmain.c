
/**
 *  内核入口（c）
 */

#include "screen.h"
#include "task.h"
#include "interrupt.h"
#include "queue.h"

typedef struct _test
{
    int val;
    QueueNode head;
} test;

Queue g_queue;

test g_arr[5];

/**
 * 内核入口函数
 */
void KMain()
{
    SetPrintColor(SCREEN_RED);

    PrintString("LightOS\n\n");

    PrintString("Gdt Entry: ");
    PrintIntHex((uint)gGdtInfo.entry);
    PrintChar('\n');

    PrintString("Gdt Size: ");
    PrintIntDec(gGdtInfo.size);
    PrintChar('\n');

    PrintString("Idt Entry: ");
    PrintIntHex((uint)gIdtInfo.entry);
    PrintChar('\n');

    PrintString("Idt Size: ");
    PrintIntDec(gIdtInfo.size);
    PrintChar('\n');

    // IntModInit();

    // TaskModInit();

    // LaunchTask();

    Queue_Init(&g_queue);

    int i = 0;
    for (i = 0; i < 5; i++)
    {
        g_arr[i].val = i;
        Queue_Add(&g_queue, &((test *)AddrOff(g_arr, i))->head);
    }
    PrintIntDec(Queue_Length(&g_queue));
    PrintChar('\n');
    Queue_Rotate(&g_queue);
    int pp = Queue_IsContained(&g_queue, &((test *)AddrOff(g_arr, 2))->head);
    PrintIntDec(pp);
    PrintChar('\n');

    PrintIntDec(Queue_ISEmpty(&g_queue));
    PrintChar('\n');
    PrintString("print queue");
    while (Queue_Length(&g_queue) > 0)
    {
        PrintChar('\n');
        QueueNode *node = Queue_Remove(&g_queue);
        int v = Queue_Node(node, test, head)->val;
        PrintIntDec(v);
    }
    PrintChar('\n');
    PrintIntDec(Queue_ISEmpty(&g_queue));
}
