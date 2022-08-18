/*
 * @Author: yangxingkun
 * @Date: 2022-08-14 21:08:33
 * @FilePath: \LightOS\keyboard.c
 * @Description: 键盘驱动
 * @Github: https://github.com/Codehouse-yxk
 */

#include "keyboard.h"
#include "screen.h"
#include "utility.h"
#include "task.h"

static Queue gWaitQueue = {0};

/**
 * @description: 按键映射表
 */
static const KeyCode gKeyMap[] = 
{
/* 0x00 - none      */ {  0,        0,        0,         0   },
/* 0x01 - ESC       */ {  0,        0,       0x01,      0x1B },
/* 0x02 - '1'       */ { '1',      '!',      0x02,      0x31 },
/* 0x03 - '2'       */ { '2',      '@',      0x03,      0x32 },
/* 0x04 - '3'       */ { '3',      '#',      0x04,      0x33 },
/* 0x05 - '4'       */ { '4',      '$',      0x05,      0x34 },
/* 0x06 - '5'       */ { '5',      '%',      0x06,      0x35 },
/* 0x07 - '6'       */ { '6',      '^',      0x07,      0x36 },
/* 0x08 - '7'       */ { '7',      '&',      0x08,      0x37 },
/* 0x09 - '8'       */ { '8',      '*',      0x09,      0x38 },
/* 0x0A - '9'       */ { '9',      '(',      0x0A,      0x39 },
/* 0x0B - '0'       */ { '0',      ')',      0x0B,      0x30 },
/* 0x0C - '-'       */ { '-',      '_',      0x0C,      0xBD },
/* 0x0D - '='       */ { '=',      '+',      0x0D,      0xBB },
/* 0x0E - BS        */ {  0,        0,       0x0E,      0x08 },
/* 0x0F - TAB       */ {  0,        0,       0x0F,      0x09 },
/* 0x10 - 'q'       */ { 'q',      'Q',      0x10,      0x51 },
/* 0x11 - 'w'       */ { 'w',      'W',      0x11,      0x57 },
/* 0x12 - 'e'       */ { 'e',      'E',      0x12,      0x45 },
/* 0x13 - 'r'       */ { 'r',      'R',      0x13,      0x52 },
/* 0x14 - 't'       */ { 't',      'T',      0x14,      0x54 },
/* 0x15 - 'y'       */ { 'y',      'Y',      0x15,      0x59 },
/* 0x16 - 'u'       */ { 'u',      'U',      0x16,      0x55 },
/* 0x17 - 'i'       */ { 'i',      'I',      0x17,      0x49 },
/* 0x18 - 'o'       */ { 'o',      'O',      0x18,      0x4F },
/* 0x19 - 'p'       */ { 'p',      'P',      0x19,      0x50 },
/* 0x1A - '['       */ { '[',      '{',      0x1A,      0xDB },
/* 0x1B - ']'       */ { ']',      '}',      0x1B,      0xDD },
/* 0x1C - CR/LF     */ {  0,        0,       0x1C,      0x0D },
/* 0x1D - l. Ctrl   */ {  0,        0,       0x1D,      0x11 },
/* 0x1E - 'a'       */ { 'a',      'A',      0x1E,      0x41 },
/* 0x1F - 's'       */ { 's',      'S',      0x1F,      0x53 },
/* 0x20 - 'd'       */ { 'd',      'D',      0x20,      0x44 },
/* 0x21 - 'f'       */ { 'f',      'F',      0x21,      0x46 },
/* 0x22 - 'g'       */ { 'g',      'G',      0x22,      0x47 },
/* 0x23 - 'h'       */ { 'h',      'H',      0x23,      0x48 },
/* 0x24 - 'j'       */ { 'j',      'J',      0x24,      0x4A },
/* 0x25 - 'k'       */ { 'k',      'K',      0x25,      0x4B },
/* 0x26 - 'l'       */ { 'l',      'L',      0x26,      0x4C },
/* 0x27 - ';'       */ { ';',      ':',      0x27,      0xBA },
/* 0x28 - '\''      */ { '\'',     '\"',     0x28,      0xDE },
/* 0x29 - '`'       */ { '`',      '~',      0x29,      0xC0 },
/* 0x2A - l. SHIFT  */ {  0,        0,       0x2A,      0x10 },
/* 0x2B - '\'       */ { '\\',     '|',      0x2B,      0xDC },
/* 0x2C - 'z'       */ { 'z',      'Z',      0x2C,      0x5A },
/* 0x2D - 'x'       */ { 'x',      'X',      0x2D,      0x58 },
/* 0x2E - 'c'       */ { 'c',      'C',      0x2E,      0x43 },
/* 0x2F - 'v'       */ { 'v',      'V',      0x2F,      0x56 },
/* 0x30 - 'b'       */ { 'b',      'B',      0x30,      0x42 },
/* 0x31 - 'n'       */ { 'n',      'N',      0x31,      0x4E },
/* 0x32 - 'm'       */ { 'm',      'M',      0x32,      0x4D },
/* 0x33 - ','       */ { ',',      '<',      0x33,      0xBC },
/* 0x34 - '.'       */ { '.',      '>',      0x34,      0xBE },
/* 0x35 - '/'       */ { '/',      '?',      0x35,      0xBF },
/* 0x36 - r. SHIFT  */ {  0,        0,       0x36,      0x10 },
/* 0x37 - '*'       */ { '*',      '*',      0x37,      0x6A },
/* 0x38 - ALT       */ {  0,        0,       0x38,      0x12 },
/* 0x39 - ' '       */ { ' ',      ' ',      0x39,      0x20 },
/* 0x3A - CapsLock  */ {  0,        0,       0x3A,      0x14 },
/* 0x3B - F1        */ {  0,        0,       0x3B,      0x70 },
/* 0x3C - F2        */ {  0,        0,       0x3C,      0x71 },
/* 0x3D - F3        */ {  0,        0,       0x3D,      0x72 },
/* 0x3E - F4        */ {  0,        0,       0x3E,      0x73 },
/* 0x3F - F5        */ {  0,        0,       0x3F,      0x74 },
/* 0x40 - F6        */ {  0,        0,       0x40,      0x75 },
/* 0x41 - F7        */ {  0,        0,       0x41,      0x76 },
/* 0x42 - F8        */ {  0,        0,       0x42,      0x77 },
/* 0x43 - F9        */ {  0,        0,       0x43,      0x78 },
/* 0x44 - F10       */ {  0,        0,       0x44,      0x79 },
/* 0x45 - NumLock   */ {  0,        0,       0x45,      0x90 },
/* 0x46 - ScrLock   */ {  0,        0,       0x46,      0x91 },
/* 0x47 - Home      */ {  0,        0,       0x47,      0x24 },
/* 0x48 - Up        */ {  0,        0,       0x48,      0x26 },
/* 0x49 - PgUp      */ {  0,        0,       0x49,      0x21 },
/* 0x4A - '-'       */ {  0,        0,       0x4A,      0x6D },
/* 0x4B - Left      */ {  0,        0,       0x4B,      0x25 },
/* 0x4C - MID       */ {  0,        0,       0x4C,      0x0C },
/* 0x4D - Right     */ {  0,        0,       0x4D,      0x27 },
/* 0x4E - '+'       */ {  0,        0,       0x4E,      0x6B },
/* 0x4F - End       */ {  0,        0,       0x4F,      0x23 },
/* 0x50 - Down      */ {  0,        0,       0x50,      0x28 },
/* 0x51 - PgDown    */ {  0,        0,       0x51,      0x22 },
/* 0x52 - Insert    */ {  0,        0,       0x52,      0x2D },
/* 0x53 - Del       */ {  0,        0,       0x53,      0x2E },
/* 0x54 - Enter     */ {  0,        0,       0x54,      0x0D },
/* 0x55 - ???       */ {  0,        0,        0,         0   },
/* 0x56 - ???       */ {  0,        0,        0,         0   },
/* 0x57 - F11       */ {  0,        0,       0x57,      0x7A },  
/* 0x58 - F12       */ {  0,        0,       0x58,      0x7B },
/* 0x59 - ???       */ {  0,        0,        0,         0   },
/* 0x5A - ???       */ {  0,        0,        0,         0   },
/* 0x5B - Left Win  */ {  0,        0,       0x5B,      0x5B },	
/* 0x5C - Right Win */ {  0,        0,       0x5C,      0x5C },
/* 0x5D - Apps      */ {  0,        0,       0x5D,      0x5D },
/* 0x5E - Pause     */ {  0,        0,       0x5E,      0x13 }
};

