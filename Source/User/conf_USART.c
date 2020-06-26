#include "conf_USART.h"
#include "101_Protocol.h"
#include "Flash.h"
#include "MG301.h"
#include "ds3231.h"
#include "fun.h"
#include "i2c_ee.h"
#include "main.h"
#include "mydef.h"
#include "stm32f10x.h"
#include "string.h"

unsigned char RS232_REC_Flag = 0;
unsigned char RS232_buff[RS232_REC_BUFF_SIZE] = {0}; //���ڽ�������
unsigned int RS232_rec_counter = 0; //����RS232���ռ���

static volatile uint16_t USART3_RxTimeoutCnt = 0;
static volatile uint16_t USART3_RxFlag = 0;
static volatile uint16_t USART3_RxLength = 0; //����������ݵĳ���
static volatile uint8_t USART3_RxBuf[GPRS_RCV_BUF]; //���ж��н���GPRS���͹���������

extern uint16_t DebugDly; //���ϵ��ȴ����ã��ϵ�10���Ӻ󴮿ڷ��͵������ݳ���
extern uint16_t DebugNRF905Dly;
uint16_t reqVersionflg;
/************************************************************************************
									���Զ˿�
*************************************************************************************/
void USART1_Configuration(void) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* �������Զ˿ڵ�ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(DEBUG_COM, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* ���õ��Զ˿ڵ�TX */
	GPIO_InitStructure.GPIO_Pin = DEBUG_COM_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_COM_GPIO_PORT, &GPIO_InitStructure);

	/* ���õ��Զ˿ڵ�RX */
	GPIO_InitStructure.GPIO_Pin = DEBUG_COM_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_COM_GPIO_PORT, &GPIO_InitStructure);
}

/*
****************************************************************************************************
* ������������ʼ����GPRSģ�����ӵ�USART����
* ���������
* ���ز�����
* ˵    ����
****************************************************************************************************
*/
void USART3_Configuration(void) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //ʱ��Ҫ�����ã�����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_2;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ClearFlag(GPRS_COM, USART_IT_RXNE);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);
	//USART_ITConfig(USART3,USART_IT_TC,ENABLE);
	USART_Cmd(USART3, ENABLE);

	/* �������Զ˿��������ڵ�GPIO��ʱ�� */

	/* ���õ��Զ˿ڵ�TX */
	GPIO_InitStructure.GPIO_Pin = GPRS_COM_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPRS_COM_GPIO_PORT, &GPIO_InitStructure);

	/* ���õ��Զ˿ڵ�RX */
	GPIO_InitStructure.GPIO_Pin = GPRS_COM_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPRS_COM_GPIO_PORT, &GPIO_InitStructure);

	GPIO_ResetBits(GPRS_COM_GPIO_PORT, GPRS_COM_TX_PIN); //���ڹض�ʱ������Ϊ�͵�ƽ
}

/*
****************************************************************************************************
* ����������
* ���������
* ���ز�����
* ˵    ����
****************************************************************************************************
*/
void USART3_SendDataToGPRS(uint8_t* pString, uint16_t DataLength) {
	unsigned int i = 0;
	USART3_InitRXbuf();
	for (i = 0; i < DataLength; i++) {
		//if (DebugDly > 0)
		USART_SendData(DEBUG_COM, pString[i]); //���ڵ����ź�
		USART_SendData(GPRS_COM, pString[i]);
		while (USART_GetFlagStatus(GPRS_COM, USART_FLAG_TXE) == RESET)
			;
	}
	USART3_RxFlag = 0; //��������һ�ν���
}
/*
****************************************************************************************************
* ����������
* ���������
* ���ز�����
* ˵    ����
****************************************************************************************************
*/
void USART1_SendData(uint8_t* pString, uint16_t DataLength) {
	unsigned int i = 0;
	//USART3_InitRXbuf();
	for (i = 0; i < DataLength; i++) {
		//if (DebugDly > 0)
		USART_SendData(DEBUG_COM, pString[i]); //���ڵ����ź�
		//USART_SendData(GPRS_COM, pString[i]);
		while (USART_GetFlagStatus(DEBUG_COM, USART_FLAG_TXE) == RESET)
			;
	}
	//USART3_RxFlag = 0; //��������һ�ν���
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
uint16_t USART1_RecDly;
void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //���յ�������
	{
		USART1_RecDly = 25;
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
			else {
				if (strstr((char*)&RS232_buff[RS232_rec_counter - 5], "debug") || strstr((char*)&RS232_buff[RS232_rec_counter - 5], "DEBUG")) { //debug�����ź�
					DebugDly = 50 * 60 * 1; //���5��������
					DebugNRF905Dly = 0;
					RS232_rec_counter = 0;
				}
				else if (strstr((char*)&RS232_buff[RS232_rec_counter - 6], "nrf905") || strstr((char*)&RS232_buff[RS232_rec_counter - 6], "NRF905")) { //debug�����ź�
					DebugNRF905Dly = 50 * 60 * 1; //���5������?
					DebugDly = 0;
					RS232_rec_counter = 0;
				}
				else if (strstr((char*)&RS232_buff[RS232_rec_counter - 7], "version") || strstr((char*)&RS232_buff[RS232_rec_counter - 7], "VERSION")) //debug�����ź�
					reqVersionflg = 1; //���5������?
			}
		}
		if (RS232_rec_counter > RS232_REC_BUFF_SIZE) //�������ջ�������С
			RS232_rec_counter = 0;
	}
}

