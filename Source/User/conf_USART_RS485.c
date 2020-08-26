#include "conf_USART_RS485.h"
#include "101_Protocol.h"
#include "Flash.h"
#include "ds3231.h"
#include "fun.h"
#include "i2c_ee.h"
#include "main.h"
#include "modbus.h"
#include "mydef.h"
#include "stm32f10x.h"
#include "string.h"

extern volatile uint8_t CSD_485_COM_TxBuf[CSD_485_TX_BUF]; //在中断中发送数据
extern volatile uint16_t CSD_485_COM_RxFlag;
extern volatile uint16_t CSD_485_COM_RxTimeoutCnt;
extern __IO uint16_t CSD_485_COM_TxCnt;
extern __IO uint16_t CSD_485_COM_TxTop;

static void USART2_NVIC_Config(void);

/*
****************************************************************************************************
* 功能描述：初始化与MODBUS模块连接的USART参数
* 输入参数：
* 返回参数：
* 说    明：
****************************************************************************************************
*/
void USART2_Configuration(void) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //时钟要先设置？？？
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	USART_ClearFlag(CSD_485_COM, USART_IT_RXNE);
	USART_ClearFlag(CSD_485_COM, USART_FLAG_TC);

	//中断配置
	USART_ITConfig(CSD_485_COM, USART_IT_RXNE, ENABLE);
	USART_ITConfig(CSD_485_COM, USART_IT_TC, ENABLE); //开发送完成中断是因为发送一旦完成即设置为接收模式

	USART2_NVIC_Config();

	USART_Cmd(USART2, ENABLE);

	/* 开启调试端口引脚所在的GPIO的时钟 */
	/* modbus TX */
	GPIO_InitStructure.GPIO_Pin = CSD_485_COM_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CSD_485_COM_GPIO_PORT, &GPIO_InitStructure);

	/* modubs RX */
	GPIO_InitStructure.GPIO_Pin = CSD_485_COM_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(CSD_485_COM_GPIO_PORT, &GPIO_InitStructure);

	GPIO_ResetBits(CSD_485_COM_GPIO_PORT, CSD_485_COM_TX_PIN); //串口关断时此引脚为低电平

	/* modubs TX EN */
	GPIO_InitStructure.GPIO_Pin = CSD_485_TX_EN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(CSD_485_TX_EN_PORT, &GPIO_InitStructure);

	CSD_485_ENABLE_RX(); //接收模式
}

static void USART2_NVIC_Config(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = CSD_485_COM_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void USART2_IRQHandler(void) {
	//RX
	if (USART_GetITStatus(CSD_485_COM, USART_IT_RXNE) == SET) {
		USART_ClearFlag(CSD_485_COM, USART_FLAG_RXNE);
		CSD_485_COM_RxFlag = 1;
		CSD_485_COM_RxTimeoutCnt = 0;
		//CSD_485_UARTIsp();
	}
	//TX
	if (USART_GetITStatus(CSD_485_COM, USART_IT_TC) == SET) {
		USART_ClearFlag(CSD_485_COM, USART_FLAG_TC);
		CSD_485_COM_TxCnt++;
		if (CSD_485_COM_TxCnt >= CSD_485_COM_TxTop) {
			CSD_485_COM_TxTop = 0;
			CSD_485_ENABLE_RX();
		}
		else {
			CSD_485_ENABLE_TX();
			USART_SendData(USART2, CSD_485_COM_TxBuf[CSD_485_COM_TxCnt]); //启动发送
		}
	}
}
