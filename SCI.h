/***********************************************************************
文件名称：SCI.h
功    能：
编写时间：2012.11.22
编 写 人：
注    意：
***********************************************************************/
#ifndef _SCI_H_
#define _SCI_H_

#define RS232_REC_BUFF_SIZE 256
#define RS232_END_FLAG1 '?' //RS232一桢数据结束标志1
#define RS232_END_FLAG2 ';' //RS232一桢数据结束标志2

extern volatile unsigned char RS232_REC_Flag;
extern volatile unsigned char RS232_buff[RS232_REC_BUFF_SIZE]; //用于接收数据
extern volatile unsigned int RS232_rec_counter; //用于RS232接收计数

void USART1_Configuration(void);
void RS232_Send_Data(unsigned char* send_buff, unsigned int length);
void RS232_putchar(unsigned char send_char);

#endif
