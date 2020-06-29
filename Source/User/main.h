#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f10x.h"
#include "string.h"

typedef struct {
	uint16_t ip_len;
	uint8_t ip_info[16];
	uint16_t port_len;
	uint8_t port_info[6];
	uint16_t addr_len;
	uint8_t addr_info[6];
	uint8_t ModuleID[4];
	uint16_t apn_len;
	uint8_t apn_info[8];
	uint16_t user_len;
	uint8_t user_info[8];
	uint16_t password_len;
	uint8_t password_info[8];
	uint16_t heart_len; //���������ݳ���
	uint8_t heart_info[16]; //����������
	uint16_t heart_time_len; //������ʱ�䳤�ȣ����ַ���ʾ������21����Ϊ2��
	uint8_t heart_time_info[6]; //������ʱ��
	uint16_t crc16;
	uint16_t FirstUsedFlag;
	uint32_t wordAlign; //δʹ��,�������ݶ��룬�����ǰ�����ݲ������֣���������ֽڴճ����֣�����Ĳ��ֲ��洢����ȡ��ʹ�ò�����
} DEVICE_SET;

extern void SysDelay(uint16_t DelayMs);
extern unsigned char WriteDATATime(void);
uint16_t CheckInfoCRCIsOK(void);
void RefreshInfoCRC(void);

#endif //__MAIN_H__
