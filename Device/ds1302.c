/*
  程序说明: DS1302驱动程序
  软件环境: Keil uVision 4.10 
  硬件环境: CT107单片机综合实训平台 8051，12MHz
  日    期: 2011-8-9
*/

#include <reg52.h>
#include <intrins.h>
#include "ds1302.h"

sbit SCK=P1^7;		
sbit SDA=P2^3;		
sbit RST = P1^3;   // DS1302复位												

void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK=0;
		SDA=temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_Ds1302(address);	
 	Write_Ds1302(dat);		
 	RST=0; 
}

unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}

void set_rtc(unsigned char *ucRtc)
{
	unsigned char i;
	Write_Ds1302_Byte(0x8e,0x00);
	Write_Ds1302_Byte(0x80,0x80);
	for(i = 0;i < 3;i++)
	{
		//鍗佽繘鍒惰浆bcd
		unsigned char bcd = ucRtc[i]/10*16 + ucRtc[i]%10;
		Write_Ds1302_Byte(0x84 - 2*i,bcd);
	}
	Write_Ds1302_Byte(0x8e,0x80);
}

void read_rtc(unsigned char *ucRtc)
{
	unsigned char i;
	unsigned char temp;
	EA = 0;
	for(i = 0;i < 3;i++)
	{
		temp = Read_Ds1302_Byte(0x85 - 2 * i);
		ucRtc[i] = temp/16*10 + temp % 16;
	}
	EA = 1;
}
