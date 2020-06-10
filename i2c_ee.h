/*********文件名：i2c_ee.h**********/

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __I2C_EE_H
#define __I2C_EE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported macro ------------------------------------------------------------*/
#define ADDR_24CXX 0xD0

#define SCLH GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_SET) // GPIOB->BSRR = GPIO_Pin_6
#define SCLL GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_RESET) //GPIOB->BRR  = GPIO_Pin_6

#define SDAH GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET) //GPIOB->BSRR = GPIO_Pin_7
#define SDAL GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET) //GPIOB->BRR  = GPIO_Pin_7

#define SCLread GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) //GPIOB->IDR  & GPIO_Pin_6
#define SDAread GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) //GPIOB->IDR  & GPIO_Pin_7

/* Exported functions ------------------------------------------------------- */
void I2C_EE_Init(void);
unsigned char I2C_EE_BufferWrite(unsigned char* psrc_data, unsigned char adr, unsigned char nbyte);
unsigned char I2C_EE_BufferRead(unsigned char* pdin_data, unsigned char adr, unsigned char nbyte);
unsigned char TWI_ReadByte(unsigned char ReadAddress);
unsigned char TWI_WriteByte(unsigned char SendByte, unsigned char WriteAddress);
#endif /* __I2C_EE_H */
