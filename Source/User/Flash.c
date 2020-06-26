#include "Flash.h"
#include "crc.h"
#include "fun.h"
#include "main.h"
#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "string.h"

extern uint8_t Info[16];

//用户设置数据做3个备份，当数据1检验失败时则读取数据2，数据2校验失败时读取数据3
#define EE_STARTADDR_USERSET_0 ((u32)0x0800F000)
#define EE_STARTADDR_USERSET_1 ((u32)0x0800F400)

//跌落数据用两个
#define EE_STARTADDR_STATUS_0 ((u32)0x0800f800)
#define EE_STARTADDR_STATUS_1 ((u32)0x0800fc00)

//跌落模块状态数据用模拟EEPROM存储

extern DEVICE_SET user_Set;

typedef enum {
	uFLASH_BUSY = 1,
	uFLASH_ERROR_PG,
	uFLASH_ERROR_WRP,
	uFLASH_COMPLETE,
	uFLASH_TIMEOUT,

} uFLASH_Status;

/*
********************************************************************************
  * 函数名称：FLASH_Page_Erase
  * 函数说明：擦除指定页面
  * 输入参数：uint32_t _addr  页面内任一地址 擦除页面后查空，非空时擦除3次
  *
  * 输出参数：无
  * 返回参数：	FLASH_COMPLETE 控除完成
				FLASH_ERROR_WRP 控除失败
  * 备    注：
********************************************************************************
*/
static FLASH_Status FLASH_Page_Erase(uint32_t _addr) {
	uint16_t i;
	uint16_t ErrCnt;
	ErrCnt = 0;

	_addr &= 0xfffffc00; //页对齐

	while (ErrCnt < 3) {
		FLASH_SetLatency(FLASH_Latency_2);
		FLASH_Unlock();
		FLASH_ErasePage(_addr); //擦除页
		FLASH_WaitForLastOperation(0x000B0000); //等待擦除完成

		for (i = 0; i < STM32_FLASH_PAGE_SIZE; i += 2) {
			if (*(uint16_t*)(_addr + STM32_FLASH_PAGE_SIZE) != 0xffff) {
			}
		}
		if (i == STM32_FLASH_PAGE_SIZE) {
			break;
		}
		{ ErrCnt++; }
	}
	if (ErrCnt == 3)
		return FLASH_ERROR_WRP;

	return FLASH_COMPLETE;
}

/*
********************************************************************************
  * 函数名称：FLASH_Write
  * 函数说明：向FLASH中一次写入半字（2字节）
  * 输入参数：uint32_t Address  要写入的首地址
  *            uint8_t *DataObj  待写入数据的首地址
  *            uint16_t Counter  待写入的字节数
  * 输出参数：无
  * 返回参数：无
  * 备    注：
********************************************************************************
*/
FLASH_Status FLASH_Write(uint32_t Address, uint16_t* DataObj, uint16_t Counter) {
	FLASH_Status Status = FLASH_COMPLETE;

	if (Status == FLASH_COMPLETE) {
		Counter = Counter / 2;
		while (Counter--) {
			Status = FLASH_ProgramHalfWord(Address, *(uint16_t*)DataObj);
			if (Status == FLASH_COMPLETE) {
				Address += 2;
				DataObj++;
				Status = FLASH_WaitForLastOperation(0x00002000); //等待编程完成

				if (Status != FLASH_COMPLETE) {
					break;
				}
			}
			else {
				break;
			}
		}
	}
	return Status;
}

/*
********************************************************************************
  * 函数名称：FLASH_Read
  * 函数说明：读Counter字节数据，Counter必须为偶数个，bit0清零
  * 输入参数：
  * 输出参数：
  * 返回参数：
  * 备    注：
********************************************************************************
*/
void FLASH_Read(uint32_t Address, uint16_t* DataObj, uint16_t Counter) {
	Counter = Counter >> 1;
	while (Counter) {
		*DataObj = *(uint16_t*)Address;
		Address += 2;
		DataObj++;
		Counter--;
	}
}
/*

*/
/*
********************************************************************************
  * 函数名称：FLASH_Check
  * 函数说明：检测内存数据和FLASH数据是否一致
  * 输入参数：
  * 输出参数：
  * 返回参数：
  * 备    注：
********************************************************************************
*/
uint16_t FLASH_Check(uint32_t Address, uint16_t* DataObj, uint16_t Counter) {
	Counter = Counter >> 1;

	while (Counter) {
		if (*DataObj != *(uint16_t*)Address) {
			return ERROR;
		}
		Address += 2;
		DataObj++;
		Counter--;
	}
	if (Counter != 0) {
		return FLASH_ERROR_WRP;
	}
	return SUCCESS;
}

