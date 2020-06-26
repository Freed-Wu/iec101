#include "Flash.h"
#include "crc.h"
#include "fun.h"
#include "main.h"
#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "string.h"

extern uint8_t Info[16];

//�û�����������3�����ݣ�������1����ʧ��ʱ���ȡ����2������2У��ʧ��ʱ��ȡ����3
#define EE_STARTADDR_USERSET_0 ((u32)0x0800F000)
#define EE_STARTADDR_USERSET_1 ((u32)0x0800F400)

//��������������
#define EE_STARTADDR_STATUS_0 ((u32)0x0800f800)
#define EE_STARTADDR_STATUS_1 ((u32)0x0800fc00)

//����ģ��״̬������ģ��EEPROM�洢

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
  * �������ƣ�FLASH_Page_Erase
  * ����˵��������ָ��ҳ��
  * ���������uint32_t _addr  ҳ������һ��ַ ����ҳ����գ��ǿ�ʱ����3��
  *
  * �����������
  * ���ز�����	FLASH_COMPLETE �س����
				FLASH_ERROR_WRP �س�ʧ��
  * ��    ע��
********************************************************************************
*/
static FLASH_Status FLASH_Page_Erase(uint32_t _addr) {
	uint16_t i;
	uint16_t ErrCnt;
	ErrCnt = 0;

	_addr &= 0xfffffc00; //ҳ����

	while (ErrCnt < 3) {
		FLASH_SetLatency(FLASH_Latency_2);
		FLASH_Unlock();
		FLASH_ErasePage(_addr); //����ҳ
		FLASH_WaitForLastOperation(0x000B0000); //�ȴ��������

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
  * �������ƣ�FLASH_Write
  * ����˵������FLASH��һ��д����֣�2�ֽڣ�
  * ���������uint32_t Address  Ҫд����׵�ַ
  *            uint8_t *DataObj  ��д�����ݵ��׵�ַ
  *            uint16_t Counter  ��д����ֽ���
  * �����������
  * ���ز�������
  * ��    ע��
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
				Status = FLASH_WaitForLastOperation(0x00002000); //�ȴ�������

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
  * �������ƣ�FLASH_Read
  * ����˵������Counter�ֽ����ݣ�Counter����Ϊż������bit0����
  * ���������
  * ���������
  * ���ز�����
  * ��    ע��
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
  * �������ƣ�FLASH_Check
  * ����˵��������ڴ����ݺ�FLASH�����Ƿ�һ��
  * ���������
  * ���������
  * ���ز�����
  * ��    ע��
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

��������д��
*/

uint16_t FLASH_WriteUserSet(void) {
	uint16_t errcnt;

	user_Set.crc16 = CRC16((uint8_t*)(&user_Set), (uint32_t)(&user_Set.crc16) - (uint32_t)(&user_Set));

	FLASH_Unlock();
	errcnt = 0;
	while (errcnt < 3) { //���д��3��

		FLASH_Page_Erase(EE_STARTADDR_USERSET_0);
		FLASH_Write(EE_STARTADDR_USERSET_0, (uint16_t*)(&user_Set), sizeof(user_Set));

		if (FLASH_Check(EE_STARTADDR_USERSET_0, (uint16_t*)&user_Set, sizeof(user_Set)) == SUCCESS) {
			break;
		}
	}
	//��������
	errcnt = 0;
	while (errcnt < 3) { //���д��3��

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

�������ݶ���


*/
uint16_t FLASH_ReadUserSet(void) {
	uint16_t crc;

	FLASH_Read(EE_STARTADDR_USERSET_0, (uint16_t*)(&user_Set), sizeof(user_Set)); //��������

	crc = CRC16((uint8_t*)(&user_Set), (uint32_t)(&user_Set.crc16) - (uint32_t)(&user_Set));
	if (crc == user_Set.crc16) {
		return SUCCESS;
	}
	else {
		FLASH_Read(EE_STARTADDR_USERSET_1, (uint16_t*)(&user_Set), sizeof(user_Set)); //��������
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

							״ֵ̬�����÷���eepromʵ��

*********************************************************************************************
*********************************************************************************************
*/

/*
			����ģ�鴦����
��˵��������ģ����һ�����ĸ�ҳ�棬ÿ����ҳ��һ�飬ÿ֡����8�ֽڴ洢ֵ��
����λ16bit ÿ��16bit���ݰ���16��״ֵ̬

0XAA55 16bit 16bit CRC16
*/

//Flash д��ʱ�ǰ�16λ���ݽ��е�
//д��ģ���״̬��Flash��
#define DataNUM 3 //(�¶�)ֵ�ֽ���
#define ONFFRAMEBYTES (uint16_t)((DataNUM * 2) + 2)
#define WRITEFLG (uint16_t)0XF55A

/*
1���ҵ��յ�ַд��
2������д�������
3�����д�������Ƿ���ȷ
4�����д�����ݲ���ȷ������������һ����ַд�����ݣ�ֱ��д����ȷ����Ϊֹ

�������ݸ�ʽ DATA DATA CRC16
*/

static FLASH_Status FLASH_WR_OneFrameToOnePage(uint32_t PAGE_StartAddress, uint16_t* pData) {
	u32 Address;
	uint16_t ValidIndex;
	FLASH_Status Status;

	//����FLASH
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	//1 ������ͷ
	for (ValidIndex = 0; ValidIndex <= (STM32_FLASH_PAGE_SIZE - EEPROM_FRAME_LEN); ValidIndex += EEPROM_FRAME_LEN) //�ҵ�δд����ֽ�
	{
		Address = PAGE_StartAddress + ValidIndex;
		if (0xffff == *(uint16_t*)Address) {
			break;
		}
	}

	//2 ��ҳ�Ѿ�д�����������ҳ��
	if (ValidIndex > (STM32_FLASH_PAGE_SIZE - EEPROM_FRAME_LEN)) {
		FLASH_Page_Erase(PAGE_StartAddress);
		Address = PAGE_StartAddress; //������ַת����һ����ַ
	}
	//3 д��һ֡����

	Status = FLASH_ProgramHalfWord(Address, FLAG_RECORD_USED);
	Status = FLASH_WaitForLastOperation(0x00002000); //�ȴ�������
	Status = FLASH_Write(Address + 2, pData, 6);

	FLASH_Lock();

	return Status;
}

//��Flash�ж�ȡһ֡����
//flash�е�ԭʼ����
//
static FLASH_Status FLASH_RD_OneFrameFromOnePage(uint32_t PAGE_StartAddress, uint16_t* pDest) {
	uint16_t i;
	uint16_t ValidIndex;

	//�����һ֡�Ƿ�������д�룬���е����һ֡����ʱֱ���˳�����Ϊ���һ֡����һ֡�Ѿ����������ݷ�Χ�������һ֡��Ϊ��Ч����֡
	for (ValidIndex = 0; ValidIndex < (STM32_FLASH_PAGE_SIZE - EEPROM_FRAME_LEN); ValidIndex += EEPROM_FRAME_LEN) //�ҵ�δд����ֽ�//�ҵ�δд����ֽ�
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
  * �������ƣ�FLASH_WR_Module_Status
  * ����˵����д��һ֡����
  * ���������
  * ���������
  * ���ز�����
  * ��    ע��
********************************************************************************
*/

int FLASH_WR_Module_Status(void) {
	uint16_t pdata[3];
	uint16_t rData[3];
	uint16_t i;

	//����Ҫ���������

	if (CheckInfoCRCIsOK() == 0) {
		FLASH_RD_Module_Status();
	}

	pdata[0] = 0x00; //������������
	for (i = 0; i < 16; i++) { //һ���洢ʮ������
		if (Info[i] == 0) { //����
			pdata[0] = pdata[0] | (1 << i);
		}
	}
	pdata[1] = pdata[0];
	pdata[2] = CRC16((uint8_t*)pdata, 4);

	rData[0] = 0;
	rData[1] = 1;
	rData[2] = 2;
	//д������ д��ʧ��ʱ���д������ҳ
	i = 0;
	while (i < PAGE_REUSE_TIMES) {
		i++;
		FLASH_RD_OneFrameFromOnePage(EE_STARTADDR_STATUS_0, rData); //�ȶ������ݣ��������һ���򲻴洢
		if ((pdata[0] == rData[0]) && (pdata[1] == rData[1]) && (pdata[2] == rData[2])) {
			break;
		}
		FLASH_WR_OneFrameToOnePage(EE_STARTADDR_STATUS_0, pdata);
	}

	//д������ д��ʧ��ʱ���д������ҳ
	rData[0] = 0;
	rData[1] = 1;
	rData[2] = 2;
	i = 0;
	while (i < PAGE_REUSE_TIMES) { //�ȶ������ݣ��������һ���򲻴洢
		i++;
		FLASH_RD_OneFrameFromOnePage(EE_STARTADDR_STATUS_1, rData);
		if ((pdata[0] == rData[0]) && (pdata[1] == rData[1]) && (pdata[2] == rData[2])) {
			break;
		}
		FLASH_WR_OneFrameToOnePage(EE_STARTADDR_STATUS_1, pdata);
	}

	return 0;
}

//��Flash�ж�ȡģ���״̬����ϵͳ��λʱ��ȡ��
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

		if ((rData[0] != rData[1]) || CRC16((uint8_t*)rData, 4) != rData[2]) { //��FLASHʧ��ʱ��Ĭ��ֵ
			for (i = 0; i < 16; i++) {
				Info[i] = 0;
			}
			RefreshInfoCRC();
			return;
		}
	}

	for (i = 0; i < 16; i++) { //�����ݳɹ�ʱ
		if ((rData[1] & (1 << i)) == 0) { //����
			Info[i] = 1;
		}
		else {
			Info[i] = 0; //����
		}
	}
	RefreshInfoCRC();
}
