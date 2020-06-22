#include "crc.h"
#include "stm32f10x.h"

extern uint8_t InfoTemp[8]; //温度

/*读温度数据*/
uint16_t bkp_ReadTempData(void) {
	uint16_t tempdata[9];

	tempdata[0] = BKP_ReadBackupRegister(BKP_DR2);
	tempdata[1] = BKP_ReadBackupRegister(BKP_DR3);
	tempdata[2] = BKP_ReadBackupRegister(BKP_DR4);
	tempdata[3] = BKP_ReadBackupRegister(BKP_DR5);
	tempdata[4] = BKP_ReadBackupRegister(BKP_DR6);
	tempdata[5] = BKP_ReadBackupRegister(BKP_DR7);
	tempdata[6] = BKP_ReadBackupRegister(BKP_DR8);
	tempdata[7] = BKP_ReadBackupRegister(BKP_DR9);
	tempdata[8] = BKP_ReadBackupRegister(BKP_DR10);

	if (CRC16((uint8_t*)tempdata, 16) == tempdata[8]) {
		InfoTemp[0] = tempdata[0];
		InfoTemp[1] = tempdata[1];
		InfoTemp[2] = tempdata[2];
		InfoTemp[3] = tempdata[3];
		InfoTemp[4] = tempdata[4];
		InfoTemp[5] = tempdata[5];
		InfoTemp[6] = tempdata[6];
		InfoTemp[7] = tempdata[7];
		return 0;
	}

	return 1; //读取失败
}

/*写温度数据*/

uint16_t bkp_WriteTempData(void) {
	uint16_t tempdata[9];

	tempdata[0] = InfoTemp[0];
	tempdata[1] = InfoTemp[1];
	tempdata[2] = InfoTemp[2];
	tempdata[3] = InfoTemp[3];
	tempdata[4] = InfoTemp[4];
	tempdata[5] = InfoTemp[5];
	tempdata[6] = InfoTemp[6];
	tempdata[7] = InfoTemp[7];
	tempdata[8] = CRC16((uint8_t*)tempdata, 16); //每个数据占用两个字节

	BKP_WriteBackupRegister(BKP_DR2, tempdata[0]);
	BKP_WriteBackupRegister(BKP_DR3, tempdata[1]);
	BKP_WriteBackupRegister(BKP_DR4, tempdata[2]);
	BKP_WriteBackupRegister(BKP_DR5, tempdata[3]);
	BKP_WriteBackupRegister(BKP_DR6, tempdata[4]);
	BKP_WriteBackupRegister(BKP_DR7, tempdata[5]);
	BKP_WriteBackupRegister(BKP_DR8, tempdata[6]);
	BKP_WriteBackupRegister(BKP_DR9, tempdata[7]);
	BKP_WriteBackupRegister(BKP_DR10, tempdata[8]);

	return 0;
}
