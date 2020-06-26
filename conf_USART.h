#ifndef __USART_DRV_H__
#define __USART_DRV_H__

#include "mydef.h"
#include "stm32f10x.h"

void USART1_Configuration(void);
void USART3_Configuration(void);

void USART3_SendDataToGPRS(uint8_t* pString, uint16_t DataLength);
void USART1_SendCharToRS232(uint8_t CharData);
void USART1_SendDataToRS232(uint8_t* pString, uint16_t DataLength);
void USART1_SendData(uint8_t* pString, uint16_t DataLength);

void USART1_IRQHandler(void);
void USART3_IRQHandler(void);
void USART3_InitRXbuf(void);

uint16_t Supervise_USART3(uint8_t* pReceiveData);

void USART1_supervise(void);

#endif //__USART_DRV_H__