/*

设置数据写入
*/

uint16_t FLASH_WriteUserSet(void) {
	uint16_t errcnt;

	user_Set.crc16 = CRC16((uint8_t*)(&user_Set), (uint32_t)(&user_Set.crc16) - (uint32_t)(&user_Set));

	FLASH_Unlock();
	errcnt = 0;
	while (errcnt < 3) { //最多写入3次

		FLASH_Page_Erase(EE_STARTADDR_USERSET_0);
		FLASH_Write(EE_STARTADDR_USERSET_0, (uint16_t*)(&user_Set), sizeof(user_Set));

		if (FLASH_Check(EE_STARTADDR_USERSET_0, (uint16_t*)&user_Set, sizeof(user_Set)) == SUCCESS) {
			break;
		}
	}
	//备份数据
	errcnt = 0;
	while (errcnt < 3) { //最多写入3次

		FLASH_Page_Erase(EE_STARTADDR_USERSET_1);
		FLASH_Write(EE_STARTADDR_USERSET_1, (uint16_t*)(&user_Set), sizeof(user_Set));

		if (FLASH_Check(EE_STARTADDR_USERSET_1, (uint16_t*)&user_Set, sizeof(user_Set)) == SUCCESS) {
			break;
		}
	}
	FLASH_Lock();
	return SUCCESS;
}

/*

设置数据读出


*/
uint16_t FLASH_ReadUserSet(void) {
	uint16_t crc;

	FLASH_Read(EE_STARTADDR_USERSET_0, (uint16_t*)(&user_Set), sizeof(user_Set)); //读出数据

	crc = CRC16((uint8_t*)(&user_Set), (uint32_t)(&user_Set.crc16) - (uint32_t)(&user_Set));
	if (crc == user_Set.crc16) {
		return SUCCESS;
	}
	else {
		FLASH_Read(EE_STARTADDR_USERSET_1, (uint16_t*)(&user_Set), sizeof(user_Set)); //读出数据
		crc = CRC16((uint8_t*)(&user_Set), (uint32_t)(&user_Set.crc16) - (uint32_t)(&user_Set));
		if (crc == user_Set.crc16) {
			return SUCCESS;
		}
		else {
			return ERROR;
		}
	}
}

/*
*********************************************************************************************
*********************************************************************************************

							状态值部分用仿真eeprom实现

*********************************************************************************************
*********************************************************************************************
*/

/*
			跌落模块处理部分
【说明】跌落模块用一共用四个页面，每两个页面一组，每帧数据8字节存储值，
数据位16bit 每个16bit数据包含16个状态值

0XAA55 16bit 16bit CRC16
*/

//Flash 写入时是按16位数据进行的
//写入模块的状态到Flash中
#define DataNUM 3 //(温度)值字节数
#define ONFFRAMEBYTES (uint16_t)((DataNUM * 2) + 2)
#define WRITEFLG (uint16_t)0XF55A

/*
1，找到空地址写入
2，读出写入的数据
3，检查写入数据是否正确
4，如果写入数据不正确，则重新找下一个地址写入数据，直到写入正确数据为止

传入数据格式 DATA DATA CRC16
*/

static FLASH_Status FLASH_WR_OneFrameToOnePage(uint32_t PAGE_StartAddress, uint16_t* pData) {
	u32 Address;
	uint16_t ValidIndex;
	FLASH_Status Status;

	//解锁FLASH
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	//1 找数据头
	for (ValidIndex = 0; ValidIndex <= (STM32_FLASH_PAGE_SIZE - EEPROM_FRAME_LEN); ValidIndex += EEPROM_FRAME_LEN) //找到未写入的字节
	{
		Address = PAGE_StartAddress + ValidIndex;
		if (0xffff == *(uint16_t*)Address) {
			break;
		}
	}

	//2 本页已经写满则清除整个页面
	if (ValidIndex > (STM32_FLASH_PAGE_SIZE - EEPROM_FRAME_LEN)) {
		FLASH_Page_Erase(PAGE_StartAddress);
		Address = PAGE_StartAddress; //清除后地址转到第一个地址
	}
	//3 写入一帧数据

	Status = FLASH_ProgramHalfWord(Address, FLAG_RECORD_USED);
	Status = FLASH_WaitForLastOperation(0x00002000); //等待编程完成
	Status = FLASH_Write(Address + 2, pData, 6);

	FLASH_Lock();

	return Status;
}

