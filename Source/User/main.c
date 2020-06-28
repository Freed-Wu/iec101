#define _main_c
#define THIS_VERSION ("GPRS V2.95(T) 20200701")
#define DEBUG_MODE
#define USER_IP "218.29.54.111"
#define USER_PORT "20001"
#define USER_ADDR "1"
#define USER_APN "ctnet"
#define USER_USER "cm"
#define USER_PASSWORD "gprs"
#define USER_HEART_TIME "40"
#define USER_FIRST_USED_FLAG 0x3378
#define USER_HEART "0123"
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "101_Protocol.h"
#include "Flash.h"
#include "MG301.h"
#include "Para_Config.h"
#include "bsp_bkp.h"
#include "conf_GPIO.h"
#include "conf_NVIC.h"
#include "conf_RCC.h"
#include "conf_USART.h"
#include "conf_sys.h"
#include "conf_tim2.h"
#include "crc.h"
#include "ds3231.h"
#include "fun.h"
#include "gprs.h"
#include "i2c_ee.h"
#include "nrf905.h"
#include "stm32f10x.h"
#include "string.h"
#include "user_Configuration.h"

#define BVL GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) //电池电压检测脚

/*静态变量声明*/
//static uint16_t PowerOK_flag;
static uint16_t ReceiveDataFromGPRSflg;

volatile uint16_t SysTick_10msflg; //10ms溢出标志，主程序10ms运行一次
DEVICE_SET user_Set;
uint16_t GPRSLEDStat;
uint16_t info_wr_flash_flag;
uint16_t temp_wr_flash_flag;
uint16_t DebugDly; //用于串口调试延时
uint16_t DebugNRF905Dly;
uint16_t nrf905ReaddRegDly;
uint32_t nrf905InitDly;

uint16_t ReceiveLength;

