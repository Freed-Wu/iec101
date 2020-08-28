#include "nrf905.h"
//#include "101_Protocol.h"
#include "conf_NVIC.h"
#include "conf_USART.H" //ԭ��
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
�¶�ģ���ַ 01 02 03
����ң������ַ 0x20
����ģ���ַ 61 62 63
һ֡����12���ֽ�
���ݸ�ʽ���Ϊ 		68 08 08 68 FF FF FF FF 01 00 SUM 16 ģ���ϴ�
					68 08 08 68 FF FF FF FF 01 00 SUM 16  Ӧ��֡

*/
/*Ӧ��֡   68 L L 68 ctrl id id id id addr data crc crc 16*/

const unsigned char RFConf[11] = {0x00, 0x6C, 0x0C, 0x44, 0x0C, 0x06, 0xa1, 0x52, 0xa3, 0x54, 0xDB};

#define WIRELESS_DCntTop 64 //һ���ж����洢64�ֽ�����

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
extern uint8_t moduleMaskEn; // ģ�����MASK
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
�ж��е����������

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
	//ÿ�����ݴ������˳� ����ʱ���·��������ݣ�һ��������64�ֽ�

	Wireless_WrPtr = 0; //ÿ�β����ж϶��建���������жϺ����������˳��ж�
	Wireless_CNT = 0;
	Wireless_RdPtr = 0;

	if (GPIO_ReadInputDataBit(NRF905_DR, NRF905_DR_PIN) != 0) //WAIT FOR DR = 1
	{
		GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_RESET); //clear TRX_CE = 0 ��ֹ���ͺͽ���
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
		USART1_SendData((char*)WIRELESS_Rxd, strlen((char*)WIRELESS_Rxd)); //��ʾ�յ�������
		GPIO_WriteBit(NRF905_TXEN, NRF905_TXEN_PIN, Bit_RESET);
		GPIO_WriteBit(NRF905_TRCE, NRF905_TRCE_PIN, Bit_SET); //�������
		GPIO_WriteBit(NRF905_CSN, NRF905_CSN_PIN, Bit_SET); //CSN = 0
	}

	//����Ӧ������ ack to wireless
	while (Wireless_CNT >= 12) {
		//�������� ģ��������������
		if ((WIRELESS_Rxd[Wireless_RdPtr] == 0x68) && (WIRELESS_Rxd[Wireless_RdPtr + 3] == 0x68) //����ͷ
			&& (WIRELESS_Rxd[Wireless_RdPtr + 1] == 0x06) && (WIRELESS_Rxd[Wireless_RdPtr + 2] == 0x06) //���ݳ���
			&& (WIRELESS_Rxd[Wireless_RdPtr + 4] == user_Set.ModuleID[0]) && (WIRELESS_Rxd[(Wireless_RdPtr + 5)] == user_Set.ModuleID[1]) && (WIRELESS_Rxd[Wireless_RdPtr + 6] == user_Set.ModuleID[2])
			&& (WIRELESS_Rxd[Wireless_RdPtr + 7] == user_Set.ModuleID[3]) && (WIRELESS_Rxd[Wireless_RdPtr + 11] == 0x16) //����β
			&& ((uint8_t)(WIRELESS_Rxd[Wireless_RdPtr + 4] + WIRELESS_Rxd[Wireless_RdPtr + 5] + WIRELESS_Rxd[Wireless_RdPtr + 6] + WIRELESS_Rxd[Wireless_RdPtr + 7] + WIRELESS_Rxd[Wireless_RdPtr + 8]
						  + WIRELESS_Rxd[Wireless_RdPtr + 9])
				== WIRELESS_Rxd[Wireless_RdPtr + 10])

		) { //У��ͨ��
			/*
						����Ӧ��֡
			*/
			tst_receiveCNT++;
			pAddr = WIRELESS_Rxd[Wireless_RdPtr + 8]; //��ַ �����¶�
			pData = WIRELESS_Rxd[Wireless_RdPtr + 9]; //����
			if ((pAddr & 0xf0) == 0x00) //����ģ��
			{
				WIRELESS_TxBuffer[0] = 0xAA;
				WIRELESS_TxBuffer[1] = user_Set.ModuleID[0];
				WIRELESS_TxBuffer[2] = user_Set.ModuleID[1];
				WIRELESS_TxBuffer[3] = user_Set.ModuleID[2];
				WIRELESS_TxBuffer[4] = user_Set.ModuleID[3];
				WIRELESS_TxBuffer[5] = (pAddr << 0x4) + 0xf; //(WIRELESS_Rxd[(Wireless_RdPtr + 5)%WIRELESS_DCntTop]&0xf0) + 0x0f;
				TxPacket();
			}
			else if ((pAddr & 0xf0) == 0x20) //ң�������ź�
			{
				if (pData == 0x53) {
					WIRELESS_TxBuffer[0] = 0xA6;
					WIRELESS_TxBuffer[1] = 0xFF;
					WIRELESS_TxBuffer[2] = 0xFF;
					WIRELESS_TxBuffer[3] = 0xFF;
					WIRELESS_TxBuffer[4] = 0xFF;
					WIRELESS_TxBuffer[5] = 0x53;
					moduleMaskEn = 1;
					moduleMaskDly = 50 * 60 * 60 * 24; //24Сʱ�Զ��ָ�
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
							��������
			*/
			ReadDATATime();
			switch (pAddr) { //ͨ����ַ����ʾ��ʲô�豸
			case 0x01:
			case 0x02:
			case 0x03: //����A //����B //����C
				if (pData == 0x01) {
					if (moduleMaskEn == 0) //������״̬��ʱ����״̬
						//ChangeUpdate(pAddr, 0x01, &Tx_Time);
						if (CheckInfoCRCIsOK() == 0)
							FLASH_RD_Module_Status();
					Info[pAddr - 1] = 0x01;
					InfoDisConnectDelay[pAddr - 1] = 50 * 60 * 60 * 12;
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
				}
				else if (pData == 0x02) {
					if (moduleMaskEn == 0) //������״̬��ʱ����״̬
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
			case 0x07: //©��1
				if (pData == 0x01) {
					if (moduleMaskEn == 0) //������״̬��ʱ����״̬
						//ChangeUpdate(pAddr + 3, 0x01, &Tx_Time);
						if (CheckInfoCRCIsOK() == 0)
							FLASH_RD_Module_Status();
					Info[pAddr + 6] = 0x01; //©���洢��Info�е�6 7 8 9
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
				}
				else if (pData == 0x02) {
					if (moduleMaskEn == 0) //������״̬��ʱ����״̬
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

				if ((int8_t)pData < 125 && (int8_t)pData > (int8_t)(-85)) { //�¶�����Ч��Χ��
					InfoTemp[pAddr - 0x61] = pData;
					TempDisConnectDelay[pAddr - 0x61] = 50 * 60 * 60 * 12;
					if (moduleMaskEn == 0) //������״̬��ʱ����״̬
						//TempChangeUpdate(pAddr - 0x60, InfoTemp[pAddr - 0x61], &Tx_Time); //�¶�ֵͻ���ϴ�
						temp_wr_flash_flag = 1;
				}

				break;
			default:
				break;
			} //switch
			Wireless_CNT = Wireless_CNT - 12;
			Wireless_RdPtr = Wireless_RdPtr + 12;
		} //У��ͨ��

		//ң����������������
		else if ((WIRELESS_Rxd[Wireless_RdPtr] == 0x68) && (WIRELESS_Rxd[Wireless_RdPtr + 3] == 0x68) //����ͷ
				 && (WIRELESS_Rxd[Wireless_RdPtr + 1] == 0x06) && (WIRELESS_Rxd[Wireless_RdPtr + 2] == 0x06) //���ݳ���
				 && (WIRELESS_Rxd[Wireless_RdPtr + 4] == 0XFF) && (WIRELESS_Rxd[(Wireless_RdPtr + 5)] == 0XFF) && (WIRELESS_Rxd[Wireless_RdPtr + 6] == 0XFF)
				 && (WIRELESS_Rxd[Wireless_RdPtr + 7] == 0XFF) && (WIRELESS_Rxd[Wireless_RdPtr + 11] == 0x16) //����β
				 && ((uint8_t)(WIRELESS_Rxd[Wireless_RdPtr + 4] + WIRELESS_Rxd[Wireless_RdPtr + 5] + WIRELESS_Rxd[Wireless_RdPtr + 6] + WIRELESS_Rxd[Wireless_RdPtr + 7]
							   + WIRELESS_Rxd[Wireless_RdPtr + 8] + WIRELESS_Rxd[Wireless_RdPtr + 9])
					 == WIRELESS_Rxd[Wireless_RdPtr + 10])) { //У��ͨ��
			/*
						����Ӧ��֡
			*/
			pAddr = WIRELESS_Rxd[Wireless_RdPtr + 8]; //��ַ �����¶�
			pData = WIRELESS_Rxd[Wireless_RdPtr + 9]; //����
			if ((pAddr & 0xf0) == 0x20) //ң�������ź�
			{
				if (pData == 0x53) {
					WIRELESS_TxBuffer[0] = 0xA6;
					WIRELESS_TxBuffer[1] = 0xFF;
					WIRELESS_TxBuffer[2] = 0xFF;
					WIRELESS_TxBuffer[3] = 0xFF;
					WIRELESS_TxBuffer[4] = 0xFF;
					WIRELESS_TxBuffer[5] = 0x53;
					moduleMaskEn = 1;
					moduleMaskDly = 50 * 60 * 60 * 24; //24Сʱ�Զ��ָ�
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
		} //ң�����ν���
		else {
			Wireless_CNT = Wireless_CNT - 1; //�����һλ
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
