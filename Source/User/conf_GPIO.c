#include "mydef.h"
#include "stm32f10x.h"

/*
****************************************************************************************************
* ����������
* ���������
* ���ز�����      
* ˵    ����
****************************************************************************************************
*/
void GPIO_Configuration(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	/********************************************************************************************************
								NRF905 �������
	*********************************************************************************************************/
	/* Configure NRF905 POWERUP  nrf905�ϵ�*/
	GPIO_InitStructure.GPIO_Pin = NRF905_PWUP_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(NRF905_PWUP, &GPIO_InitStructure);

	/* Configure NRF905 TX OR RX CE  		NRF905ʹ��оƬ���ͻ����*/
	GPIO_InitStructure.GPIO_Pin = NRF905_TRCE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(NRF905_TRCE, &GPIO_InitStructure);
	/* Configure NRF905 TX ENABLE  		NRF905���÷��ͻ����ģʽ*/
	GPIO_InitStructure.GPIO_Pin = NRF905_TXEN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(NRF905_TXEN, &GPIO_InitStructure);
	/* Configure NRF905 CSN --> PA12 					NRF905ʹ��SPI*/
	GPIO_InitStructure.GPIO_Pin = NRF905_CSN_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(NRF905_CSN, &GPIO_InitStructure);
	/* Configure NRF905 CD  								NRF905��� �ز����*/
	GPIO_InitStructure.GPIO_Pin = NRF905_CD_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(NRF905_CD, &GPIO_InitStructure);

	/* Configure NRF905 AM  								NRF��ַƥ��*/
	GPIO_InitStructure.GPIO_Pin = NRF905_AM_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(NRF905_AM, &GPIO_InitStructure);

	/* Configure NRF905 DR --> PA11  					NRF905���ջ����������*/
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

	/* Configure GSM TERM ON   							1 GSM���ػ�����*/
	GPIO_InitStructure.GPIO_Pin = GSM_PWUP_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GSM_PWUP, &GPIO_InitStructure);

	/* Configure UART RTS   							2 GSM������*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_RTS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GSM_UART_RTS, &GPIO_InitStructure);

	/* Configure UART CTS    						3 GSM�������*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_CTS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GSM_UART_CTS, &GPIO_InitStructure);

	/* Configure UART DTR    						4 GSM�����ն˾���ָʾ*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_DTR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GSM_UART_DTR, &GPIO_InitStructure);

	/* Configure UART DCD    						5 GSM�ز����*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_DCD_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GSM_UART_DCD, &GPIO_InitStructure);

	/* Configure UART DSR    						6 GSM�����豸����*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_DSR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GSM_UART_DSR, &GPIO_InitStructure);

	/* Configure UART RI    							7 GSMģ������ָʾ*/
	GPIO_InitStructure.GPIO_Pin = GSM_UART_RI_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GSM_UART_RI, &GPIO_InitStructure);
	/************************************************************************
	Configure ARM RUN PIN  						ָʾLED
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
	/*I2C INIT  DS3231 ��λ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //RST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/*I2C INT  DS3231 ������� �ж�*/
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
