

#ifndef __MODBUS_H__
#define __MODBUS_H__


#include "stm32f10x.h"


#define MODBUS_LED_IDLE 			0
#define MODBUS_LED_START 			1
#define MODBUS_LED_CMD_ERROR 		3
#define MODBUS_LED_CMD_TO 		4
#define MODBUS_LED_DATA 			5
#define MODBUS_LED_DATA_OK 		6
#define MODBUS_LED_DATA_ERROR 	7
#define MODBUS_LED_DATA_TO 		8


#define  MODBUS_RUN_Txdata 0


#define MODBUS_TX_BUF_NUM		8
#define MODBUS_STAT_BUF_NUM		32


typedef struct{
	uint16_t Num;
	uint16_t PtrIn;
	uint16_t PtrOut;
	uint16_t Buf[MODBUS_STAT_BUF_NUM];
}ModbusStatBuf;


#define MODBUS_YX_DL_A	0
#define MODBUS_YX_DL_B	1
#define MODBUS_YX_DL_C	2
#define MODBUS_YX_BAT	3

#define MODBUS_YX_DL_DISCONNECT_A	4
#define MODBUS_YX_DL_DISCONNECT_B	5
#define MODBUS_YX_DL_DISCONNECT_C	6
#define MODBUS_YX_TEMP_DISCONNECT_A	7
#define MODBUS_YX_TEMP_DISCONNECT_B	8
#define MODBUS_YX_TEMP_DISCONNECT_C	9

#define MODBUS_YC_TEMP_A	0
#define MODBUS_YC_TEMP_B	1
#define MODBUS_YC_TEMP_C	2

typedef struct {
	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t Hour;
	uint8_t Min;
	uint8_t Sec;
	uint16_t MilliSec;
} TimeStructure;


void LED_Toggle(void);

void CSD_485_COM_InitRXbuf(void);
void Supervise_CSD_485_COM(void);
void CSD_485_UARTIsp(void);
uint8_t DefinePDP(void);
uint8_t ConnectToTCP(void);
uint8_t Send_AT_Command(uint8_t *pCmd,uint8_t WaitTime);
uint16_t ReceiveDataFromModbus(uint8_t *pRecBuffer);
void SendDataToGPRSbuf(char *pDataBuf,uint16_t pLength);
uint16_t ModbusGetStatBuf(void);
void ModbusLoadStatBuf(uint16_t pDataBuf);
void AddAnalogDa(uint8_t adr,uint16_t da);
void SetB(uint16_t adr);
void ClrB(uint16_t adr);

#endif //__MG301_H__



