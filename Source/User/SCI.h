/***********************************************************************
�ļ����ƣ�SCI.h
��    �ܣ�
��дʱ�䣺2012.11.22
�� д �ˣ�
ע    �⣺
***********************************************************************/
#ifndef _SCI_H_
#define _SCI_H_

#define RS232_REC_BUFF_SIZE 256
#define RS232_END_FLAG1 '?' //RS232һ�����ݽ�����־1
#define RS232_END_FLAG2 ';' //RS232һ�����ݽ�����־2

extern volatile unsigned char RS232_REC_Flag;
extern volatile unsigned char RS232_buff[RS232_REC_BUFF_SIZE]; //���ڽ�������
extern volatile unsigned int RS232_rec_counter; //����RS232���ռ���

void USART1_Configuration(void);
void RS232_Send_Data(unsigned char* send_buff, unsigned int length);
void RS232_putchar(unsigned char send_char);

#endif