//从Flash中读取一帧数据
//flash中的原始数据
//
static FLASH_Status FLASH_RD_OneFrameFromOnePage(uint32_t PAGE_StartAddress, uint16_t* pDest) {
	uint16_t i;
	uint16_t ValidIndex;

	//检测下一帧是否有数据写入，运行到最后一帧数据时直接退出，因为最后一帧的下一帧已经超出了数据范围，则最后一帧即为有效数据帧
	for (ValidIndex = 0; ValidIndex < (STM32_FLASH_PAGE_SIZE - EEPROM_FRAME_LEN); ValidIndex += EEPROM_FRAME_LEN) //找到未写入的字节//找到未写入的字节
	{
		if (FLAG_RECORD_USED != *(uint16_t*)(PAGE_StartAddress + ValidIndex + EEPROM_FRAME_LEN)) {
			break;
		}
	}

	// when the 'current' record is found, read data of it
	for (i = 0; i < (EEPROM_DATA_LEN / 2); i++)
		*(pDest + i) = *(uint16_t*)(PAGE_StartAddress + 2 + ValidIndex + i * 2);

	return FLASH_COMPLETE;
}

/*
********************************************************************************
  * 函数名称：FLASH_WR_Module_Status
  * 函数说明：写入一帧数据
  * 输入参数：
  * 输出参数：
  * 返回参数：
  * 备    注：
********************************************************************************
*/

int FLASH_WR_Module_Status(void) {
	uint16_t pdata[3];
	uint16_t rData[3];
	uint16_t i;

	//生成要存入的数据

	if (CheckInfoCRCIsOK() == 0) {
		FLASH_RD_Module_Status();
	}

	pdata[0] = 0x00; //重新排列数据
	for (i = 0; i < 16; i++) { //一共存储十个数据
		if (Info[i] == 0) { //正常
			pdata[0] = pdata[0] | (1 << i);
		}
	}
	pdata[1] = pdata[0];
	pdata[2] = CRC16((uint8_t*)pdata, 4);

	rData[0] = 0;
	rData[1] = 1;
	rData[2] = 2;
	//写入数据 写入失败时最多写满整个页
	i = 0;
	while (i < PAGE_REUSE_TIMES) {
		i++;
		FLASH_RD_OneFrameFromOnePage(EE_STARTADDR_STATUS_0, rData); //先读出数据，如果数据一致则不存储
		if ((pdata[0] == rData[0]) && (pdata[1] == rData[1]) && (pdata[2] == rData[2])) {
			break;
		}
		FLASH_WR_OneFrameToOnePage(EE_STARTADDR_STATUS_0, pdata);
	}

	//写入数据 写入失败时最多写满整个页
	rData[0] = 0;
	rData[1] = 1;
	rData[2] = 2;
	i = 0;
	while (i < PAGE_REUSE_TIMES) { //先读出数据，如果数据一致则不存储
		i++;
		FLASH_RD_OneFrameFromOnePage(EE_STARTADDR_STATUS_1, rData);
		if ((pdata[0] == rData[0]) && (pdata[1] == rData[1]) && (pdata[2] == rData[2])) {
			break;
		}
		FLASH_WR_OneFrameToOnePage(EE_STARTADDR_STATUS_1, pdata);
	}

	return 0;
}

//从Flash中读取模块的状态，在系统复位时读取。
void FLASH_RD_Module_Status(void) {
	uint16_t rData[3];
	uint16_t i;

	rData[0] = 0;
	rData[1] = 1;
	rData[2] = 2;
	FLASH_RD_OneFrameFromOnePage(EE_STARTADDR_STATUS_0, rData);

	if ((rData[0] != rData[1]) || CRC16((uint8_t*)rData, 4) != rData[2]) {
		rData[0] = 0;
		rData[1] = 1;
		rData[2] = 2;

		FLASH_RD_OneFrameFromOnePage(EE_STARTADDR_STATUS_1, rData);

		if ((rData[0] != rData[1]) || CRC16((uint8_t*)rData, 4) != rData[2]) { //读FLASH失败时置默认值
			for (i = 0; i < 16; i++) {
				Info[i] = 0;
			}
			RefreshInfoCRC();
			return;
		}
	}

	for (i = 0; i < 16; i++) { //读数据成功时
		if ((rData[1] & (1 << i)) == 0) { //跌落
			Info[i] = 1;
		}
		else {
			Info[i] = 0; //正常
		}
	}
	RefreshInfoCRC();
}