void USART1_supervise(void) {
	uint16_t i;
	if (USART1_RecDly)
		USART1_RecDly--;
	if (USART1_RecDly == 1) {
		for (i = 0; i < RS232_REC_BUFF_SIZE; i++)
			RS232_buff[i] = 0;
		RS232_rec_counter = 0;
	}
}

void USART3_IRQHandler(void) {
	if (USART_GetFlagStatus(GPRS_COM, USART_FLAG_RXNE) == SET) {
		USART_ClearFlag(GPRS_COM, USART_FLAG_RXNE);
		USART3_RxFlag = 1; //�����жϵĳ�ʱ������ÿ����һ�����ݣ������Ҫ��λһ��
		USART3_RxTimeoutCnt = 0; //һֱ����ʱ�����ʱ
		USART3_RxBuf[USART3_RxLength] = USART_ReceiveData(GPRS_COM);
		if (DebugDly > 0) //���յ����ź�ͨ�����Դ��ڷ���
			DEBUG_COM->DR = USART3_RxBuf[USART3_RxLength];
		if (USART3_RxLength < 61)
			USART3_RxLength++;
	}
}

/***********************************************************************
�������ƣ�RUSART1_SendDataToRS232(uint8_t *pString,uint16_t DataLength)
��    �ܣ�RS232�����ַ���
���������send_buff:�����͵�����ָ�룻length�����͵����ݳ��ȣ��ַ�������
���������
��дʱ�䣺2015.11.22
�� д �ˣ�
ע    �⣺
***********************************************************************/
uint16_t usart1sentdelay;
void USART1_SendDataToRS232(uint8_t* pString, uint16_t DataLength) {
	unsigned int i = 0;
	for (i = 0; i < DataLength; i++) {
		USART1->DR = pString[i];
		usart1sentdelay = 65535;
		while ((USART1->SR & 0X40) == 0) {
			usart1sentdelay--;
			if (usart1sentdelay == 0)
				break;
		}
	}
}

void USART1_SendCharToRS232(uint8_t CharData) {
	USART1->DR = CharData;
	while ((USART1->SR & 0X40) == 0)
		;
	USART_ClearITPendingBit(USART1, USART_IT_TC);
}
/***************************************************************************************
�������ƣ�RS232_Send_Data(unsigned char *send_buff,unsigned int length)
��    �ܣ����USART3�Ľ�������,�ڽ��յ�һ���������ݺ���λ��־λ,����д��data;
���������
����������������ݳ��ȣ���������ָ��
��дʱ�䣺2015.12.19
�� д �ˣ�������
ע    �⣺


****************************************************************************************/
uint16_t Supervise_USART3(uint8_t* pReceiveData) {
	uint16_t i;
	uint16_t pReceiveLength;

	pReceiveLength = 0;
	if (USART3_RxFlag == 1) //GPRSͨ�������·�����ʱ�ͻ��ô�λ
		USART3_RxTimeoutCnt++; //ÿ���յ�һ�����ݣ�������ᱻ����
	else
		USART3_RxTimeoutCnt = 0;
	if (USART3_RxTimeoutCnt > 3) //�������ݺ�30ms
	{
		USART3_RxFlag = 0;
		USART3_RxTimeoutCnt = 0;
		memset(pReceiveData, 0, GPRS_RCV_BUF);
		for (i = 0; i < USART3_RxLength; i++) //�63�������һ���ֽ�Ϊ�ַ���������
			pReceiveData[i] = USART3_RxBuf[i];
		pReceiveData[i] = '\0'; //�ַ�������������һЩ�ַ�����������Ҫͨ�����ʶ���������
		pReceiveLength = USART3_RxLength;
		USART3_RxLength = 0;
	}
	return pReceiveLength; //�������ݳ���
}

void USART3_InitRXbuf(void) {
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
	USART3_RxFlag = 0;
	USART3_RxLength = 0;
	USART3_RxTimeoutCnt = 0;
	//memset(USART3_RxBuf,0,GPRS_RCV_BUF);
	USART_ClearFlag(GPRS_COM, USART_IT_RXNE);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}