static keyCodeBuff gKCBuff = {0};

/**
 * @description: 检测按键类型
 * @param 扫描码
 * @return 按下：KeyPress， 松开：KeyRelease
 */
static uint GetKeyType(byte code)
{
    uint ret = KeyPress;

    if(code & 0x80)
    {
        ret = KeyRelease;
    }

    return ret;
}

/**
 * @description: 判断是否为shift键
 * @param 扫描码
 * @return 是：1， 否：0
 */
static uint IsShift(byte code)
{
    return (code==0x2A || code==0x36);
}

/**
 * @description: 判断是否为Caps键
 * @param 扫描码
 * @return 是：1， 否：0
 */
static uint IsCapsLock(byte code)
{
    return code == 0x3A;
}

/**
 * @description: 判断是否为NumLck键
 * @param 扫描码
 * @return 是：1， 否：0
 */
static uint IsNumLock(byte code)
{
    return code == 0x45;
}

/**
 * @description: 判断是否是数字小键盘
 * @param 扫描码
 * @param 扩展E0键标志
 * @return 是：1， 否：0
 */
static int IsNumPadKey(byte code, int E0)
{
    static const byte cNumScanCode[]                 = {0x52, 0x53, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47, 0x48, 0x49, 0x35, 0x37, 0x4A, 0x4E, 0x1C};  
    static const byte cNumE0[ArrayNum(cNumScanCode)] = {0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,    0,    0,    0,    1   };

    int ret = 0;
    int i = 0;

    for(i=0; i<ArrayNum(cNumScanCode); i++)
    {
        byte* pc = AddrOff(cNumScanCode, i);
        byte* pe = AddrOff(cNumE0, i);

        if(*pc==code && *pe==E0)
        {
            ret = 1;
            break;
        }
    }

    return ret;
}

