#include "SCI.H"
#include "stm32f10x_lib.h"
/***********************************************************************
�ļ����ƣ�SCI.C
��    �ܣ���ɶ�usart1�Ĳ���
��дʱ�䣺2012.11.22
�� д �ˣ�
ע    �⣺��������ͨ���ж������ض����ַ���ȷ��һ֡�����Ƿ�����ġ�
***********************************************************************/

volatile unsigned char RS232_REC_Flag = 0;
volatile unsigned char RS232_buff[RS232_REC_BUFF_SIZE] = 0; //���ڽ�������
volatile unsigned int RS232_rec_counter = 0; //����RS232���ռ���

void USART1_Configuration(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/********************����ΪUSART1����**************************/
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
	//USART_InitStructure.USART_WordLength = USART_WordLength_9b;//9λ����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //8λ����
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //1λֹͣλ
	//USART_InitStructure.USART_Parity = USART_Parity_Even;//żУ�飬��У��ʱӦ��ѡ��9λ����
	USART_InitStructure.USART_Parity = USART_Parity_No; //��żʧ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Ӳ��������ʧ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //���ͺͽ���ʹ��

	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //ʹ��USART1�ж�
	USART_Cmd(USART1, ENABLE); //ʹ��USART1����
	USART_ClearITPendingBit(USART1, USART_IT_TC); //����ж�TCλ

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/***********************************************************************
�������ƣ�void USART1_IRQHandler(void)
��    �ܣ����SCI�����ݵĽ��գ�������ʶ
���������
���������
��дʱ�䣺2012.11.22
�� д �ˣ�
ע    �⣺RS232�õ���USART1
***********************************************************************/
void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //���յ�������
	{
		RS232_buff[RS232_rec_counter] = USART1->DR; //
		RS232_rec_counter++;
		/***��RS232_END_FLAG1��RS232_END_FLAG2������ַ���Ϊһ֡���ݵĽ�����ʶ***/
		if (RS232_rec_counter >= 2) //ֻ�н��յ�2���������ϲ����ж�
		{
			if (RS232_buff[RS232_rec_counter - 2] == RS232_END_FLAG1 && RS232_buff[RS232_rec_counter - 1] == RS232_END_FLAG2) //֡��ʼ��־
			{
				RS232_REC_Flag = 1;
				RS232_rec_counter = 0;
			}
		}
		if (RS232_rec_counter > RS232_REC_BUFF_SIZE) //�������ջ�������С
		{
			RS232_rec_counter = 0;
		}
	}
	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
		USART_ClearITPendingBit(USART1, USART_IT_TXE); /* Clear the USART transmit interrupt                  */
	}
}

/***********************************************************************
�������ƣ�RS232_Send_Data(unsigned char *send_buff,unsigned int length)
��    �ܣ�RS232�����ַ���
���������send_buff:�����͵�����ָ�룻length�����͵����ݳ��ȣ��ַ�������
���������
��дʱ�䣺2012.11.22
�� д �ˣ�
ע    �⣺
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
