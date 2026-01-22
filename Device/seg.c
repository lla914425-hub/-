#include "seg.h"

pdata unsigned char seg_dula[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,
																	0xbf,   //-
	0xc6,  //C	
	0x8c,  //P
		};

void seg_disp(unsigned char wela,unsigned char dula,bit point)
{
	unsigned char temp;
	//消隐
	P0 = 0xff;
	temp = P2 & 0x1f;
	temp |= 0xe0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;
	
	P0 = 0x01 << wela;
	temp = P2 & 0x1f;
	temp |= 0xc0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;
	
	P0 = seg_dula[dula];
	if(point)
		P0 &= 0x7f;//小数点亮
	else
		P0 |= 0x80;//小数点灭
	temp = P2 & 0x1f;
	temp |= 0xe0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;
}