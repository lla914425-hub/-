#ifndef _IIC_H
#define _IIC_H

void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 

unsigned char AD_read(unsigned char addr);
void DA_write(unsigned char dat);
void EEPROM_write(unsigned char *str,unsigned char addr,unsigned char num);
void EEPROM_read(unsigned char *str,unsigned char addr,unsigned char num);

void EEPROM_write_byte(unsigned char dat,unsigned char addr);
unsigned char EEPROM_read_byte(unsigned char addr);

#endif