/**
 * @description: 生成主键盘键码
 * @param 按键映射表
 * @param shift键状态
 * @param caps键状态
 * @return 键码
 */
static uint MakeNormalCode(KeyCode* tkc, int shift, int caps)
{
    uint ret = 0;
    if(!caps)   //没有按下caps
    {
        if(!shift)  //没有按下shift
            ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii1;
        else
            ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii2;
    }
    else        //按下caps
    {
        if(('a'<=tkc->ascii1) && (tkc->ascii1<='z'))    //是字母键
        {
            if(!shift)  //没有按下shift
                ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii2;
            else
                ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii1;
        }
        else
        {
            if(!shift)  //没有按下shift
                ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii1;
            else
                ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii2;
        }
    }

    return ret;
}

/**
 * @description: 生成小键盘键码
 * @param 按键映射表
 * @param shift键状态
 * @param numLock键状态
 * @return 键码
 */
static uint MakeNumCode(KeyCode* tkc, int shift, int numLock)
{
    static const KeyCode cNumKeyMap[] = 
    {
        { '0',       0,       0x52,     0x2D },
        { '.',       0,       0x53,     0x2E },
        { '1',       0,       0x4F,     0x23 },
        { '2',       0,       0x50,     0x28 },
        { '3',       0,       0x51,     0x22 },
        { '4',       0,       0x4B,     0x25 },
        { '5',       0,       0x4C,     0x0C },
        { '6',       0,       0x4D,     0x27 },
        { '7',       0,       0x47,     0x24 },
        { '8',       0,       0x48,     0x26 },
        { '9',       0,       0x49,     0x21 },
        { '/',      '/',      0x35,     0x6F },
        { '*',      '*',      0x37,     0x6A },
        { '-',      '-',      0x4A,     0x6D },
        { '+',      '+',      0x4E,     0x6B },
        {  0,        0,       0x1C,     0x0D },
        {  0,        0,        0,        0   }
    };

    uint ret = 0;
    int i = 0;
    KeyCode* nkc = AddrOff(cNumKeyMap, i);

    while(nkc->scode)
    {
        if(isEqual(tkc->scode, nkc->scode))
        {
            tkc = nkc;
            break;
        }
        i++;
        nkc = AddrOff(cNumKeyMap, i);
    }

    if(isEqual(tkc, nkc))
    {
        if(!numLock)  //没有按下numLock，不代表数字
        {
            ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii2;
        }
        else
        {
            if(!shift)
                ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii1;
            else
                ret = (tkc->scode << 16) | (tkc->kcode << 8) | tkc->ascii2;
        }
    }

    return ret;
}

/**
 * @description: 生成按键码
 * @param 按键映射表
 * @param shift键状态
 * @param capsLock键状态
 * @param numLock键状态
 * @param E0扩展键标志
 * @return 键码
 */
static uint MakeCode(KeyCode* tkc, int shift, int capsLock, int numLock, int E0)
{
    uint ret = 0;

    if(IsNumPadKey(tkc->scode, E0))    //判断是否是数字小键盘
    {
        ret = MakeNumCode(tkc, shift, numLock);
    }
    else
    {
        ret = MakeNormalCode(tkc, shift, capsLock);
    }
    return ret;
}

/**
 * @description: 将Pause键原始扫描码码映射到一个新的扫描码，然后通过按照处理普通按键的方式进行处理
 * @param 扫描码
 * @return 没有处理该扫描码：0  处理了该扫描码：1
 */
