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
unsigned char RS232_buff[RS232_REC_BUFF_SIZE] = {0}; //用于接收数据
unsigned int RS232_rec_counter = 0; //用于RS232接收计数

static volatile uint16_t USART3_RxTimeoutCnt = 0;
static volatile uint16_t USART3_RxFlag = 0;
static volatile uint16_t USART3_RxLength = 0; //定义接收数据的长度
static volatile uint8_t USART3_RxBuf[GPRS_RCV_BUF]; //在中断中接收GPRS发送过来的数据

extern uint16_t DebugDly; //刚上电后等待配置，上电10分钟后串口发送调试数据出来
extern uint16_t DebugNRF905Dly;
uint16_t reqVersionflg;
/************************************************************************************
									调试端口
*************************************************************************************/
void USART1_Configuration(void) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 开启调试端口的时钟 */
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

	/* 配置调试端口的TX */
	GPIO_InitStructure.GPIO_Pin = DEBUG_COM_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_COM_GPIO_PORT, &GPIO_InitStructure);

	/* 配置调试端口的RX */
	GPIO_InitStructure.GPIO_Pin = DEBUG_COM_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_COM_GPIO_PORT, &GPIO_InitStructure);
}

/*
****************************************************************************************************
* 功能描述：初始化与GPRS模块连接的USART参数
* 输入参数：
* 返回参数：
* 说    明：
****************************************************************************************************
*/
void USART3_Configuration(void) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //时钟要先设置？？？
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

	/* 开启调试端口引脚所在的GPIO的时钟 */

	/* 配置调试端口的TX */
	GPIO_InitStructure.GPIO_Pin = GPRS_COM_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPRS_COM_GPIO_PORT, &GPIO_InitStructure);

	/* 配置调试端口的RX */
	GPIO_InitStructure.GPIO_Pin = GPRS_COM_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPRS_COM_GPIO_PORT, &GPIO_InitStructure);

	GPIO_ResetBits(GPRS_COM_GPIO_PORT, GPRS_COM_TX_PIN); //串口关断时此引脚为低电平
}

/*
****************************************************************************************************
* 功能描述：
* 输入参数：
* 返回参数：
* 说    明：
****************************************************************************************************
*/
void USART3_SendDataToGPRS(uint8_t* pString, uint16_t DataLength) {
	unsigned int i = 0;
	USART3_InitRXbuf();
	for (i = 0; i < DataLength; i++) {
		//if (DebugDly > 0)
		USART_SendData(DEBUG_COM, pString[i]); //用于调试信号
		USART_SendData(GPRS_COM, pString[i]);
		while (USART_GetFlagStatus(GPRS_COM, USART_FLAG_TXE) == RESET)
			;
	}
	USART3_RxFlag = 0; //重新启动一次接收
}
/*
****************************************************************************************************
* 功能描述：
* 输入参数：
* 返回参数：
* 说    明：
****************************************************************************************************
*/
void USART1_SendData(uint8_t* pString, uint16_t DataLength) {
	unsigned int i = 0;
	//USART3_InitRXbuf();
	for (i = 0; i < DataLength; i++) {
		//if (DebugDly > 0)
		USART_SendData(DEBUG_COM, pString[i]); //用于调试信号
		//USART_SendData(GPRS_COM, pString[i]);
		while (USART_GetFlagStatus(DEBUG_COM, USART_FLAG_TXE) == RESET)
			;
	}
	//USART3_RxFlag = 0; //重新启动一次接收
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
uint16_t USART1_RecDly;
void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收到了数据
	{
		USART1_RecDly = 25;
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
			else {
				if (strstr((char*)&RS232_buff[RS232_rec_counter - 5], "debug") || strstr((char*)&RS232_buff[RS232_rec_counter - 5], "DEBUG")) { //debug允许信号
					DebugDly = 50 * 60 * 1; //输出5分钟数据
					DebugNRF905Dly = 0;
					RS232_rec_counter = 0;
				}
				else if (strstr((char*)&RS232_buff[RS232_rec_counter - 6], "nrf905") || strstr((char*)&RS232_buff[RS232_rec_counter - 6], "NRF905")) { //debug允许信号
					DebugNRF905Dly = 50 * 60 * 1; //输出5分钟数?
					DebugDly = 0;
					RS232_rec_counter = 0;
				}
				else if (strstr((char*)&RS232_buff[RS232_rec_counter - 7], "version") || strstr((char*)&RS232_buff[RS232_rec_counter - 7], "VERSION")) //debug允许信号
					reqVersionflg = 1; //输出5分钟数?
			}
		}
		if (RS232_rec_counter > RS232_REC_BUFF_SIZE) //超过接收缓冲区大小
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
		USART3_RxFlag = 1; //启动中断的超时记数，每接收一组数据，这个都要置位一次
		USART3_RxTimeoutCnt = 0; //一直接收时清除延时
		USART3_RxBuf[USART3_RxLength] = USART_ReceiveData(GPRS_COM);
		if (DebugDly > 0) //接收到的信号通过调试串口发出
			DEBUG_COM->DR = USART3_RxBuf[USART3_RxLength];
		if (USART3_RxLength < 61)
			USART3_RxLength++;
	}
}

/***********************************************************************
函数名称：RUSART1_SendDataToRS232(uint8_t *pString,uint16_t DataLength)
功    能：RS232发送字符串
输入参数：send_buff:待发送的数据指针；length：发送的数据长度（字符个数）
输出参数：
编写时间：2015.11.22
编 写 人：
注    意：
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
函数名称：RS232_Send_Data(unsigned char *send_buff,unsigned int length)
功    能：监控USART3的接收数据,在接收到一串完整数据后置位标志位,数据写入data;
输入参数：
输出参数：返回数据长度，接收数据指针
编写时间：2015.12.19
编 写 人：陈连鹏
注    意：


****************************************************************************************/
uint16_t Supervise_USART3(uint8_t* pReceiveData) {
	uint16_t i;
	uint16_t pReceiveLength;

	pReceiveLength = 0;
	if (USART3_RxFlag == 1) //GPRS通过串口下发数据时就会置此位
		USART3_RxTimeoutCnt++; //每接收到一个数据，这个都会被清零
	else
		USART3_RxTimeoutCnt = 0;
	if (USART3_RxTimeoutCnt > 3) //接收数据后30ms
	{
		USART3_RxFlag = 0;
		USART3_RxTimeoutCnt = 0;
		memset(pReceiveData, 0, GPRS_RCV_BUF);
		for (i = 0; i < USART3_RxLength; i++) //最长63个，最后一个字节为字符串结束符
			pReceiveData[i] = USART3_RxBuf[i];
		pReceiveData[i] = '\0'; //字符串结束符，在一些字符串处理中需要通过这个识别结束条件
		pReceiveLength = USART3_RxLength;
		USART3_RxLength = 0;
	}
	return pReceiveLength; //返回数据长度
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
