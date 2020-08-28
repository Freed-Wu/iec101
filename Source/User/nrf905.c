#include "nrf905.h"
//#include "101_Protocol.h"
#include "conf_NVIC.h"
#include "conf_USART.H" //原无
#include "conf_USART.h"
#include "conf_sys.h"
#include "conf_usart.h"
#include "ds3231.h"
#include "flash.h"
#include "fun.h"
#include "i2c_ee.h"
#include "main.h"
#include "modbus.h"
#include "mydef.h"
#include "stm32f10x.h"

/*
温度模块地址 01 02 03
屏蔽遥控器地址 0x20
跌落模块地址 61 62 63
一帧数据12个字节
数据格式变更为 		68 08 08 68 FF FF FF FF 01 00 SUM 16 模块上传
					68 08 08 68 FF FF FF FF 01 00 SUM 16  应答帧

*/
/*应答帧   68 L L 68 ctrl id id id id addr data crc crc 16*/

const unsigned char RFConf[11] = {0x00, 0x6C, 0x0C, 0x44, 0x0C, 0x06, 0xa1, 0x52, 0xa3, 0x54, 0xDB};

#define WIRELESS_DCntTop 64 //一次中断最多存储64字节数据

unsigned char WIRELESS_Rxd[WIRELESS_DCntTop]; //unsigned char WIRELESS_Rxd[64];
uint16_t Wireless_WrPtr;
uint16_t Wireless_RdPtr;
uint16_t Wireless_CNT;
static unsigned char WIRELESS_TxBuffer[16];
static unsigned char WIRELESS_RxBuffer[16];

extern uint16_t DebugNRF905Dly;
extern DEVICE_SET user_Set;
extern TimeStructure Tx_Time; //main
extern uint16_t info_wr_flash_flag;
extern uint16_t temp_wr_flash_flag;
extern uint8_t moduleMaskEn; // 模块故障MASK
extern uint32_t moduleMaskDly;
extern uint8_t InfoTemp[8];
extern uint8_t Info[16];
extern uint32_t InfoDisConnectDelay[3];
extern uint32_t TempDisConnectDelay[3];

/*

*/
void SpiWrite(unsigned char b) {
	unsigned char i = 8;

	if (DebugNRF905Dly)
		USART1_SendCharToRS232(b);
	while (i--) {
		Delay(10);
		GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET); //CLK = 0
		if (b & 0x80)
			GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_SET); //SMDI
		else
			GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET); //SMDI
		b <<= 1;
		Delay(10);
		GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET); //CLK = 1
		Delay(10);

		GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET); //CLK = 0
	}

	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET); //clk = 0
	Delay(10);
}

unsigned char SpiRead(void) {
	unsigned char i = 8;
	unsigned char ddata = 0;
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET); //SMDI
	while (i--) {
		ddata <<= 1;
		GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET); //CLK = 0
		Delay(10);
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)) //SMDO
			ddata |= 0x01;
		GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET); //CLK = 1
		Delay(10);
	}
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET); //CLK = 0
	Delay(10);
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_SET); //SMDI

	if (DebugNRF905Dly)
		USART1_SendCharToRS232(ddata);
	return ddata;
}
/*

*/
void SPI_RdConfiRegs(unsigned char cmd, int length) {
	int i;
	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_RESET); //CSN = 0
	Delay(10);
	if (DebugNRF905Dly) {
		USART1_SendCharToRS232(0xaa);
		USART1_SendCharToRS232(0xaa);
		USART1_SendCharToRS232(0xaa);
		USART1_SendCharToRS232(0xaa);
	}
	SpiWrite(cmd);

	for (i = 0; i < length; i++) //read tx address
		WIRELESS_RxBuffer[i] = SpiRead();

	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET); //CSN = 1
	Delay(10);
}