uint8_t Info[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //开关量数量 0/1/2 跌落 3跌落状态 4温度状态 5 欠压 6/7/8/9 漏保
uint16_t InfoCRC16; //每次数据读写之前都要校验数据的有效性，无效时要重新从FLASH中读取
uint8_t InfoTemp[8]; //温度
uint8_t DataFromGPRSBuffer[64];
uint8_t moduleMaskEn; // 模块故障MASK
uint32_t moduleMaskDly;
uint32_t PowerLowDly;
uint32_t PowerOKDly;

uint32_t InfoDisConnectDelay[3];
uint32_t TempDisConnectDelay[3];
uint8_t ReceiveData[100];

unsigned int nrf905DR;
extern uint16_t reqVersionflg;
extern uint16_t GPRSStat;
extern TimeStructure Tx_Time;

//正常返回1，错误返回0
//每次读写都要重新校验CRC，有错时则重新从FLASH中读取数据
uint16_t CheckInfoCRCIsOK(void) {
	return CRC16(Info, 16) == InfoCRC16;
}

//数据改变后更新CRC
void RefreshInfoCRC(void) {
	InfoCRC16 = CRC16(Info, 16);
}

void InitAllPara(void) {
	unsigned char heart[9] = USER_HEART;
	user_Set.ip_len = strlen(USER_IP);
	strcpy((char*)user_Set.ip_info, USER_IP);
	user_Set.port_len = strlen(USER_PORT);
	strcpy((char*)user_Set.port_info, USER_PORT);
	user_Set.addr_len = strlen(USER_ADDR);
	strcpy((char*)user_Set.addr_info, USER_ADDR);
	user_Set.ModuleID[0] = 0x2f;
	user_Set.ModuleID[1] = 0x2f;
	user_Set.ModuleID[2] = 0xff;
	user_Set.ModuleID[3] = 0xf1;
	ReceiveDataFromGPRSflg = 0;
	memset(DataFromGPRSBuffer, 0, 64);
	user_Set.apn_len = strlen(USER_APN);
	strcpy((char*)user_Set.apn_info, USER_APN);
	user_Set.user_len = strlen(USER_USER);
	strcpy((char*)user_Set.user_info, USER_USER);
	user_Set.password_len = strlen(USER_PASSWORD);
	strcpy((char*)user_Set.password_info, USER_PASSWORD);
	user_Set.heart_len = 7;
	memcpy((char*)user_Set.heart_info, (char*)heart, 9);
	user_Set.heart_time_len = strlen(USER_HEART_TIME);
	strcpy((char*)user_Set.heart_time_info, USER_HEART_TIME);
	user_Set.FirstUsedFlag = USER_FIRST_USED_FLAG;
}

static uint16_t run_loop_cnt;

int main(void) {
	uint8_t Temp[16];
	run_loop_cnt = 0;
	info_wr_flash_flag = 0;
	temp_wr_flash_flag = 0;
	DebugNRF905Dly = 0 * 50 * 60;
	//片上设备初始化
	RCC_Configuration();
	SysTick_Init(); //10ms中断一次
	NVIC_Configuration();
	GPIO_Configuration();
	USART1_Configuration(); //RS232配置通道
	USART3_Configuration(); //GPRS通道
	EXTI_Configuration(); //NRF595中断脚控制
	SYSCLKConfig_STOP(); //休眠配置
	//片外设备初始化

	//work led	on
	GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //灯亮
	Delay(10);
	PowerUp_NRF905();
	Delay(50);

	FLASH_ReadUserSet();

	if (user_Set.FirstUsedFlag != USER_FIRST_USED_FLAG) {
		InitAllPara(); //初始化参数
		FLASH_WriteUserSet();
	}
	memset(Temp, 0x00, 16);
	memcpy(Temp, user_Set.addr_info, user_Set.addr_len);
	LINK_ADDRESS = Str2Int((char*)Temp);
	memset(Temp, 0x00, 16);

	FLASH_RD_Module_Status();

#ifndef DEBUG_MODE
	IWDG_Init(IWDG_Prescaler_16, 0xFFF); //1.6s
	IWDG_Feed(); //clr WDG
#endif
	GPRS_init(); //变量初始化
	//读温度值
	PWR_BackupAccessCmd(ENABLE); //使能后备寄存器仿问
	if ((BKP_ReadBackupRegister(BKP_DR1) != 0XA55A) || bkp_ReadTempData() != 0) {
		BKP_WriteBackupRegister(BKP_DR1, 0XA55A);
		InfoTemp[0] = 30;
		InfoTemp[1] = 30;
		InfoTemp[2] = 30;
		InfoTemp[3] = 30;
		InfoTemp[4] = 30;
		InfoTemp[5] = 30;
		InfoTemp[6] = 30;
		InfoTemp[7] = 30;
		bkp_WriteTempData(); //初始化一次数据
	}

	EXTI_ClearITPendingBit(EXTI_Line11);
	EnableExtINT();

	DebugDly = 0; //0分钟时间
	PowerLowDly = 0;
	PowerOKDly = 0;
	nrf905ReaddRegDly = 50 * 1 * 1;
	nrf905InitDly = 50 * 60 * 60 * 47; //48个小时强制初始化一次
	InfoDisConnectDelay[0] = 50 * 60 * 60 * 12;
	InfoDisConnectDelay[1] = 50 * 60 * 60 * 12;
	InfoDisConnectDelay[2] = 50 * 60 * 60 * 12;

	TempDisConnectDelay[0] = 50 * 60 * 60 * 12;
	TempDisConnectDelay[1] = 50 * 60 * 60 * 12;
	TempDisConnectDelay[2] = 50 * 60 * 60 * 12;

	moduleMaskEn = 0;
	moduleMaskDly = 0;
	while (1) {
		if (SysTick_10msflg) {
			SysTick_10msflg = 0;
			if (DebugDly > 0)
				DebugDly--;
			if (moduleMaskDly > 0)
				moduleMaskDly--;
			else
				moduleMaskEn = 0; //屏蔽24小时后自动退出

			if (reqVersionflg) {
				reqVersionflg = 0;
				USART1_SendDataToRS232(THIS_VERSION, strlen(THIS_VERSION));
			}
			//--------------------------------
			if (DebugNRF905Dly > 0)
				DebugNRF905Dly--;
			if (nrf905InitDly > 0)
				nrf905InitDly--;

			if (InfoDisConnectDelay[0])
				InfoDisConnectDelay[0]--;
			if (InfoDisConnectDelay[1])
				InfoDisConnectDelay[1]--;
			if (InfoDisConnectDelay[2])
				InfoDisConnectDelay[2]--;

			if (TempDisConnectDelay[0])
				TempDisConnectDelay[0]--;
			if (TempDisConnectDelay[1])
				TempDisConnectDelay[1]--;
			if (TempDisConnectDelay[2])
				TempDisConnectDelay[2]--;

			if ((InfoDisConnectDelay[0] == 0) || (InfoDisConnectDelay[1] == 0) || (InfoDisConnectDelay[2] == 0)) {
				if (Info[3] == 0) { //状态改变保存一次
					DisableExtINT();
					if (CheckInfoCRCIsOK() == 0) //每次改变Info数据之前都要重新
						FLASH_RD_Module_Status();
					Info[3] = 0x01;
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
					EnableExtINT();
				}
			}
			else {
				if (Info[3] == 1) { //状态改变保存一次
					DisableExtINT();
					if (CheckInfoCRCIsOK() == 0) //每次改变Info数据之前都要重新
						FLASH_RD_Module_Status();
					Info[3] = 0x00;
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
					EnableExtINT();
				}
			}
			//温度不在线测试
			if ((TempDisConnectDelay[0] == 0) || (TempDisConnectDelay[1] == 0) || (TempDisConnectDelay[2] == 0)) {
				if (Info[4] == 0) { //状态改变保存一次
					DisableExtINT();
					if (CheckInfoCRCIsOK() == 0) //每次改变Info数据之前都要重新
						FLASH_RD_Module_Status();
					Info[4] = 0x01;
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
					EnableExtINT();
				}
			}
			else {
				if (Info[4] == 1) { //状态改变保存一次
					DisableExtINT();
					if (CheckInfoCRCIsOK() == 0) //每次改变Info数据之前都要重新
						FLASH_RD_Module_Status();
					Info[4] = 0x00;
					RefreshInfoCRC();
					info_wr_flash_flag = 1;
					EnableExtINT();
				}
			}

			if (GPIO_ReadInputDataBit(NRF905_DR, NRF905_DR_PIN)) { //中断中未捕获数据上升沿时 初始化中断服务程序并初始化NRF905
				Delay(10);
				if (GPIO_ReadInputDataBit(NRF905_DR, NRF905_DR_PIN)) {
					DisableExtINT();
					PowerUp_NRF905();
					NVIC_Configuration();
					EnableExtINT();
				}
			}

			/*检测NRF905的有效性*/
			if (nrf905ReaddRegDly > 0)
				nrf905ReaddRegDly--;
			else {
				uint16_t nrf905errorflg;

				nrf905errorflg = 0;
				nrf905ReaddRegDly = 50 * 20; //20S读一次寄存器值
				DisableExtINT();
				if (checknrf905_conf() == 0) //错误
					nrf905errorflg = 1;
				if (checknrf905_addr() == 0) //错误
					nrf905errorflg = 1;
				if (nrf905errorflg == 1 || (nrf905InitDly == 0)) {
					nrf905errorflg = 0;
					nrf905InitDly = 50 * 60 * 60 * 47; //48个小时强制初始化一次
					PowerUp_NRF905();
				}
				EnableExtINT();
			}
			//LED_Toggle();
#ifndef DEBUG_MODE
			IWDG_Feed(); //clr WDG
#endif
			//一旦检测到TCP通道断开，立即启动连接
			ReceiveDataFromGPRSflg = SuperviseTCP(DataFromGPRSBuffer);
			if (ReceiveDataFromGPRSflg)
				DataProcess();

			//配置串口处理程序
			rs232_set_process();
			USART1_supervise(); //长时间没有接收到数据时清一次数据
			//电池电压检测，正常时BVL为低电平
			if (!BVL) //电压低
			{
				PowerOKDly = 0;
				if (CheckInfoCRCIsOK() == 0)
					FLASH_RD_Module_Status();

				if (Info[5] == 0) //当前状态为高电压状态
				{
					if (PowerLowDly < 50 * 60 * 10) //延时10分钟报电压低
						PowerLowDly++;
					else {
						GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET);
						//read time
						ReadDATATime();
						ChangeUpdate(0x06, 0x01, &Tx_Time);
						if (CheckInfoCRCIsOK() == 0) //每次改变Info数据之前都要重新
							FLASH_RD_Module_Status();
						Info[5] = 0x01;
						RefreshInfoCRC();
						info_wr_flash_flag = 1;
						//USART1_SendDataToRS232("Module Power is Low\r\n",21);
						//work led off
						GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
					}
				}
			}
			else { //Power is ok
				PowerLowDly = 0;
				if (CheckInfoCRCIsOK() == 0)
					FLASH_RD_Module_Status();
				if (Info[5] != 0) {
					if (PowerOKDly < 50 * 60 * 30) //30分钟报正常
						PowerOKDly++;
					else {
						GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET);
						//read time
						ReadDATATime();
						ChangeUpdate(0x06, 0x00, &Tx_Time);
						if (CheckInfoCRCIsOK() == 0)
							FLASH_RD_Module_Status();
						Info[5] = 0x00;
						RefreshInfoCRC();
						info_wr_flash_flag = 1;
						//USART1_SendDataToRS232("Module Power is OK\r\n",20);
						//work led off
						GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
					}
				}
			}

			//
			if (info_wr_flash_flag == 1) {
				FLASH_WR_Module_Status();
				info_wr_flash_flag = 0;
			}
			if (temp_wr_flash_flag == 1) {
				bkp_WriteTempData();
				temp_wr_flash_flag = 0;
			}

			if (run_loop_cnt > 0)
				run_loop_cnt--;
			switch (GPRSLEDStat) {
			case GPRS_LED_IDLE:
				if (GPRSStat < 0x700) { //配置状态
					if (((run_loop_cnt) == 14)) //半秒钟闪一次
						GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
					else if (run_loop_cnt == 12)
						GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
					else if (run_loop_cnt == 0)
						run_loop_cnt = 15;
				}
				else {
					GPRSLEDStat = GPRSGetStatBuf();
					if (GPRSLEDStat == GPRS_LED_IDLE) { //非配置的等待状态4s钟闪4mS
						if (GPRSStat == GPRS_RUN_Txdata) { //等待指令返回及发送数据
							if (run_loop_cnt == 49)
								GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
							else if (run_loop_cnt == 47)
								GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
							else if (run_loop_cnt == 0) //2S亮一次
								run_loop_cnt = 50;
						}
						else if (GPRSStat == GPRS_RUN_Txdata_ACK) { //等待数据返回正确
							if (run_loop_cnt == 49)
								GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
							else if (run_loop_cnt == 47)
								GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
							else if (run_loop_cnt == 0)
								run_loop_cnt = 50;
						}
						else { //正常运行的空闲状态4S闪一次
							if (run_loop_cnt == 199)
								GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
							else if (run_loop_cnt == 194)
								GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
							else if (run_loop_cnt == 0) { //4S亮一次
								GPRSLEDStat = GPRSGetStatBuf();
								run_loop_cnt = 200;
							}
						}
					}
					else //进入发送状态
						run_loop_cnt = 50;
				}
				break;
			case GPRS_LED_START: //一长
				if (run_loop_cnt == 49)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 35)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 0) { //4S亮一次
					GPRSLEDStat = GPRSGetStatBuf();
					run_loop_cnt = 50;
				}
				break;
				//case GPRS_LED_CMD_OK:	//一长一短
			case GPRS_LED_DATA_OK: //
				if (run_loop_cnt == 49)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 35)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 30)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 28)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 0)
					GPRSLEDStat = GPRSGetStatBuf();
				run_loop_cnt = 50;
				break;
			case GPRS_LED_CMD_ERROR: //一长两短
			case GPRS_LED_DATA_ERROR:
				if (run_loop_cnt == 49)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 35)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 30) //1短
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 28)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 20)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 18)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 0) {
					GPRSLEDStat = GPRSGetStatBuf();
					run_loop_cnt = 50;
				}
				break;
			case GPRS_LED_CMD_TO: //一长三短
			case GPRS_LED_DATA_TO:
				if (run_loop_cnt == 49)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 38)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 30) //1短
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 28)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 20)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 18)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 10)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else if (run_loop_cnt == 8)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				else if (run_loop_cnt == 0) {
					GPRSLEDStat = GPRSGetStatBuf();
					run_loop_cnt = 100;
				}
				break;
			case GPRS_LED_DATA: //频闪
				if ((run_loop_cnt & 0x2) != 0)
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_RESET); //work led on
				else
					GPIO_WriteBit(ARM_RUN, ARM_RUN_PIN, Bit_SET);
				if (run_loop_cnt == 0) {
					GPRSLEDStat = GPRSGetStatBuf();
					run_loop_cnt = 100;
				}
				break;
			default:
				GPRSLEDStat = GPRS_LED_IDLE;
				break;
			}

		} //if()
#ifndef DEBUG_MODE
		__WFI(); //进入睡眠模式
#endif
	} //while
} //main

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1) {
	}
}

#endif
