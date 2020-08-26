#ifndef __USART_RS485_H__
#define __USART_RS485_H__

#include "mydef.h"
#include "stm32f10x.h"

void USART2_Configuration(void);

void USART2_SendDataToGPRS(uint8_t* pString, uint16_t DataLength);
void USART2_IRQHandler(void);
void USART2_InitRXbuf(void);

void SuperviseModbus(void);

#endif //__USART_RS485_H__