void TxPacket(void) {
	int i, j;
	int data;
	//  while(GPIO_ReadInputDataBit(NRF905_CD, NRF905_CD_PIN) != 0){;}
	if (DebugNRF905Dly) {
		USART1_SendCharToRS232(0x22);
		USART1_SendCharToRS232(0x22);
		USART1_SendCharToRS232(0x22);
		USART1_SendCharToRS232(0x22);
	}
	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET); //CSN = 0
	Delay(10000);
	GPIO_WriteBit(NRF905_TXEN, NRF905_TXEN_PIN, Bit_SET); //TX_EN = 1
	GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_RESET); //TRX_CE = 0
	Delay(10);
	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_RESET); //CSN = 0
	// Delay(10);
	SpiWrite(0x20); //WRITE TX DATA COMMAND
	SpiWrite(WIRELESS_TxBuffer[0]); //WRITE TX DATA COMMAND
	SpiWrite(WIRELESS_TxBuffer[1]); //WRITE TX DATA COMMAND
	SpiWrite(WIRELESS_TxBuffer[2]); //WRITE TX DATA COMMAND
	SpiWrite(WIRELESS_TxBuffer[3]); //WRITE TX DATA COMMAND
	SpiWrite(WIRELESS_TxBuffer[4]); //WRITE TX DATA COMMAND
	SpiWrite(WIRELESS_TxBuffer[5]);
	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET); //CSN = 1
	Delay(300);
	GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_SET); //TRX_CE = 1
	Delay(100);
	GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_RESET); //TRX_CE = 0
	i = 0;
	j = 0;
	data = GPIO_ReadInputDataBit(NRF905_DR, NRF905_DR_PIN);
	while (data == 0) {
		data = GPIO_ReadInputDataBit(NRF905_DR, NRF905_DR_PIN);
		if (j < 100) {
			if (i < 10000)
				i++;
			else {
				i = 0;
				j = j + 1;
			}
		}
		else
			break;
	}

	GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_SET); //TRX_CE = 1
	GPIO_WriteBit(NRF905_TXEN, NRF905_TXEN_PIN, Bit_RESET); //TX_EN = 1
	//  SPI_RdConfiRegs(0x21,6);
	//	Delay(500);
}
/*
中断中调用这个函数

*/
uint16_t tst_receiveCNT = 0;
void Wait_Rec_Packet(void) {
	uint8_t pAddr;
	uint8_t pData;

	if (DebugNRF905Dly) {
		USART1_SendCharToRS232(0x88);
		USART1_SendCharToRS232(0x88);
		USART1_SendCharToRS232(0x88);
		USART1_SendCharToRS232(0x88);
	}

	DisableExtINT();
	//每次数据处理完退出 进入时重新放入新数据，一次最多放入64字节

	Wireless_WrPtr = 0; //每次产生中断都清缓冲区，进中断后处理完数据退出中断
	Wireless_CNT = 0;
	Wireless_RdPtr = 0;

	if (GPIO_ReadInputDataBit(NRF905_DR, NRF905_DR_PIN) != 0) //WAIT FOR DR = 1
	{
		GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_RESET); //clear TRX_CE = 0 禁止发送和接收
		GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_RESET); //CSN = 0

		SpiWrite(0x24); //read rx buffer                                       //read RX buffer command
		while (GPIO_ReadInputDataBit(NRF905_DR, NRF905_DR_PIN) != 0) //wait for DR = 1
		{
			WIRELESS_Rxd[Wireless_WrPtr] = SpiRead();
			Wireless_WrPtr = Wireless_WrPtr + 1;
			Wireless_CNT = Wireless_CNT + 1;
			if (Wireless_CNT > 64) {
				Wireless_CNT = 0;
				PowerUp_NRF905();
				break;
			}
		}
		USART1_SendData((char*)WIRELESS_Rxd, strlen((char*)WIRELESS_Rxd)); //显示收到的数据
		GPIO_WriteBit(NRF905_TXEN, NRF905_TXEN_PIN, Bit_RESET);
		GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_SET); //允许接收
		GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET); //CSN = 0
	}

	//处理及应答数据 ack to wireless
	while (Wireless_CNT >= 12) {
		//跌落数据 模块数据在这里面
		if ((WIRELESS_Rxd[Wireless_RdPtr] == 0x68) && (WIRELESS_Rxd[Wireless_RdPtr + 3] == 0x68) //数据头
			&& (WIRELESS_Rxd[Wireless_RdPtr + 1] == 0x06) && (WIRELESS_Rxd[Wireless_RdPtr + 2] == 0x06) //数据长度
			&& (WIRELESS_Rxd[Wireless_RdPtr + 4] == user_Set.ModuleID[0]) && (WIRELESS_Rxd[(Wireless_RdPtr + 5)] == user_Set.ModuleID[1]) && (WIRELESS_Rxd[Wireless_RdPtr + 6] == user_Set.ModuleID[2])
			&& (WIRELESS_Rxd[Wireless_RdPtr + 7] == user_Set.ModuleID[3]) && (WIRELESS_Rxd[Wireless_RdPtr + 11] == 0x16) //数据尾
			&& ((uint8_t)(WIRELESS_Rxd[Wireless_RdPtr + 4] + WIRELESS_Rxd[Wireless_RdPtr + 5] + WIRELESS_Rxd[Wireless_RdPtr + 6] + WIRELESS_Rxd[Wireless_RdPtr + 7] + WIRELESS_Rxd[Wireless_RdPtr + 8]
						  + WIRELESS_Rxd[Wireless_RdPtr + 9])
				== WIRELESS_Rxd[Wireless_RdPtr + 10])

		) { //校验通过
			/*
						返回应答帧
			*/
			tst_receiveCNT++;
			pAddr = WIRELESS_Rxd[Wireless_RdPtr + 8]; //地址 跌落温度
			pData = WIRELESS_Rxd[Wireless_RdPtr + 9]; //数据
			if ((pAddr & 0xf0) == 0x00) //跌落模块
			{
				WIRELESS_TxBuffer[0] = 0xAA;
				WIRELESS_TxBuffer[1] = user_Set.ModuleID[0];
				WIRELESS_TxBuffer[2] = user_Set.ModuleID[1];
				WIRELESS_TxBuffer[3] = user_Set.ModuleID[2];
				WIRELESS_TxBuffer[4] = user_Set.ModuleID[3];
				WIRELESS_TxBuffer[5] = (pAddr << 0x4) + 0xf; //(WIRELESS_Rxd[(Wireless_RdPtr + 5)%WIRELESS_DCntTop]&0xf0) + 0x0f;
				TxPacket();
			}
			else if ((pAddr & 0xf0) == 0x20) //遥控屏蔽信号
			{
				if (pData == 0x53) {
					WIRELESS_TxBuffer[0] = 0xA6;
					WIRELESS_TxBuffer[1] = 0xFF;
					WIRELESS_TxBuffer[2] = 0xFF;
					WIRELESS_TxBuffer[3] = 0xFF;
					WIRELESS_TxBuffer[4] = 0xFF;
					WIRELESS_TxBuffer[5] = 0x53;
					moduleMaskEn = 1;
					moduleMaskDly = 50 * 60 * 60 * 24; //24小时自动恢复
					TxPacket();
				}
				else if (pAddr == 0x5c) {
					WIRELESS_TxBuffer[0] = 0xA6;
					WIRELESS_TxBuffer[1] = 0xFF;
					WIRELESS_TxBuffer[2] = 0xFF;
					WIRELESS_TxBuffer[3] = 0xFF;
					WIRELESS_TxBuffer[4] = 0xFF;
					WIRELESS_TxBuffer[5] = 0x5c;
					moduleMaskDly = 0;
					moduleMaskEn = 0;
					TxPacket();
				}
			}
			/*
							处理数据
			*/
			ReadDATATime();
			switch (pAddr) { //通过地址来标示是什么设备
			case 0x01:
			case 0x02:
			case 0x03: //跌落A //跌落B //跌落C
				if (pData == 0x01) {
					if (moduleMaskEn == 0) //非屏蔽状态及时发送状态
						//ChangeUpdate(pAddr, 0x01, &Tx_Time);
						if (CheckInfoCRCIsOK() == 0)
							FLASH_RD_Module_Status();
					Info[pAddr - 1] = 0x01;
					InfoDisConnectDelay[pAddr - 1] = 50 * 60 * 60 * 12;
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
				}
				else if (pData == 0x02) {
					if (moduleMaskEn == 0) //非屏蔽状态及时发送状态
						//ChangeUpdate(pAddr, 0x00, &Tx_Time);
						if (CheckInfoCRCIsOK() == 0)
							FLASH_RD_Module_Status();
					Info[pAddr - 1] = 0x00;
					InfoDisConnectDelay[pAddr - 1] = 50 * 60 * 60 * 12;
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
				}
				break;
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07: //漏报1
				if (pData == 0x01) {
					if (moduleMaskEn == 0) //非屏蔽状态及时发送状态
						//ChangeUpdate(pAddr + 3, 0x01, &Tx_Time);
						if (CheckInfoCRCIsOK() == 0)
							FLASH_RD_Module_Status();
					Info[pAddr + 6] = 0x01; //漏保存储在Info中的6 7 8 9
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
				}
				else if (pData == 0x02) {
					if (moduleMaskEn == 0) //非屏蔽状态及时发送状态
						//ChangeUpdate(pAddr + 3, 0x00, &Tx_Time);
						if (CheckInfoCRCIsOK() == 0)
							FLASH_RD_Module_Status();
					Info[pAddr + 6] = 0x00;
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
				}
				break;

			case 0x61:
			case 0x62:
			case 0x63:

				if ((int8_t)pData < 125 && (int8_t)pData > (int8_t)(-85)) { //温度在有效范围内
					InfoTemp[pAddr - 0x61] = pData;
					TempDisConnectDelay[pAddr - 0x61] = 50 * 60 * 60 * 12;
					if (moduleMaskEn == 0) //非屏蔽状态及时发送状态
						//TempChangeUpdate(pAddr - 0x60, InfoTemp[pAddr - 0x61], &Tx_Time); //温度值突发上传
						temp_wr_flash_flag = 1;
				}

				break;
			default:
				break;
			} //switch
			Wireless_CNT = Wireless_CNT - 12;
			Wireless_RdPtr = Wireless_RdPtr + 12;
		} //校验通过

		//遥控屏蔽数据在这里
		else if ((WIRELESS_Rxd[Wireless_RdPtr] == 0x68) && (WIRELESS_Rxd[Wireless_RdPtr + 3] == 0x68) //数据头
				 && (WIRELESS_Rxd[Wireless_RdPtr + 1] == 0x06) && (WIRELESS_Rxd[Wireless_RdPtr + 2] == 0x06) //数据长度
				 && (WIRELESS_Rxd[Wireless_RdPtr + 4] == 0XFF) && (WIRELESS_Rxd[(Wireless_RdPtr + 5)] == 0XFF) && (WIRELESS_Rxd[Wireless_RdPtr + 6] == 0XFF)
				 && (WIRELESS_Rxd[Wireless_RdPtr + 7] == 0XFF) && (WIRELESS_Rxd[Wireless_RdPtr + 11] == 0x16) //数据尾
				 && ((uint8_t)(WIRELESS_Rxd[Wireless_RdPtr + 4] + WIRELESS_Rxd[Wireless_RdPtr + 5] + WIRELESS_Rxd[Wireless_RdPtr + 6] + WIRELESS_Rxd[Wireless_RdPtr + 7]
							   + WIRELESS_Rxd[Wireless_RdPtr + 8] + WIRELESS_Rxd[Wireless_RdPtr + 9])
					 == WIRELESS_Rxd[Wireless_RdPtr + 10])) { //校验通过
			/*
						返回应答帧
			*/
			pAddr = WIRELESS_Rxd[Wireless_RdPtr + 8]; //地址 跌落温度
			pData = WIRELESS_Rxd[Wireless_RdPtr + 9]; //数据
			if ((pAddr & 0xf0) == 0x20) //遥控屏蔽信号
			{
				if (pData == 0x53) {
					WIRELESS_TxBuffer[0] = 0xA6;
					WIRELESS_TxBuffer[1] = 0xFF;
					WIRELESS_TxBuffer[2] = 0xFF;
					WIRELESS_TxBuffer[3] = 0xFF;
					WIRELESS_TxBuffer[4] = 0xFF;
					WIRELESS_TxBuffer[5] = 0x53;
					moduleMaskEn = 1;
					moduleMaskDly = 50 * 60 * 60 * 24; //24小时自动恢复
					TxPacket();
				}
				else if (pAddr == 0x5c) {
					WIRELESS_TxBuffer[0] = 0xA6;
					WIRELESS_TxBuffer[1] = 0xFF;
					WIRELESS_TxBuffer[2] = 0xFF;
					WIRELESS_TxBuffer[3] = 0xFF;
					WIRELESS_TxBuffer[4] = 0xFF;
					WIRELESS_TxBuffer[5] = 0x5c;
					moduleMaskDly = 0;
					moduleMaskEn = 0;
					TxPacket();
				}
			}
			Wireless_CNT = Wireless_CNT - 12;
			Wireless_RdPtr = Wireless_RdPtr + 12;
		} //遥控屏蔽结束
		else {
			Wireless_CNT = Wireless_CNT - 1; //向后移一位
			Wireless_RdPtr = Wireless_RdPtr + 1;
		}

	} //while(1)

	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET); //CSN = 1
	GPIO_WriteBit(NRF905_TXEN, NRF905_TXEN_PIN, Bit_RESET); //TX_EN = 0
	GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_SET); //TRX_CE = 1
	EnableExtINT();
}

