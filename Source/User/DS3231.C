//#include "101_Protocol.h"
#include "i2c_ee.h"
#include "modbus.h"

TimeStructure Tx_Time;
unsigned char Time_Data[8];
unsigned char RTC_Read_data[6];

unsigned char WriteDATATime(void) {
	unsigned char xpErrCnt;
	unsigned char RetryCnt;
	unsigned char temp_time;
	xpErrCnt = 1;
	RetryCnt = 0;

	temp_time = (((Time_Data[5] / 10) << 4) & 0x70) + ((Time_Data[5] % 10) & 0x0f);
	TWI_WriteByte(temp_time, 0);
	temp_time = (((Time_Data[4] / 10) << 4) & 0x70) + ((Time_Data[4] % 10) & 0x0f);
	TWI_WriteByte(temp_time, 1);
	temp_time = (((Time_Data[3] / 10) << 4) & 0x30) + ((Time_Data[3] % 10) & 0x0f);
	TWI_WriteByte(temp_time, 2);
	temp_time = (((Time_Data[2] / 10) << 4) & 0x30) + ((Time_Data[2] % 10) & 0x0f);
	TWI_WriteByte(temp_time, 4);
	temp_time = (((Time_Data[1] / 10) << 4) & 0x10) + ((Time_Data[1] % 10) & 0x0f);
	TWI_WriteByte(temp_time, 5);
	temp_time = (((Time_Data[0] / 10) << 4) & 0xf0) + ((Time_Data[0] % 10) & 0x0f);
	TWI_WriteByte(temp_time, 6);

	if (RetryCnt <= 10) {
		xpErrCnt = 0;
	}
	else {
		xpErrCnt = 1;
	}
	return (xpErrCnt);
}

unsigned char ReadDATATime(void) {
	unsigned char xpErrCnt;
	unsigned char RetryCnt;
	//unsigned char RTC_Read_data[6];
	xpErrCnt = 1;
	RetryCnt = 0;

	RTC_Read_data[0] = TWI_ReadByte(0); //seconds
	RTC_Read_data[1] = TWI_ReadByte(1); //Minutes
	RTC_Read_data[2] = TWI_ReadByte(2); //Hours
	RTC_Read_data[3] = TWI_ReadByte(4); //Date
	RTC_Read_data[4] = TWI_ReadByte(5); //Month
	RTC_Read_data[5] = TWI_ReadByte(6); //Year

	Tx_Time.Year = ((RTC_Read_data[5] >> 4) & 0x0f) * 10 + (RTC_Read_data[5] & 0x0f);
	Tx_Time.Month = ((RTC_Read_data[4] >> 4) & 0x01) * 10 + (RTC_Read_data[4] & 0x0f);
	Tx_Time.Day = ((RTC_Read_data[3] >> 4) & 0x03) * 10 + (RTC_Read_data[3] & 0x0f);
	Tx_Time.Hour = ((RTC_Read_data[2] >> 4) & 0x03) * 10 + (RTC_Read_data[2] & 0x0f);
	Tx_Time.Min = ((RTC_Read_data[1] >> 4) & 0x0f) * 10 + (RTC_Read_data[1] & 0x0f);
	Tx_Time.Sec = ((RTC_Read_data[0] >> 4) & 0x0f) * 10 + (RTC_Read_data[0] & 0x0f);
	Tx_Time.MilliSec = 0;

	if (RetryCnt <= 10) {
		xpErrCnt = 0;
	}
	else {
		xpErrCnt = 1;
	}
	return (xpErrCnt);
}
