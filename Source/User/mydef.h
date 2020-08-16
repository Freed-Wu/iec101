#ifndef _mydef_h
#define _define_h



/*****************************************************************************
*	GSM													*
*														*
******************************************************************************/
 #define GSM_PWUP                      	 	GPIOB
  #define GSM_PWUP_PIN                     	GPIO_Pin_15 
   
  #define GSM_RESET                      	GPIOB
  #define GSM_RESET_PIN                     GPIO_Pin_2 

  
  #define GSM_UART_RTS                      GPIOB
  #define GSM_UART_RTS_PIN                  GPIO_Pin_9
  
  #define GSM_UART_CTS                      	GPIOB
  #define GSM_UART_CTS_PIN                    GPIO_Pin_13

  #define GSM_UART_DTR                      	GPIOB
  #define GSM_UART_DTR_PIN                    GPIO_Pin_12  

  #define GSM_UART_DCD                      	GPIOB
  #define GSM_UART_DCD_PIN                    GPIO_Pin_14
  
  #define GSM_UART_DSR                      	GPIOB
  #define GSM_UART_DSR_PIN                    GPIO_Pin_8
  													 
  #define GSM_UART_RI                      	GPIOB
  #define GSM_UART_RI_PIN                    GPIO_Pin_1
 /*end define GSM module control pin*/  

  #define RCC_GSM_CNTL_PIN                   RCC_APB2Periph_GPIOB
  
/****************************************************************************
*	NRF905																	*
*																			*
*****************************************************************************/
  /*define NRF905 control pin*/  
  #define NRF905_PWUP                      	 GPIOA			////nrf905上电
  #define NRF905_PWUP_PIN                     GPIO_Pin_0

  #define NRF905_TRCE                      	 GPIOA			//使能芯片发送或接收
  #define NRF905_TRCE_PIN                     GPIO_Pin_2   

  #define NRF905_TXEN                      	 GPIOA			//设置发送或接收模式
  #define NRF905_TXEN_PIN                     GPIO_Pin_3
	 

  #define NRF905_CSN                      	GPIOA			//使能SPI
  #define NRF905_CSN_PIN                    GPIO_Pin_12	
  #define RCC_NRF905_CSN                    RCC_APB2Periph_GPIOA
    	 
  #define NRF905_CD                      	GPIOA			//nrf输出载波检测	
  #define NRF905_CD_PIN                     GPIO_Pin_1
  	 
  #define NRF905_AM                      	GPIOA			//nrf地址匹配
  #define NRF905_AM_PIN                     GPIO_Pin_4	
													   
  //#define NRF905_DR                      	GPIOA
  //#define NRF905_DR_PIN                     GPIO_Pin_9
  //#define GPIO_NRF905_DR_EXTI_Line          EXTI_Line9

  #define NRF905_DR                      	GPIOA			//接收或发送数据完成
  #define NRF905_DR_PIN                   GPIO_Pin_11
  #define GPIO_NRF905_DR_EXTI_Line        EXTI_Line11 
	
/****************************************************************************
*	DS3231																	*
*																			*
*****************************************************************************/	
	
	#define TIMER_SCL                      	GPIOB
  #define TIMER_SCL_PIN                     GPIO_Pin_6 
  #define TIMER_SDA                      	GPIOB
  #define TIMER_SDA_PIN                     GPIO_Pin_7
  #define TIMER_RST                      	GPIOC
  #define TIMER_RST_PIN                     GPIO_Pin_10	
  #define TIMER_INT                      	GPIOC
  #define TIMER_INT_PIN                     GPIO_Pin_11

  #define ARM_RUN                      		GPIOC			//ARM运行指示灯 低电平亮
  #define ARM_RUN_PIN                     	GPIO_Pin_9


#define DEBUG_COM                   USART1
#define DEBUG_COM_CLK               RCC_APB2Periph_USART1

#define DEBUG_COM_GPIO_PORT         GPIOA
#define DEBUG_COM_GPIO_CLK          RCC_APB2Periph_GPIOA

#define DEBUG_COM_TX_PIN            GPIO_Pin_9
#define DEBUG_COM_RX_PIN            GPIO_Pin_10

#define DEBUG_COM_TX_SOURCE         GPIO_PinSource9
#define DEBUG_COM_RX_SOURCE         GPIO_PinSource10
#define DEBUG_COM_GPIO_AF           GPIO_AF_USART1
#define DEBUG_COM_IRQn              USART1_IRQn



#define GPRS_COM               		USART3
#define GPRS_COM_CLK               	RCC_APB1Periph_USART3
#define GPRS_COM_GPIO_PORT         	GPIOB
#define GPRS_COM_TX_PIN            	GPIO_Pin_10
#define GPRS_COM_RX_PIN            	GPIO_Pin_11
#define GPRS_COM_IRQn              	USART3_IRQn
#define GPRS_RCV_BUF 			   	80

#define RS232_REC_BUFF_SIZE			80		 
#define RS232_END_FLAG1	'?'			//RS232一桢数据结束标志1 
#define RS232_END_FLAG2	';'			//RS232一桢数据结束标志2 


//MODBUS ??
#define CSD_485_COM              		USART2
#define CSD_485_COM_CLK               	RCC_APB1Periph_USART2
#define CSD_485_COM_GPIO_PORT         	GPIOA
#define CSD_485_COM_TX_PIN            	GPIO_Pin_2
#define CSD_485_COM_RX_PIN            	GPIO_Pin_3
#define CSD_485_COM_IRQn              	USART2_IRQn
#define CSD_485_RX_BUF 			   	64
#define CSD_485_TX_BUF 			   	64

#define CSD_485_TX_EN_PORT         	GPIOA
#define CSD_485_TX_EN_PIN           GPIO_Pin_1

#define CSD_485_ENABLE_TX() GPIO_SetBits(CSD_485_TX_EN_PORT,CSD_485_TX_EN_PIN)
#define CSD_485_ENABLE_RX() GPIO_ResetBits(CSD_485_TX_EN_PORT,CSD_485_TX_EN_PIN)

#define TRUE    1
#define FALSE   0




#endif





