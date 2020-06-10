#include "conf_USART.H"
#include "ds3231.h"
#include "flash.h"
#include "fun.h"
#include "i2c_ee.h"
#include "main.h"
#include "stm32f10x.h"
#include "string.h"

void InitAllPara(void);

char const up_test_prt_data[] = "---设置信息---";
const char up_test_sn_data[] = "序列号:";
const char up_test_time_data[] = "系统时间:";
const char up_test_ip_data[] = "IP地址:";
const char up_test_port_data[] = "端口号:";
const char up_test_addr_data[] = "101规约地址:";
const char up_test_apn_data[] = "APN:";
const char up_test_user_data[] = "用户名:";
const char up_test_password_data[] = "密码:";
const char up_test_heartpackage_data[] = "心跳包数据:";
const char up_test_hearttime_data[] = "心跳包间隔时间:";

extern unsigned char RTC_Read_data[6];
extern uint32_t HeartTime; //定义心跳包时间默认值为30s
extern uint16_t ReqGPRSConfigflg;
extern unsigned char RS232_REC_Flag;
extern unsigned char RS232_buff[RS232_REC_BUFF_SIZE]; //用于接收数据

extern uint16_t LINK_ADDRESS; //定义链路地址
extern DEVICE_SET user_Set;

void rs232_set_process(void) {
	unsigned char SNtemp[2];
	//	char TelNum_temp[14];
	unsigned char Tx_SNtemp[8];
	//	unsigned char Set_Data_Len;
	unsigned char Set_Time_Data;
	unsigned char Tx_Timetemp[32];
	int prog_pare_flag = 0;
	uint8_t Temp[16] = {0x00};
	uint8_t HeartData[64] = {0x00};
	uint8_t HexDataLength = 0;
	uint8_t i = 0;
	uint8_t Data2, Data1;
	uint8_t templen = 0;
	if (RS232_REC_Flag == 1) {
		if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x51) //Set Serial Number
		{
			if (RS232_buff[2] == 12 && RS232_buff[3] == 'S' && RS232_buff[4] == 'N' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				if (RS232_buff[7] <= 0x39) {
					SNtemp[0] = RS232_buff[7] - 0x30;
				}
				else if (RS232_buff[7] <= 0x46) {
					SNtemp[0] = RS232_buff[7] - 55;
				}
				else {
					SNtemp[0] = RS232_buff[7] - 87;
				}
				if (RS232_buff[8] <= 0x39) {
					SNtemp[1] = RS232_buff[8] - 0x30;
				}
				else if (RS232_buff[8] <= 0x46) {
					SNtemp[1] = RS232_buff[8] - 55;
				}
				else {
					SNtemp[1] = RS232_buff[8] - 87;
				}
				user_Set.ModuleID[0] = SNtemp[0] * 0x10 + SNtemp[1];

				if (RS232_buff[9] <= 0x39) {
					SNtemp[0] = RS232_buff[9] - 0x30;
				}
				else if (RS232_buff[9] <= 0x46) {
					SNtemp[0] = RS232_buff[9] - 55;
				}
				else {
					SNtemp[0] = RS232_buff[9] - 87;
				}
				if (RS232_buff[10] <= 0x39) {
					SNtemp[1] = RS232_buff[10] - 0x30;
				}
				else if (RS232_buff[10] <= 0x46) {
					SNtemp[1] = RS232_buff[10] - 55;
				}
				else {
					SNtemp[1] = RS232_buff[10] - 87;
				}
				user_Set.ModuleID[1] = SNtemp[0] * 0x10 + SNtemp[1];

				if (RS232_buff[11] <= 0x39) {
					SNtemp[0] = RS232_buff[11] - 0x30;
				}
				else if (RS232_buff[11] <= 0x46) {
					SNtemp[0] = RS232_buff[11] - 55;
				}
				else {
					SNtemp[0] = RS232_buff[8] - 87; //数组越界
				}
				if (RS232_buff[12] <= 0x39) {
					SNtemp[1] = RS232_buff[12] - 0x30;
				}
				else if (RS232_buff[12] <= 0x46) {
					SNtemp[1] = RS232_buff[12] - 55;
				}
				else {
					SNtemp[1] = RS232_buff[12] - 87;
				}
				user_Set.ModuleID[2] = SNtemp[0] * 0x10 + SNtemp[1];

				if (RS232_buff[13] <= 0x39) {
					SNtemp[0] = RS232_buff[13] - 0x30;
				}
				else if (RS232_buff[13] <= 0x46) {
					SNtemp[0] = RS232_buff[13] - 55;
				}
				else {
					SNtemp[0] = RS232_buff[13] - 87;
				}
				if (RS232_buff[14] <= 0x39) {
					SNtemp[1] = RS232_buff[14] - 0x30;
				}
				else if (RS232_buff[14] <= 0x46) {
					SNtemp[1] = RS232_buff[14] - 55;
				}
				else {
					SNtemp[1] = RS232_buff[14] - 87;
				}
				user_Set.ModuleID[3] = SNtemp[0] * 0x10 + SNtemp[1];
				prog_pare_flag = 1;
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x52) //Set IP
		{
			if (RS232_buff[3] == 'I' && RS232_buff[4] == 'P' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				user_Set.ip_len = RS232_buff[2];
				memcpy(user_Set.ip_info, &RS232_buff[7], user_Set.ip_len);
				user_Set.ip_info[user_Set.ip_len] = '\0';
				prog_pare_flag = 1;
				ReqGPRSConfigflg = 1; //重新配置101
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x53) //Set Port Number
		{
			if (RS232_buff[3] == 'P' && RS232_buff[4] == 'T' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				user_Set.port_len = RS232_buff[2];
				memset(user_Set.port_info, 0, 5);
				memcpy(user_Set.port_info, &RS232_buff[7], user_Set.port_len);
				prog_pare_flag = 1;
				ReqGPRSConfigflg = 1; //重新配置101
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x54) //Set 101 Address
		{
			if (RS232_buff[3] == 'A' && RS232_buff[4] == 'D' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				user_Set.addr_len = RS232_buff[2];
				memset(user_Set.addr_info, 0, 5);
				memcpy(user_Set.addr_info, &RS232_buff[7], user_Set.addr_len);
				prog_pare_flag = 1;
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);

				//将101地址转换为整型
				memcpy(Temp, user_Set.addr_info, user_Set.addr_len);
				LINK_ADDRESS = (uint16_t)Str2Int((char*)Temp);
				memset(Temp, 0x00, 16);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x55) //Set Time
		{
			if (RS232_buff[2] == 22 && RS232_buff[3] == 'S' && RS232_buff[4] == 'T' && RS232_buff[5] == '-') {
				Set_Time_Data = (RS232_buff[23] - 0x30) * 0x10 + (RS232_buff[24] - 0x30);
				TWI_WriteByte(Set_Time_Data, 0);
				Set_Time_Data = (RS232_buff[20] - 0x30) * 0x10 + (RS232_buff[21] - 0x30);
				TWI_WriteByte(Set_Time_Data, 1);
				Set_Time_Data = (RS232_buff[17] - 0x30) * 0x10 + (RS232_buff[18] - 0x30);
				TWI_WriteByte(Set_Time_Data, 2);
				Set_Time_Data = (RS232_buff[14] - 0x30) * 0x10 + (RS232_buff[15] - 0x30);
				TWI_WriteByte(Set_Time_Data, 4);
				Set_Time_Data = (RS232_buff[11] - 0x30) * 0x10 + (RS232_buff[12] - 0x30);
				TWI_WriteByte(Set_Time_Data, 5);
				Set_Time_Data = (RS232_buff[8] - 0x30) * 0x10 + (RS232_buff[9] - 0x30);
				TWI_WriteByte(Set_Time_Data, 6);

				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x56) //Request System Information
		{
			if (RS232_buff[2] == 3 && RS232_buff[3] == 'R' && RS232_buff[4] == 'E' && RS232_buff[5] == 'Q') {
				//DebugDly = 0;	//关闭DEBUG模式
				FLASH_ReadUserSet();
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				USART1_SendDataToRS232((unsigned char*)up_test_prt_data, 14);
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);
				//Send SNID
				USART1_SendDataToRS232((unsigned char*)up_test_sn_data, 7);
				Tx_SNtemp[0] = (user_Set.ModuleID[0] >> 4) & 0x0f;
				if (Tx_SNtemp[0] <= 9) {
					Tx_SNtemp[0] = Tx_SNtemp[0] + 0x30;
				}
				else {
					Tx_SNtemp[0] = Tx_SNtemp[0] + 55;
				}
				Tx_SNtemp[1] = user_Set.ModuleID[0] & 0x0f;
				if (Tx_SNtemp[1] <= 9) {
					Tx_SNtemp[1] = Tx_SNtemp[1] + 0x30;
				}
				else {
					Tx_SNtemp[1] = Tx_SNtemp[1] + 55;
				}
				Tx_SNtemp[2] = (user_Set.ModuleID[1] >> 4) & 0x0f;
				if (Tx_SNtemp[2] <= 9) {
					Tx_SNtemp[2] = Tx_SNtemp[2] + 0x30;
				}
				else {
					Tx_SNtemp[2] = Tx_SNtemp[2] + 55;
				}
				Tx_SNtemp[3] = user_Set.ModuleID[1] & 0x0f;
				if (Tx_SNtemp[3] <= 9) {
					Tx_SNtemp[3] = Tx_SNtemp[3] + 0x30;
				}
				else {
					Tx_SNtemp[3] = Tx_SNtemp[3] + 55;
				}
				Tx_SNtemp[4] = (user_Set.ModuleID[2] >> 4) & 0x0f;
				if (Tx_SNtemp[4] <= 9) {
					Tx_SNtemp[4] = Tx_SNtemp[4] + 0x30;
				}
				else {
					Tx_SNtemp[4] = Tx_SNtemp[4] + 55;
				}
				Tx_SNtemp[5] = user_Set.ModuleID[2] & 0x0f;
				if (Tx_SNtemp[5] <= 9) {
					Tx_SNtemp[5] = Tx_SNtemp[5] + 0x30;
				}
				else {
					Tx_SNtemp[5] = Tx_SNtemp[5] + 55;
				}
				Tx_SNtemp[6] = (user_Set.ModuleID[3] >> 4) & 0x0f;
				if (Tx_SNtemp[6] <= 9) {
					Tx_SNtemp[6] = Tx_SNtemp[6] + 0x30;
				}
				else {
					Tx_SNtemp[6] = Tx_SNtemp[6] + 55;
				}
				Tx_SNtemp[7] = user_Set.ModuleID[3] & 0x0f;
				if (Tx_SNtemp[7] <= 9) {
					Tx_SNtemp[7] = Tx_SNtemp[7] + 0x30;
				}
				else {
					Tx_SNtemp[7] = Tx_SNtemp[7] + 55;
				}
				USART1_SendDataToRS232(Tx_SNtemp, 8);
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);
				//Send Time
				USART1_SendDataToRS232((unsigned char*)up_test_time_data, 9);
				Tx_Timetemp[0] = '2';
				Tx_Timetemp[1] = '0';
				ReadDATATime();

				Tx_Timetemp[2] = (RTC_Read_data[5] >> 4) + '0';
				Tx_Timetemp[3] = (RTC_Read_data[5] & 0x0f) + '0';
				Tx_Timetemp[4] = '-';
				Tx_Timetemp[5] = (RTC_Read_data[4] >> 4) + '0';
				Tx_Timetemp[6] = (RTC_Read_data[4] & 0x0f) + '0';
				Tx_Timetemp[7] = '-';
				Tx_Timetemp[8] = (RTC_Read_data[3] >> 4) + '0';
				Tx_Timetemp[9] = (RTC_Read_data[3] & 0x0f) + '0';
				Tx_Timetemp[10] = ' ';
				Tx_Timetemp[11] = ' ';
				Tx_Timetemp[12] = (RTC_Read_data[2] >> 4) + '0';
				Tx_Timetemp[13] = (RTC_Read_data[2] & 0x0f) + '0';
				Tx_Timetemp[14] = ':';
				Tx_Timetemp[15] = (RTC_Read_data[1] >> 4) + '0';
				Tx_Timetemp[16] = (RTC_Read_data[1] & 0x0f) + '0';
				Tx_Timetemp[17] = ':';
				Tx_Timetemp[18] = (RTC_Read_data[0] >> 4) + '0';
				Tx_Timetemp[19] = (RTC_Read_data[0] & 0x0f) + '0';
				USART1_SendDataToRS232(Tx_Timetemp, 20);
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				//Send IP address
				USART1_SendDataToRS232((unsigned char*)up_test_ip_data, 7);
				if (user_Set.ip_len <= 15) {
					USART1_SendDataToRS232((unsigned char*)user_Set.ip_info, user_Set.ip_len);
				}
				else {
					USART1_SendDataToRS232("IP is not Set", 13);
				}
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				//Send Port Number
				USART1_SendDataToRS232((unsigned char*)up_test_port_data, 7);
				if (user_Set.port_len <= 5) {
					USART1_SendDataToRS232((unsigned char*)user_Set.port_info, user_Set.port_len);
				}
				else {
					USART1_SendDataToRS232("Port is not Set", 15);
				}
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				//Send 101 address
				USART1_SendDataToRS232((unsigned char*)up_test_addr_data, 12);
				if (user_Set.addr_len <= 5) {
					USART1_SendDataToRS232((unsigned char*)user_Set.addr_info, user_Set.addr_len);
				}
				else {
					USART1_SendDataToRS232("101 Address is not Set", 22);
				}
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				//Send APN
				USART1_SendDataToRS232((unsigned char*)up_test_apn_data, 4);
				if (user_Set.apn_len <= 20) {
					USART1_SendDataToRS232((unsigned char*)user_Set.apn_info, user_Set.apn_len);
				}
				else {
					USART1_SendDataToRS232("APN is not Set", 14);
				}
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				//Send USER NAME
				USART1_SendDataToRS232((unsigned char*)up_test_user_data, 7);
				if (user_Set.user_len <= 32) {
					USART1_SendDataToRS232((unsigned char*)user_Set.user_info, user_Set.user_len);
				}
				else {
					USART1_SendDataToRS232("User is not Set", 15);
				}
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				//Send PASSWORD
				USART1_SendDataToRS232((unsigned char*)up_test_password_data, 5);
				if (user_Set.password_len <= 32) {
					USART1_SendDataToRS232((unsigned char*)user_Set.password_info, user_Set.password_len);
				}
				else {
					USART1_SendDataToRS232("Password is not Set", 19);
				}
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				//Send HEART PACKAGE DATA

				USART1_SendDataToRS232((unsigned char*)up_test_heartpackage_data, 11);
				if (user_Set.heart_len <= 64) {
					char str_heart_info[64];
					hex2Str(str_heart_info, user_Set.heart_info, user_Set.heart_len);
					USART1_SendDataToRS232((unsigned char*)str_heart_info, user_Set.heart_len * 2);
				}
				else {
					USART1_SendDataToRS232("Heart Package is not Set", 24);
				}
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);

				//Send HEART PACKAGE TIME
				USART1_SendDataToRS232((unsigned char*)up_test_hearttime_data, 15);
				if (user_Set.heart_time_len <= 5) {
					USART1_SendDataToRS232((unsigned char*)user_Set.heart_time_info, user_Set.heart_time_len);
				}
				else {
					USART1_SendDataToRS232("Heart Time is not Set", 21);
				}
				USART1_SendCharToRS232(0x0d);
				USART1_SendCharToRS232(0x0a);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x57) //Set APN
		{
			if (RS232_buff[3] == 'A' && RS232_buff[4] == 'P' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				user_Set.apn_len = RS232_buff[2];
				memcpy(user_Set.apn_info, &RS232_buff[7], user_Set.apn_len);
				user_Set.apn_info[user_Set.apn_len] = '\0'; //字符串最后一个字节要赋一个'\0'
				prog_pare_flag = 1;
				ReqGPRSConfigflg = 1; //重新配置101
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x58) //Set USER
		{
			if (RS232_buff[3] == 'U' && RS232_buff[4] == 'S' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				user_Set.user_len = RS232_buff[2];
				memcpy(user_Set.user_info, &RS232_buff[7], user_Set.user_len);
				user_Set.user_info[user_Set.user_len] = '\0';
				prog_pare_flag = 1;
				ReqGPRSConfigflg = 1; //重新配置101
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x59) //Set PASSWORD
		{
			if (RS232_buff[3] == 'P' && RS232_buff[4] == 'W' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				user_Set.password_len = RS232_buff[2];
				memcpy(user_Set.password_info, &RS232_buff[7], user_Set.password_len);
				user_Set.password_info[user_Set.password_len] = '\0';
				prog_pare_flag = 1;
				ReqGPRSConfigflg = 1; //重新配置101
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x5A) //Set HEART PACKAGE
		{
			if (RS232_buff[3] == 'H' && RS232_buff[4] == 'P' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				user_Set.heart_len = RS232_buff[2];

				//将心跳包的字符型数据转换成16进制数
				memcpy(HeartData, &RS232_buff[7], user_Set.heart_len);

				HexDataLength = (user_Set.heart_len / 2);
				while (HexDataLength > 0) {
					if ((HeartData[i] >= '0') && (HeartData[i] <= '9')) {
						Data1 = HeartData[i] - '0';
					}
					else if ((HeartData[i] >= 'A') && (HeartData[i] <= 'F')) {
						Data1 = HeartData[i] - 55;
					}
					if ((HeartData[i + 1] >= '0') && (HeartData[i + 1] <= '9')) {
						Data2 = HeartData[i + 1] - '0';
					}
					else if ((HeartData[i + 1] >= 'A') && (HeartData[i + 1] <= 'F')) {
						Data2 = HeartData[i + 1] - 55;
					}
					user_Set.heart_info[i / 2] = Data1 * 0x10 + Data2;
					i += 2;
					templen = i / 2;
					HexDataLength--;
				}
				if (user_Set.heart_len % 2) {
					if ((HeartData[i] >= '0') && (HeartData[i] <= '9')) {
						user_Set.heart_info[i / 2] = HeartData[i] - '0';
					}
					else if ((HeartData[i] >= 'A') && (HeartData[i] <= 'F')) {
						user_Set.heart_info[i / 2] = HeartData[i] - 55;
					}
					i++;
					templen++;
				}
				user_Set.heart_len = templen;
				i = 0;
				//str_cpy0(&RS232_buff[7],user_Set.heart_info,user_Set.heart_len);
				prog_pare_flag = 1;
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else if (RS232_buff[0] == 0xff && RS232_buff[1] == 0x5B) //Set HEART TIME
		{
			if (RS232_buff[3] == 'H' && RS232_buff[4] == 'T' && RS232_buff[5] == 'S' && RS232_buff[6] == '-') {
				user_Set.heart_time_len = RS232_buff[2];
				memcpy(user_Set.heart_time_info, &RS232_buff[7], user_Set.heart_time_len);
				prog_pare_flag = 1;
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf1);

				//将心跳包时间从字符串转为整型
				memcpy(Temp, user_Set.heart_time_info, user_Set.heart_time_len);
				HeartTime = Str2Int((char*)Temp);
				HeartTime *= 100; //换算成ms

				memset(Temp, 0, 16);
			}
			else {
				USART1_SendCharToRS232(0x55);
				USART1_SendCharToRS232(0xf3);
			}
		}
		else {
			USART1_SendCharToRS232(0x55);
			USART1_SendCharToRS232(0xf2);
		}
		RS232_REC_Flag = 0;
	}

	if (prog_pare_flag == 1) {
		//DebugDly = 0;	//配置完成30S后进入调试模式
		FLASH_WriteUserSet();
		InitAllPara();
		FLASH_ReadUserSet();
		prog_pare_flag = 0;
	}
}
