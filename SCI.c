#include "SCI.H"
#include "stm32f10x_lib.h"
/***********************************************************************
文件名称：SCI.C
功    能：完成对usart1的操作
编写时间：2012.11.22
编 写 人：
注    意：本例程是通过判断两个特定的字符来确定一帧数据是否结束的。
***********************************************************************/

volatile unsigned char RS232_REC_Flag = 0;
volatile unsigned char RS232_buff[RS232_REC_BUFF_SIZE] = 0; //用于接收数据
volatile unsigned int RS232_rec_counter = 0; //用于RS232接收计数

void USART1_Configuration(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/********************以下为USART1配置**************************/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);
	/*
  *  USART1_TX -> PA9 , USART1_RX ->	PA10
  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	//USART_InitStructure.USART_WordLength = USART_WordLength_9b;//9位数据
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //8位数据
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //1位停止位
	//USART_InitStructure.USART_Parity = USART_Parity_Even;//偶校验，有校验时应该选择9位数据
	USART_InitStructure.USART_Parity = USART_Parity_No; //奇偶失能
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //硬件流控制失能
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //发送和接受使能

	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //使能USART1中断
	USART_Cmd(USART1, ENABLE); //使能USART1外设
	USART_ClearITPendingBit(USART1, USART_IT_TC); //清除中断TC位

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/***********************************************************************
函数名称：void USART1_IRQHandler(void)
功    能：完成SCI的数据的接收，并做标识
输入参数：
输出参数：
编写时间：2012.11.22
编 写 人：
注    意：RS232用的是USART1
***********************************************************************/
void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收到了数据
	{
		RS232_buff[RS232_rec_counter] = USART1->DR; //
		RS232_rec_counter++;
		/***以RS232_END_FLAG1和RS232_END_FLAG2定义的字符作为一帧数据的结束标识***/
		if (RS232_rec_counter >= 2) //只有接收到2个数据以上才做判断
		{
			if (RS232_buff[RS232_rec_counter - 2] == RS232_END_FLAG1 && RS232_buff[RS232_rec_counter - 1] == RS232_END_FLAG2) //帧起始标志
			{
				RS232_REC_Flag = 1;
				RS232_rec_counter = 0;
			}
		}
		if (RS232_rec_counter > RS232_REC_BUFF_SIZE) //超过接收缓冲区大小
		{
			RS232_rec_counter = 0;
		}
	}
	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
		USART_ClearITPendingBit(USART1, USART_IT_TXE); /* Clear the USART transmit interrupt                  */
	}
}

/***********************************************************************
函数名称：RS232_Send_Data(unsigned char *send_buff,unsigned int length)
功    能：RS232发送字符串
输入参数：send_buff:待发送的数据指针；length：发送的数据长度（字符个数）
输出参数：
编写时间：2012.11.22
编 写 人：
注    意：
***********************************************************************/
void RS232_Send_Data(unsigned char* send_buff, unsigned int length) {
	unsigned int i = 0;
	for (i = 0; i < length; i++) {
		USART1->DR = send_buff[i];
		while ((USART1->SR & 0X40) == 0)
			;
	}
}

void RS232_putchar(unsigned char send_char) {
	USART1->DR = send_char;
	while ((USART1->SR & 0X40) == 0)
		;
}
