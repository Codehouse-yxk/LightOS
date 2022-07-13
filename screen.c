
/**
 * 屏幕打印功能
*/

#include "screen.h"

static byte gPosW = 0;
static byte gPosH = 0;
static byte gColor = SCREEN_WHITE;

void ClearScreen()
{
	int h = 0;
	int w = 0;
	SetPrintPos(0, 0);
	for (h = 0; h < SCREEN_HEIGHT; h++)
	{
		for (w = 0; w < SCREEN_WIDTH; w++)
		{
			PrintChar(' ');
		}
	}
	SetPrintPos(0, 0);
}

int SetPrintPos(byte w, byte h)
{
	int ret = 0;
	if (ret = ((w < SCREEN_WIDTH) && (h < SCREEN_HEIGHT)))
	{
		gPosW = w;
		gPosH = h;

		/* 重定位光标 */
		ushort bx = SCREEN_WIDTH * h + w;
		asm volatile(
			//设置光标位置高8位
			"movw %0, %%bx\n"
			"movw $0x03D4, %%dx\n"
			"movb $0x0E, %%al\n"
			"outb %%al, %%dx\n"
			"movw $0x03D5, %%dx\n"
			"movb %%bh, %%al\n"
			"outb %%al, %%dx\n"

			//设置光标位置低8位
			"movw %0, %%bx\n"
			"movw $0x03D4, %%dx\n"
			"movb $0x0F, %%al\n"
			"outb %%al, %%dx\n"
			"movw $0x03D5, %%dx\n"
			"movb %%bl, %%al\n"
			"outb %%al, %%dx\n"
			"\n"
			"\n"
			"\n"
			"\n"
			:
			: "r"(bx)
			: "ax", "bx", "dx"
		);
	}
	return ret;
}

void SetPrintColor(PrintColor c)
{
	gColor = c;
}

int PrintChar(const char c)
{
	int ret = 0;
	if ((c == '\n') || (c == '\r')) //进行换行
	{
		ret = SetPrintPos(0, gPosH + 1);
	}
	else
	{
		byte pw = gPosW;
		byte ph = gPosH;
		if ((pw < SCREEN_WIDTH) && (ph < SCREEN_HEIGHT))
		{
			int edi = (SCREEN_WIDTH * ph + pw) * 2;
			byte ah = gColor;
			char al = c;
			asm volatile(
				"movl %0, %%edi\n"
				"movb %1, %%ah\n"
				"movb %2, %%al\n"
				"movw %%ax, %%gs:(%%edi)\n"
				:							 //输出参数，未使用
				: "r"(edi), "r"(ah), "r"(al) //输入参数，限定符r表示任何寄存器
				: "edi", "ax"				 //告诉编译器这两个寄存器的值被改动
			);
			pw++;
			if (pw == SCREEN_WIDTH)
			{
				pw = 0;
				ph++;
			}
			ret = 1;
		}

		SetPrintPos(pw, ph);
	}
	return ret;
}

int PrintString(const char *s)
{
	int cnt = 0;

	if (s)
	{
		while (*s)
		{
			cnt += PrintChar(*s++);
		}
	}
	else
	{
		cnt = -1;
	}

	return cnt;
}

int PrintIntDec(int n)
{
	int ret = 0;
	if (n < 0)
	{
		ret += PrintChar('-');
		n = -n;
		ret += PrintIntDec(n);
	}
	else
	{
		if (n < 10)
		{
			ret += PrintChar('0' + n);
		}
		else
		{
			ret += PrintIntDec(n / 10);
			ret += PrintIntDec(n % 10);
		}
	}

	return ret;
}

int PrintIntHex(uint n)
{
	char hex[11] = {'0', 'x'}; // 11 = '0' + 'x' + '8个F' + '结束 0'

	int i = 0;
	for (i = 9; i >= 2; i--)
	{
		int p = 0xF & n;
		if (p < 10)
		{
			hex[i] = ('0' + p); //将0~9的数字转换为对应字符
		}
		else
		{
			hex[i] = ('A' + p - 10); //将A~F的数字转换为对应字符
		}
		n >>= 4;
	}

	return PrintString(hex);
}
