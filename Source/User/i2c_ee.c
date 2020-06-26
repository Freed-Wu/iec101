/*********�ļ�����i2c_ee.c**********/

#include "i2c_ee.h"

enum ENUM_TWI_REPLY { TWI_NACK = 0, TWI_ACK = 1 };

enum ENUM_TWI_BUS_STATE { TWI_READY = 0, TWI_BUS_BUSY = 1, TWI_BUS_ERROR = 2 };

void I2C_EE_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// Configure I2C1 pins: SCL and SDA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void TWI_delay(void) {
	unsigned char i = 10; //i=10��ʱ1.5us//��������Ż��ٶ� ����������͵�5����д��
	while (i--)
		;
}
/**************************************************************************
 ��ʱ
 ms����ʱ�ĺ�����
 CYCLECOUNTER / 72000000
***************************************************************************/
void DelayMs(unsigned short ms) {
	unsigned short iq0;
	unsigned short iq1;
	for (iq0 = ms; iq0 > 0; iq0--) {
		for (iq1 = 11998; iq1 > 0; iq1--)
			; // ( (6*iq1+9)*iq0+15 ) / 72000000
	}
}

unsigned char TWI_Start(void) {
	SDAH;
	SCLH;
	TWI_delay();
	if (!SDAread)
		return TWI_BUS_BUSY; //SDA��Ϊ�͵�ƽ������æ,�˳�
	SDAL;
	TWI_delay();
	if (SDAread)
		return TWI_BUS_ERROR; //SDA��Ϊ�ߵ�ƽ�����߳���,�˳�
	SCLL;
	TWI_delay();
	return TWI_READY;
}

/*void TWI_Stop(void)
 {
  SCLL;
  TWI_delay();
  SDAL;
  TWI_delay();
  SCLH;
  TWI_delay();
  SDAH;
  TWI_delay();
 }*/
void TWI_Stop(void) {
	SDAL;
	SCLL;
	TWI_delay();
	SCLH;
	TWI_delay();
	SDAH;
	TWI_delay();
}

void TWI_Ack(void) {
	SCLL;
	TWI_delay();
	SDAL;
	TWI_delay();
	SCLH;
	TWI_delay();
	SCLL;
	TWI_delay();
}

void TWI_NoAck(void) {
	SCLL;
	TWI_delay();
	SDAH;
	TWI_delay();
	SCLH;
	TWI_delay();
	SCLL;
	TWI_delay();
}

unsigned char TWI_WaitAck(void) //����Ϊ:=1��ACK,=0��ACK
{
	SCLL;
	TWI_delay();
	SDAH;
	TWI_delay();
	SCLH;
	TWI_delay();
	if (SDAread) {
		SCLL;
		return 0;
	}
	SCLL;
	return 1;
}

void TWI_SendByte(unsigned char SendByte) //���ݴӸ�λ����λ//
{
	unsigned char i = 8;
	while (i--) {
		SCLL;
		TWI_delay();
		if (SendByte & 0x80)
			SDAH;
		else
			SDAL;
		SendByte <<= 1;
		TWI_delay();
		SCLH;
		TWI_delay();
	}
	SCLL;
}

unsigned char TWI_ReceiveByte(void) //���ݴӸ�λ����λ//
{
	unsigned char i = 8;
	unsigned char ReceiveByte = 0;

	SDAH;
	while (i--) {
		ReceiveByte <<= 1;
		SCLL;
		TWI_delay();
		SCLH;
		TWI_delay();
		if (SDAread) {
			ReceiveByte |= 0x01;
		}
	}
	SCLL;
	return ReceiveByte;
}

//���أ�3д��ɹ���0д������ַ����1����æ��2����
//д��1�ֽ�����           SendByte����д������    WriteAddress����д���ַ
unsigned char TWI_WriteByte(unsigned char SendByte, unsigned char WriteAddress) {
	unsigned char i;
	//unsigned short j;
	i = TWI_Start();
	if (i)
		return i;

	TWI_SendByte(ADDR_24CXX & 0xFE); //д������ַ  д�룺��ַ���λ��0����ȡ����ַ���λ��1

	if (!TWI_WaitAck()) {
		TWI_Stop();
		return 0;
	}

	TWI_SendByte(WriteAddress); //������ʼ��ַ
	TWI_WaitAck();
	TWI_SendByte(SendByte); //д����
	TWI_WaitAck();
	TWI_Stop();
	//ע�⣺��Ϊ����Ҫ�ȴ�EEPROMд�꣬���Բ��ò�ѯ����ʱ��ʽ(10ms)
	DelayMs(12); //д����ʱ 12ms  д���ڴ���10ms����
	return 3;
}