static uint PauseHandler(byte code)
{
    static byte cPauseCode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
    static int cPause = 0;
    uint ret = ((code == 0xE1) || cPause);

    if(ret)
    {
        byte* pc = AddrOff(cPauseCode, cPause);

        if(code == *pc)
        {
            cPause++;
        }
        else
        {
            cPause = 0;
            ret = 0;
        }

        //确认是pause键，就重新映射扫描码
        if(cPause == ArrayNum(cPauseCode))
        {
            cPause = 0;
            PutScanCode(0x5E);  //0xE1, 0x1D, 0x45 --> 0x5E【按下】
            PutScanCode(0xDE);  //0xE1, 0x9D, 0xC5 --> 0xDE【松开】
        }
    }
    return ret;
}

/**
 * @description: 获取缓冲区键码
 * @return 键码
 */
static uint FeachKeyCode()
{
    uint ret = 0;

    if(gKCBuff.count > 0)
    {
        uint *p = AddrOff(gKCBuff.buff, gKCBuff.head);
        ret = *p;

        gKCBuff.head = (gKCBuff.head + 1) % gKCBuff.max;
        gKCBuff.count--;
    }

    return ret;
}

/**
 * @description: 缓存按键信息
 * @param 键码
 */
static void StoreKeyCode(uint code)
{
    uint* p = NULL;
    
    if(gKCBuff.count < gKCBuff.max)
    {
        p = AddrOff(gKCBuff.buff, gKCBuff.tail);
        *p = code;
        gKCBuff.count++;
        gKCBuff.tail = (gKCBuff.tail + 1) % gKCBuff.max;
    }
    else if(gKCBuff.count > 0)
    {
        FeachKeyCode();
        StoreKeyCode(code); //只会递归一次
    }
}

/**
 * @description: 判断并处理普通按键扫描码
 * @param 扫描码
 * @return 没有处理该扫描码：0  处理了该扫描码：1
 */
static uint KeyHandler(byte code)
{
    static int cShift = 0;
    static int cCapsLock = 0;
    static int cNumLock = 0;
    static int E0 = 0;      
    uint ret = 0;

    if(code == 0xE0)    //判断是否时扩展扫描码
    {
        E0 = 1;
        ret = 1;
    }
    else 
    {
        uint pressed = GetKeyType(code);
        KeyCode* tkc = NULL;

        if(!pressed)
        {
            code -= 0x80;       //为了方便统一处理按下和松开键
        }

        tkc = AddrOff(gKeyMap, code);

        if( ret = !!(tkc->scode)) //确认是否支持当前按键
        {
            if(IsShift(code))
            {
                cShift = pressed;
            }
            else if(IsCapsLock(code) && pressed)
            {
                cCapsLock = cCapsLock ? 0 : 1;
            }
            else if(IsNumLock(code) && pressed)
            {
                cNumLock = cNumLock ? 0 : 1;
            }
            
            //生成键码，4字节，格式为：按键动作|扫描码|虚拟键码|ASCII码
            uint mcode = pressed | MakeCode(tkc, cShift, cCapsLock, cNumLock, E0);
            StoreKeyCode(mcode); //存储键码
            
            E0 = 0;
        }
    }

    return ret;
}

void KeyboardModInit()
{
    gKCBuff.max = 2;
    Queue_Init(&gWaitQueue);
}

void PutScanCode(byte code)
{
    if(PauseHandler(code))      //处理pause特殊按键
    {
        
    }
    else if(KeyHandler(code))   //处理普通按键
    {

    }
    else                        //其他特殊按键暂时不做处理
    {
        PrintString("Unknow Key\n");
    }
}


/**
 * @description: 进入等待队列
 * @param param1
 * @param param2
 */
static void Gowait(uint param1, uint param2)
{
    Event* event = CreateEvent(KeyboardEvent, (uint)&gWaitQueue, param1, 0);
    EventSchedule(WAIT, event);
}

/**
 * @description: 唤醒所有等待键盘输入的任务
 * @param 缓冲区的键码
 */
static void NotifyAll(uint code)
{
    Event event = {KeyboardEvent, (uint)&gWaitQueue, code, 0};
    EventSchedule(NOTIFY, &event);
}

/**
 * @description: 读取按键数据
 * @param param1
 * @param param2
 */
static void ReadKeyboard(uint param1, uint param2)
{
    if(param1)
    {
        uint kc = FeachKeyCode();

        if(kc)
        {
            uint *ret = (uint *)param1;
            *ret = kc;
            NotifyAll(kc);
        }
        else
        {
            Gowait(param1, param2);
        }
    }
}

void NotifyGetKeyCode()
{
    uint kc = FeachKeyCode();
    if(kc)
    {
        NotifyAll(kc);
    }
}

void KeyboardCallHandler(uint cmd, uint param1, uint param2)
{
    switch (cmd)
    {
        case 0:
        {
            ReadKeyboard(param1, param2);
            break;
        }
        
        default:
            break;
    }
}