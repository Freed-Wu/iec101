#include "Para_Config.h"
#include "Flash.h"
#include "MG301.h"
#include "fun.h"
#include "stm32f10x.h"
#include "string.h"
ConfigParaObj MG301_ConfigParaObj;
uint8_t IsFirstUsed = 0; //是否第一次上电标志位
uint8_t ConfigFlag = 0; //配置信息标志位，置1表示有信息需要写入到FLASH中

void ReadParaToRam(ConfigParaObj* ParaObj) {
	uint8_t i = 0;
	uint8_t Heart[32] = {0x55, 0xaa, 0x33, 0x44, 0x66};
	FLASH_Read((uint32_t)ParaSaveAddress, (uint8_t*)ParaObj->ServerIP_Port, 60);
	if ((ParaObj->WriteFlag1 != 0x55) || (ParaObj->WriteFlag2 != 0x33)) {
		ParaObj->WriteFlag1 = 0x55;
		ParaObj->WriteFlag2 = 0x33;
		ParaObj->LinkAddress = 0x0021;
		memcpy(ParaObj->ServerIP_Port, "180.175.212.248:1008", strlen("180.175.212.248:1008"));
		ParaObj->HeartTime = 30; //心跳包间隔为30s
		for (i = 0; i < 32; i++) {
			ParaObj->HeartData[i] = Heart[i];
		}
		ParaObj->HeartLength = 5;
		ParaObj->Res = 0;

		FLASH_Write((uint32_t)ParaSaveAddress, (unsigned char*)ParaObj->ServerIP_Port, 60);
	}
	else {
		//FLASH_Read(ParaSaveAddress,ParaObj->ServerIP_Port,55);
	}
}

void WriteParaToFlash(ConfigParaObj* ParaObj) {
	FLASH_Write(ParaSaveAddress, (unsigned char*)ParaObj->ServerIP_Port, 55);
}
