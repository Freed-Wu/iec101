#include "mydef.h"
#include "stm32f10x.h"

/*
****************************************************************************************************
* 功能描述：
* 输入参数：
* 返回参数：      
* 说    明：
****************************************************************************************************
*/
void GPIO_Configuration(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	/********************************************************************************************************
								NRF905 相关引脚
	*********************************************************************************************************/
	/* Configure NRF905 POWERUP  nrf905上电*/
	GPIO_InitStructure.GPIO_Pin = NRF905_PWUP_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(NRF905_PWUP, &GPIO_InitStructure);

	/* Configure NRF905 TX OR RX CE  		NRF905使能芯片发送或接收*/
	GPIO_InitStructure.GPIO_Pin = NRF905_TRCE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(NRF905_TRCE, &GPIO_InitStructure);
	/* Configure NRF905 TX ENABLE  		NRF905设置发送或接收模式*/
	GPIO_InitStructure.GPIO_Pin = NRF905_TXEN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(NRF905_TXEN, &GPIO_InitStructure);
	/* Configure NRF905 CSN --> PA12 					NRF905使能SPI*/
	GPIO_InitStructure.GPIO_Pin = NRF905_CSN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(NRF905_CSN, &GPIO_InitStructure);
	/* Configure NRF905 CD  								NRF905输出 载波检测*/
	GPIO_InitStructure.GPIO_Pin = NRF905_CD_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(NRF905_CD, &GPIO_InitStructure);

	/* Configure NRF905 AM  								NRF地址匹配*/
	GPIO_InitStructure.GPIO_Pin = NRF905_AM_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(NRF905_AM, &GPIO_InitStructure);

	/* Configure NRF905 DR --> PA11  					NRF905接收或发送数据完成*/
	GPIO_InitStructure.GPIO_Pin = NRF905_DR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(NRF905_DR, &GPIO_InitStructure);

	/* Configure SPI CLK  								NRF905 SPI CLK*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Configure SPI MISO ON   							NRF905 SPI MISO*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Configure SPI MOSI ON   							NRF905 SPI MOSI*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

	/**********************************************************************
			Configure GSM TERM ON   
	**********************************************************************/

	GPIO_InitStructure.GPIO_Pin = GSM_RESET_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GSM_RESET, &GPIO_InitStructure);

	/* Configure GSM TERM ON   							1 GSM开关机控制*/
	GPIO_InitStructure.GPIO_Pin = GSM_PWUP_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GSM_PWUP, &GPIO_InitStructure);

	/* Configure UART RTS   							2 GSM请求发送*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_RTS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GSM_UART_RTS, &GPIO_InitStructure);

	/* Configure UART CTS    						3 GSM清除发送*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_CTS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GSM_UART_CTS, &GPIO_InitStructure);

	/* Configure UART DTR    						4 GSM数据终端就绪指示*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_DTR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GSM_UART_DTR, &GPIO_InitStructure);

	/* Configure UART DCD    						5 GSM载波检测*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_DCD_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GSM_UART_DCD, &GPIO_InitStructure);

	/* Configure UART DSR    						6 GSM数据设备就绪*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_DSR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GSM_UART_DSR, &GPIO_InitStructure);

	/* Configure UART RI    							7 GSM模块振铃指示*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_RI_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GSM_UART_RI, &GPIO_InitStructure);
	/************************************************************************
	Configure ARM RUN PIN  						指示LED
	*************************************************************************/
	GPIO_InitStructure.GPIO_Pin = ARM_RUN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(ARM_RUN, &GPIO_InitStructure);

	/* Configure Low Power detect PIN  BVL -->  PB0*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*************************************************************************


	************************************************************************/
	/*I2C INIT  DS3231 复位*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //RST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/*I2C INT  DS3231 输出脉冲 中断*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //INT
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/*I2C   DS3231 SCL */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*I2C   DS3231 SDA */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
