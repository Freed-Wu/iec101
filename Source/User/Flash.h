#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"

#define EEPROM_DATA_LEN 0x06
#define STM32_FLASH_PAGE_SIZE 0x400
#define FLAG_RECORD_USED 0xAA55
#define FLAG_RECORD_USED_LEN 0X02
#define EEPROM_PAGE_SIZE STM32_FLASH_PAGE_SIZE
#define EEPROM_FRAME_LEN (EEPROM_DATA_LEN + FLAG_RECORD_USED_LEN)
#define PAGE_REUSE_TIMES (STM32_FLASH_PAGE_SIZE) / (EEPROM_FRAME_LEN) //每页可写入数据个数
/**
** Functions declarations of API.
**/

uint16_t FLASH_WriteUserSet(void);
uint16_t FLASH_ReadUserSet(void);

int FLASH_WR_Module_Status(void);
void FLASH_RD_Module_Status(void);

#endif