void PowerUp_NRF905(void) {
	int i;

	GPIO_WriteBit(NRF905_PWUP, NRF905_PWUP_PIN, Bit_RESET);
	if (DebugNRF905Dly) {
		USART1_SendCharToRS232(0x11);
		USART1_SendCharToRS232(0x11);
		USART1_SendCharToRS232(0x11);
		USART1_SendCharToRS232(0x11);
	}
	Delay(10000);
	GPIO_WriteBit(NRF905_PWUP, NRF905_PWUP_PIN, Bit_SET);
	Delay(10000);
	GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_RESET);
	GPIO_WriteBit(NRF905_TXEN, NRF905_TXEN_PIN, Bit_RESET); //TX_EN = 0
	Delay(1);
	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET);
	Delay(10000);
	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_RESET);

	for (i = 0; i < 11; i++) {
		SpiWrite(RFConf[i]);
		Delay(10);
	}
	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET);
	Delay(10); //transmit address
	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_RESET); //CSN = 0

	SpiWrite(0x22); //WRITE TX ADDRESS CMD
	SpiWrite(0xa1); //TX ADDRESS
	SpiWrite(0x51);
	SpiWrite(0xa8);
	SpiWrite(0x58);

	GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET); //CSN = 1
	GPIO_WriteBit(NRF905_TXEN, NRF905_TXEN_PIN, Bit_RESET); //TX_EN = 0
	GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_SET); //TRX_CE = 1
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
uint16_t reccnt = 0;
//
unsigned int checknrf905_conf(void) {
	SPI_RdConfiRegs(0x10, 10); //read configure registers
	return !strncmp((const char*)WIRELESS_RxBuffer + 1, (const char*)RFConf + 2, 9);
}

unsigned int checknrf905_addr(void) {
	SPI_RdConfiRegs(0x23, 4); //read tx address
	return !strncmp((const char*)WIRELESS_RxBuffer, "\xa1\x51\xa8\x58", 4);
}
