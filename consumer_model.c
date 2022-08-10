/*
 * @Author: yangxingkun
 * @Date: 2022-08-10 23:22:11
 * @FilePath: \LightOS\consumer_model.c
 * @Description: 生产消费者模型，测试mutex
 * @Github: https://github.com/Codehouse-yxk
 */

#include "consumer_model.h"
#include "list.h"
#include "syscall.h"
#include "memory.h"
#include "screen.h"

typedef struct 
{
    ListNode head;
    char val;
}Product;


static uint gMutex = 0;
static List gList = {0};
static uint gMaxNum = 40;
static uint producerCnt = 0;
static uint consumerCnt = 0;

void Store(const char v)
{
    Product* new = Malloc(sizeof(Product));

    if(new)
    {
        new->val = v;
        List_AddTail(&gList, (ListNode*)new);
    }
}

static uint Feach(const char c)
{
    uint ret = 0;
    ListNode* pos = NULL;
    List_ForEach(&gList, pos)
    {
        if(((Product*)pos)->val == c)
        {
            ret = 1;
            List_DelNode(pos);
            Free(pos);
            break;
        }
    }
    return ret;
}

void ProducerA()
{
    List_Init(&gList);
    gMutex = CreateMutex(STRICT);
    int flag = 1;
    SetPrintPos(0, 15);
    PrintString("Producer : ");
    while (flag)
    {
        EnterCritical(gMutex);
        if(producerCnt < gMaxNum)
        {
            Store('A');
            producerCnt++;
            SetPrintPos(15+producerCnt, 15);
            PrintChar('A');
        }
        else
        {
            flag = 0;
        }
        ExitCritical(gMutex);
        Delay(4);
    }
    SetPrintPos(0, 20);
    PrintString("Producer A exit \n");
}

void ProducerB()
{
    int flag = 1;
    while (flag)
    {
        EnterCritical(gMutex);
        if(producerCnt < gMaxNum)
        {
            Store('B');
            producerCnt++;
            SetPrintPos(15+producerCnt, 15);
            PrintChar('B');
        }
        else
        {
            flag = 0;
        }
        ExitCritical(gMutex);
        Delay(6);
    }
    SetPrintPos(0, 21);
    PrintString("Producer B exit \n");
}

void ConsumerA()
{
    SetPrintPos(0, 18);
    PrintString("Consumer : ");
    while(consumerCnt<gMaxNum)
    {
        EnterCritical(gMutex);
        if(Feach('A'))
        {
            consumerCnt++;
            SetPrintPos(15+consumerCnt, 18);
            PrintChar('A');
        }
        ExitCritical(gMutex);
        Delay(2);
    }
    SetPrintPos(0, 22);
    PrintString("Consumer A exit \n");
}

void ConsumerB()
{
    while(consumerCnt<gMaxNum)
    {
        EnterCritical(gMutex);
        if(Feach('B'))
        {
            consumerCnt++;
            SetPrintPos(15+consumerCnt, 18);
            PrintChar('B');
        }
        ExitCritical(gMutex);
        Delay(3);
    }
    SetPrintPos(0, 23);
    PrintString("Consumer B exit \n");
}