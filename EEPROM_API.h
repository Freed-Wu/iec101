/**
* Copyright (c) 2006, Freescale Semiconductor
* File name : STM32_EEPROM_API.h
* CPU : MC9S08
* Compiler: CodeWarrior v5.1 08C Compiler
*
* Author : Jerry Shi (b01136@freescale.com)
*
* Description : This head file includes definition for subroutines that
* emulating EEPROM with FLASH memory on HCS08 MCU.
*
* History :
* 10/08/2006 : API works on MC9S08QG8CDTE
*/
#ifndef __STM32_EEPROM_API_H
#define __STM32_EEPROM_API_H
/**
** Below macros define some nesseccsry data for EEPROM emulation.
** Users MUST modify those macros according to their own application.
** PRM file also should be modified to update the ROM address and
** reserve EEPROM address (2 pages).
** STACKSIZE must be carefully reviewed because the APU functions
** uses many stack size. Do-on-stack code size is 43 bytes.
** Function STM32_EE_ModifyByte() uses as much as EEPROM_DATA_LEN
** bytes of stack to hold the data buffer.
**/
/**
** EEPROM data is treated as fixed-size record.
** FLAG, DATA0, DATA1, DATA2, ..., DATAn
**
**/
#define EEPROM_DATA_LEN 0x0E
/**
** Refer to the PRM file of your project and define STM32_FLASH_START_ADDR
** accordingly. Also the PRM file should be modified to specify the new ROM
** address. 2 pages are used for EEPROM emulatio, thus ROM = ROM + 0x200.
**/
#define STM32_FLASH_START_ADDR 0xE000
/**
** Page size of HCS08 is 0x200
**/
#define STM32_FLASH_PAGE_SIZE 0x400
/**
** FLASH block protection. Default is off.
** It is stongly to turn it on to protect the code space of CPU.
**/
#define STM32_FLASH_PROTECTION 0xFF
/**
** Possible return values of operation result.
**/
#define STM32_EE_ERROR_OK 0x00
#define STM32_EE_ERROR_UNKOWN 0xFF
#define STM32_EE_ERROR_OUT_OF_RANGE 0x01
/**
** Below macros are defined for API use only.
** Do NOT modify it.
**/
#define FLAG_RECORD_USED 0xAA55
#define FLAG_RECORD_USED_LEN 0X02
#define EEPROM_START_ADDR STM32_FLASH_START_ADDR
#define EEPROM_PAGE_SIZE STM32_FLASH_PAGE_SIZE
#define EEPROM_FRAME_LEN (EEPROM_DATA_LEN + FLAG_RECORD_USED_LEN)
/**
			每帧的数据头为半字
**/
#define PAGE_REUSE_TIMES (STM32_FLASH_PAGE_SIZE) / (EEPROM_DATA_LEN + 2)
/**
** Functions declarations of API.
**/

void STM32_EE_WriteRecord(unsigned char* src);
void STM32_EE_ReadRecord(unsigned char* dest);

#endif
