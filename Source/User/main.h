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
	uint16_t heart_len; //心跳包数据长度
	uint8_t heart_info[16]; //心跳包数据
	uint16_t heart_time_len; //心跳包时间长度（以字符表示，比如21秒则为2）
	uint8_t heart_time_info[6]; //心跳包时间
	uint16_t crc16;
	uint16_t FirstUsedFlag;
	uint32_t wordAlign; //未使用,用于数据对齐，即如果前面数据不是整字，则由这个字节凑成整字，多余的部分不存储不读取不使用不检验
} DEVICE_SET;

extern void SysDelay(uint16_t DelayMs);
extern unsigned char WriteDATATime(void);
uint16_t CheckInfoCRCIsOK(void);
void RefreshInfoCRC(void);

#endif //__MAIN_H__