//���أ�0д������ַ����1����æ��2����,
//����1�ֽ�����
//ReadAddress����������ַ
unsigned char TWI_ReadByte(unsigned char ReadAddress) {
	unsigned char i, temp;
	i = TWI_Start();
	if (i)
		return i;

	TWI_SendByte((ADDR_24CXX & 0xFE)); //д������ַ����ִ��һ��αд����
	if (!TWI_WaitAck()) {
		TWI_Stop();
		return 0;
	}

	TWI_SendByte(ReadAddress); //������ʼ��ַ
	TWI_WaitAck();
	TWI_Start();
	TWI_SendByte((ADDR_24CXX & 0xFE) | 0x01); //��������ַ    д�룺��ַ���λ��0����ȡ����ַ���λ��1
	TWI_WaitAck();

	//*pDat = TWI_ReceiveByte();
	temp = TWI_ReceiveByte();

	TWI_NoAck();

	TWI_Stop();
	return temp; //���ص������0��1��2������������ͬ�ˣ��ٿ���һ��
}

/***************************************************************************
 ��24c256��д����ֽ�
 psrc_data��ָ��Ҫд�����������ָ��
 adr��24c256��Ҫд�����ݵ��׵�ַ
 nbyte��д����ֽ���
 ����ֵ:  0��ִ����ϣ�1��ִ�г��ִ���
 �β��У�C02ֻ��һ����ַadr��C256���и�λ��ַhadr�͵�λ��ַladr
 ***************************************************************************/
unsigned char I2C_EE_BufferWrite(unsigned char* psrc_data, unsigned char adr, unsigned char nbyte) {
	unsigned char i;

	for (; nbyte != 0; nbyte--) {
		i = TWI_Start();
		if (i)
			return i;

		TWI_SendByte(ADDR_24CXX & 0xFE); //д������ַ

		if (!TWI_WaitAck()) {
			TWI_Stop();
			return 0;
		}

		TWI_SendByte(adr); //������ʼ��ַ
		TWI_WaitAck();
		TWI_SendByte(*psrc_data); //д����
		TWI_WaitAck();
		psrc_data++; //ָ���д���ݵ�ָ���1
		adr++; //��24C08�Ĳ�����ַ��1
		TWI_Stop();
		//ע�⣺��Ϊ����Ҫ�ȴ�EEPROMд�꣬���Բ��ò�ѯ����ʱ��ʽ(10ms)
		DelayMs(12); //д����ʱ 12ms  д���ڴ���10ms����
	}
	return 0;
}

/***************************************************************************
 ��24c02������ֽ�
 pdin_data��ָ��Ҫ����������ݵ������ָ��
 adr��24c02��Ҫ�������ݵ��׵�ַ
 nbyte���������ֽ���
 ����ֵ:  0��ִ����ϣ�1��ִ�г��ִ���
 ***************************************************************************/
unsigned char I2C_EE_BufferRead(unsigned char* pdin_data, unsigned char adr, unsigned char nbyte) {
	unsigned char i;
	i = TWI_Start();
	if (i)
		return i;

	TWI_SendByte((ADDR_24CXX & 0xFE)); //д������ַ����ִ��һ��αд����
	if (!TWI_WaitAck()) {
		TWI_Stop();
		return 0;
	}

	TWI_SendByte(adr); //������ʼ��ַ
	TWI_WaitAck();
	TWI_Start();
	TWI_SendByte((ADDR_24CXX & 0xFE) | 0x01); //��������ַ    д�룺��ַ���λ��0����ȡ����ַ���λ��1
	TWI_WaitAck();

	while (nbyte != 1) //����ǰ(nbyte-1)���ֽ�
	{
		*pdin_data = TWI_ReceiveByte(); //ѭ����24C02�ж����ݣ�����pdin_data��ָ�Ĵ洢����
		TWI_Ack(); //IICӦ��
		pdin_data++; //ָ��洢�������ݵĴ洢��ָ���1
		nbyte--; //ʣ��Ҫ������ֽڼ�1
	};

	*pdin_data = TWI_ReceiveByte(); //�������һ���ֽ�
	TWI_NoAck(); //IIC��Ӧ�����

	TWI_Stop();

	return 0;
}

/*
void TWI_24CXX_Write(uint8_t* pDat, uint8_t nAddr, uint8_t nLen) {
	uint16_t i;
	for (i = 0; i < nLen; i++) {
		TWI_WriteByte(*(pDat + i), nAddr + i);
	}
}
void TWI_24CXX_Read(uint8_t* pDat, uint8_t nAddr, uint8_t nLen) {
	uint16_t i;
	for (i = 0; i < nLen; i++)
		*(pDat + i) = TWI_ReadByte(nAddr + i);
}
*/
