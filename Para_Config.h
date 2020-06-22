#ifndef __PARA_CONFIG_H__
#define __PARA_CONFIG_H__

#include "stm32f10x.h"

#define ParaSaveAddress 0x0800C800

// ������Ҫ����Ĳ�����֯��һ���ṹ���У���������д
typedef struct {
	int8_t ServerIP_Port[21];
	uint8_t HeartTime;
	int8_t HeartData[32]; //����������Ϊ32�ֽ�
	uint8_t HeartLength;
	uint16_t LinkAddress;
	uint8_t WriteFlag1;
	uint8_t WriteFlag2;
	uint8_t Res;
} ConfigParaObj;

extern ConfigParaObj MG301_ConfigParaObj;

extern uint8_t ConfigFlag;

extern void ReadParaToRam(ConfigParaObj* ParaObj);

extern void WriteParaToFlash(ConfigParaObj* ParaObj);

#endif //__PARA_CONFIG_H__
