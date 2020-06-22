#include "eeprom_api.h"
#include "stm32f10x.h"

static void flash_byte_prog(uint16_t _addr, uint16_t _data);
static void flash_page_erase(unsigned int _addr);
static uint16_t flash_HalfWordRead(uint16_t _addr);

/*
** ===================================================================
** Method : STM32_EE_WriteRecord
**
** Description :
** this function writes the whold data record to the next
** available space block of FLASH, also the Record Has Been
** used flag before the data. If the current page will be full,
** it erases the next page.
** Patameters:
** unsigned char *. -- point to the source data
** Return Values:
** no 以半字为单位存储
** ===================================================================
*/
void STM32_EE_WriteRecord(uint16_t* src) {
	unsigned int i, j;
	unsigned char flag_page_switch;

	for (;;) {
		for (i = 0; i < PAGE_REUSE_TIMES * EEPROM_DATA_LEN; i = i + EEPROM_FRAME_LEN) { //找记录第一个0XAA55
			if (FLAG_RECORD_USED != flash_HalfWordRead(EEPROM_START_ADDR + i)) {
				// this record space is empty
				// wite flag to mark this record is used
				flash_byte_prog(EEPROM_START_ADDR + i, FLAG_RECORD_USED);
				// then write the record data to the following addresses
				for (j = 0; j < EEPROM_DATA_LEN / 2; j++) //每次一个半字
				{
					FLASH_ProgramHalfWord(EEPROM_START_ADDR + 1 + i + j, *(src + j));
				}
				// do we need to switch to next page?
				flash_page_erase(EEPROM_START_ADDR);
				return;
			}
		}

	} //for(;;)
	return;
}

/*
** ===================================================================
** Method : STM32_EE_ReadRecord
**
** Description :
** this function finds the current record in the current page,
** then reads the all the data in the record to a buffer.
** Patameters:
** unsigned char *. -- point to the dest buffer
** Return Values:
** no 读数据到缓冲区
** ===================================================================
*/
void STM32_EE_ReadRecord(unsigned char* dest) {
	unsigned int i, j;
	// visit all the records till find out the 'current' record
	for (i = 0; i < (PAGE_REUSE_TIMES - 1) * EEPROM_DATA_LEN; i += EEPROM_DATA_LEN + 1) {
		if (FLAG_RECORD_USED != flash_HalfWordRead(EEPROM_START_ADDR + 1 + i + EEPROM_DATA_LEN))
			break;
	}
	// when the 'current' record is found, read data of it
	for (j = 0; j < EEPROM_DATA_LEN; j++)
		*(dest + j) = flash_HalfWordRead(EEPROM_START_ADDR + 1 + i + j);
	return;
}

/*
** ===================================================================
** Method : flash_byte_prog
**
** Description :
** flash_byte_prog programs one byte of data to the specified
** address of FLASH. this is an internal method only used by API.
** Patameters:
** unsigned int, -- target address to be programmed
** unsigned char. -- data to be programmed
** Return Values:
** no
** ===================================================================
*/
static void flash_uint16_prog(unsigned int _addr, unsigned char _data) {}
/*
** ===================================================================
** Method : flash_page_erase
**
** Description :
** flash_page_erase erases one page that contains the specified
** address of FLASH. this is an internal method only used by API.
** Patameters:
** unsigned int. -- target address to be erased
** Return Values:
** no
** ===================================================================
*/
static void flash_page_erase(unsigned int _addr) {}
/*
** ===================================================================
** Method : flash_byte_read
**
** Description :
** flash_byte_read reads one byte data from the specified
** address of FLASH. this is an internal method only used by API.
** Patameters:
** unsigned int. -- target address to be erased
** Return Values:
** unsigned char. -- the data
** ===================================================================
*/
static uint16_t flash_HalfWordRead(uint16_t _addr) {
	return *(uint16_t*)_addr;
}
