/*
 * @Author: yangxingkun
 * @Date: 2022-08-10 23:22:11
 * @FilePath: \LightOS\consumer_model.c
 * @Description: 生产消费者模型
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
static uint producerACnt = 0;
static uint producerBCnt = 0;
static uint consumerACnt = 0;
static uint consumerBCnt = 0;

static void Store(const char v)
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

static void ProducerA()
{
    int flag = 1;
    SetPrintPos(0, 15);
    PrintString("ProducerA: ");
    while (flag)
    {
        EnterCritical(gMutex);
        if(producerACnt < gMaxNum)
        {
            Store('A');
            producerACnt++;
            SetPrintPos(15+producerACnt, 15);
            PrintChar('A');
        }
        else
        {
            flag = 0;
        }
        ExitCritical(gMutex);
        Delay(1);
    }
    SetPrintPos(0, 20);
    PrintString("Producer A exit \n");
}

static void ProducerB()
{
    int flag = 1;
    SetPrintPos(0, 16);
    PrintString("ProducerB: ");
    while (flag)
    {
        EnterCritical(gMutex);
        if(producerBCnt < gMaxNum)
        {
            Store('B');
            producerBCnt++;
            SetPrintPos(15+producerBCnt, 16);
            PrintChar('B');
        }
        else
        {
            flag = 0;
        }
        ExitCritical(gMutex);
        Delay(2);
    }
    SetPrintPos(0, 21);
    PrintString("Producer B exit \n");
}

static void ConsumerA()
{
    SetPrintPos(0, 17);
    PrintString("ConsumerA: ");
    while(consumerACnt<gMaxNum)
    {
        EnterCritical(gMutex);
        if(Feach('A'))
        {
            consumerACnt++;
            SetPrintPos(15+consumerACnt, 17);
            PrintChar('A');
        }
        ExitCritical(gMutex);
        Delay(1);
    }
    SetPrintPos(0, 22);
    PrintString("Consumer A exit \n");
}

static void ConsumerB()
{    
    SetPrintPos(0, 18);
    PrintString("ConsumerB: ");
    while(consumerBCnt<gMaxNum)
    {
        EnterCritical(gMutex);
        if(Feach('B'))
        {
            consumerBCnt++;
            SetPrintPos(15+consumerBCnt, 18);
            PrintChar('B');
        }
        ExitCritical(gMutex);
        Delay(3);
    }
    SetPrintPos(0, 23);
    PrintString("Consumer B exit \n");
}

static void Initialize()
{
    SetPrintPos(0,12);
    PrintString("Consumer model task initialize");
    List_Init(&gList);
    gMutex = CreateMutex(STRICT);
}

static void DeInit()
{
    Wait("ProducerA");
    Wait("ProducerB");
    Wait("ConsumerA");
    Wait("ConsumerB");
    ListNode* pos = NULL;
    List_ForEach(&gList, pos)
    {
        List_DelNode(pos);
        Free(pos);
    }

    DestroyMutex(gMutex);

    SetPrintPos(0,13);
    PrintString("Consumer model task finish");
}

void RunConsumerModel()
{
    Initialize();

    RegApp("ProducerA", ProducerA, 255);
    RegApp("ProducerB", ProducerB, 255);
    RegApp("ConsumerA", ConsumerA, 255);
    RegApp("ConsumerB", ConsumerB, 255);

    RegApp("DeInit", DeInit, 1);
}