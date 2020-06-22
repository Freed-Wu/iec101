#include "MG301.h"
#include "Para_Config.h"
#include "conf_USART.h"
#include "fun.h"
#include "main.h"
#include "mydef.h"
#include "stm32f10x.h"
#include "string.h"

GPRS_Tx GPRS_Tx0;
GPRSStatBuf GPRSStatBuf0;

uint16_t ReqGPRSConfigflg;

/*static*/ uint32_t BeatCnt = 0;
static uint8_t GPRS_ReceiveData[GPRS_RCV_BUF];
static uint16_t GPRS_ReceiveLength;

#define TIME_GPRSNotCallRstDelay (uint32_t)(50 * 60 * 12) //(50 * 60 * 12)

uint32_t HeartTime = 3000; //定义心跳包时间默认值为30s
uint16_t GPRSStat = GPRS_IDLE;
//uint16_t GPRSSubStat = GPRS_IDLE_SUB;
uint16_t sGPRSTimeDelay;
uint16_t sGPRSSentdataDelay;
uint32_t sGPRSNotCallRstDelay;
uint16_t GPRSErrorCnt = 0;
uint16_t GPRSOpenErrorCnt;
uint16_t GPRSSendBeatDataflg = 0;
uint16_t GPRSFaultcnt; //接收不到总召唤计数，每次收不到总召唤则复位301，3次复位不成功则关机重启
uint16_t GPRSCheckRstcnt;
uint16_t GPRSSendFrrorFlg; //发送数据错误指令

extern DEVICE_SET user_Set;

//内部函数声明
//static uint16_t GetDataToGPRSTxbuf(char * pdata);
void rmDataFromGPRSTxbuf(void);

void GPRSInitTxBuf(void) {
	uint16_t i;

	GPRS_Tx0.TxNum = 0;
	GPRS_Tx0.TxPtrIn = 0;
	GPRS_Tx0.TxPtrOut = 0;
	for (i = 0; i < GPRS_TX_BUF_NUM; i++) {
		GPRS_Tx0.TxBuf[i][0] = '\0';
		GPRS_Tx0.TxLength[i] = 0;
	}
}
void GPRSInitTxStatBuf(void) {
	GPRSStatBuf0.Num = 0;
	GPRSStatBuf0.PtrIn = 0;
	GPRSStatBuf0.PtrOut = 0;
	GPRSStatBuf0.Buf[GPRSStatBuf0.PtrIn] = GPRS_LED_IDLE;
};

void GPRS_init(void) {
	static uint16_t data_flg;

	GPIO_ResetBits(GPIOB, GPIO_Pin_15); //低电压时开机脚为高，不动作

	memset(GPRS_ReceiveData, 0, GPRS_RCV_BUF);
	GPRS_ReceiveLength = 0;

	//以下是将部分参数的字符型转为整型数据
	//将心跳包时间从字符串转为整型
	HeartTime = Str2Int((char*)user_Set.heart_time_info);
	HeartTime *= 100; //换算成ms

	//初始化发送缓冲区
	if (data_flg == 0) {
		data_flg = 1;
		GPRSInitTxBuf();
	}
	GPRSStat = GPRS_IDLE;
	GPRSErrorCnt = 0;
	GPRSSendBeatDataflg = 0;
	sGPRSTimeDelay = 100;
	sGPRSSentdataDelay = 100;
	sGPRSNotCallRstDelay = TIME_GPRSNotCallRstDelay;
	GPRSOpenErrorCnt = 0;
	GPRSFaultcnt = 0;
	GPRSCheckRstcnt = 0;
	GPRSInitTxStatBuf();
}
/*
****************************************************************************************************
* 功能描述：TCP通道重新连接，一旦检测到断开立即重连
* 输入参数：
* 返回参数：
* 说    明：
****************************************************************************************************
*/

/*******************************************************************************
					管理TCP的正常运行
********************************************************************************/

uint16_t SuperviseTCP(uint8_t* pRecBuffer) {
	char* pReceiveData; //指向数据域地址
	uint16_t pReceiveLength;
	uint16_t i;

	pReceiveLength = 0;
	GPRS_ReceiveLength = Supervise_USART3(GPRS_ReceiveData); //接收到数据标示
	if (sGPRSTimeDelay > 0)
		sGPRSTimeDelay--;
	if (sGPRSSentdataDelay > 0)
		sGPRSSentdataDelay--;
	if (BeatCnt < HeartTime) {
		BeatCnt += 2; //步进是20ms
	}
	if (GPRSStat > 0x700) { //正常收发数据状态5分钟未收到总召唤时重发
		if (sGPRSNotCallRstDelay) {
			sGPRSNotCallRstDelay--;
		}
	}
	else {
		sGPRSNotCallRstDelay = TIME_GPRSNotCallRstDelay; //12分钟;
	}

	switch (GPRSStat) {
	case GPRS_IDLE:
		if (sGPRSTimeDelay == 0) {
			GPRSErrorCnt = 0;
			memset(GPRS_ReceiveData, 0, GPRS_RCV_BUF); //初始化ReceiveData序列
			sGPRSTimeDelay = NEXT_CMD_DLY; //
			GPRSStat = GPRS_POWER_CMD_SEND;
		}
		break;
	case GPRS_POWER_CMD_SEND:
		USART3_InitRXbuf();
		USART3_SendDataToGPRS("AT\r", strlen("AT\r")); //是否开机
		sGPRSTimeDelay = WAIT_ACK;

		GPRSStat = GPRS_POWER_CMD_ACK;
		break;
	case GPRS_POWER_CMD_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_APN_CMD_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_IDLE;
			}
		}
		else {
			if (sGPRSTimeDelay == 0) {
				GPRSStat = GPRS_POWER_ON_START;
			}
		}
		break;
	case GPRS_POWER_ON_START:
		if (sGPRSTimeDelay == 0) {
			GPIO_SetBits(GPIOB, GPIO_Pin_15); //拉低GPRS模块开机引脚电平
			DelayMs(500);
			GPIO_ResetBits(GPIOB, GPIO_Pin_15);
			sGPRSTimeDelay = WAIT_START; //延时2S左右
			GPRSStat = GPRS_POWER_WAIT_START;
			USART3_InitRXbuf();
		}
		break;
	case GPRS_POWER_WAIT_START: //上电成功后会返回一个字符串
		/*if(GPRS_ReceiveLength != 0){//接收到一帧数据
				if(strstr((const char *)GPRS_ReceiveData,"SYSSTART") != NULL)//开机状态
				{
					GPRSStat = GPRS_ECHO_CMD_SEND;
					sGPRSTimeDelay = NEXT_CMD_DLY;
					GPIO_ResetBits(GPIOB,GPIO_Pin_15);	//拉低GPRS模块开机引脚电平
				}else if(strstr((const char *)GPRS_ReceiveData,"SHUTDOWN") != NULL){
					GPRSStat = GPRS_POWER_ON_START;
					sGPRSTimeDelay = NEXT_CMD_DLY;

				}*/
		if (sGPRSTimeDelay == (WAIT_START - 125)) {
			GPIO_ResetBits(GPIOB, GPIO_Pin_15); //拉低GPRS模块开机引脚电平
			GPRSStat = GPRS_APN_CMD_SEND;
			sGPRSTimeDelay = NEXT_CMD_DLY;
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
			GPIO_ResetBits(GPIOB, GPIO_Pin_15); //拉低GPRS模块开机引脚电平
		}
		break;
	case GPRS_POWER_RST:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS("AT+CFUN=1,1\r", strlen("AT+CFUN=1,1\r"));
			GPRSStat = GPRS_POWER_RST_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_POWER_RST_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_POWER_WAIT_START;
				sGPRSTimeDelay = WAIT_ACK;
			}
			else {
				GPRSStat = GPRS_POWER_ON_START;
				sGPRSTimeDelay = 0;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_POWER_ON_START;
			sGPRSTimeDelay = 0;
		}
		break;
	//设置APN
	case GPRS_APN_CMD_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS("AT+CGDCONT=1,\"IP\",\"CMNET\"\r", SIZEOF("AT+CGDCONT=1,\"IP\",\"CMNET\"\r"));
			GPRSStat = GPRS_APN_CMD_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_APN_CMD_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_CHECK_CSQ_A;
				sGPRSTimeDelay = 200; //延时4S检测信号质量
			}
			else {
				GPRSStat = GPRS_IDLE;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;
		//链接GPRS-------------------------------------------------------
		//搜网是显示信号强度-------------------------------------
	case GPRS_CHECK_CSQ_A:
		if (sGPRSTimeDelay == 0) {
			//	GPRSErrorCnt = 0;
			USART3_SendDataToGPRS("AT+CSQ\r", strlen("AT+CSQ\r"));
			GPRSStat = GPRS_CHECK_CSQ_ACK_A;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_CHECK_CSQ_ACK_A:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_CHECK_CREG_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_CHECK_CREG_SEND; //是否检测正确都进行下一条指令
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;
	//查询注册网络状态
	case GPRS_CHECK_CREG_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS("AT+CREG?\r", strlen("AT+CREG?\r"));
			GPRSStat = GPRS_CHECK_CREG_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_CHECK_CREG_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_CHECK_CPSI_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_CHECK_CPSI_SEND; //是否检测正确都进行下一条指令
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;
	//查询注册信息
	case GPRS_CHECK_CPSI_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS("AT+CPSI?\r", strlen("AT+CPSI?\r"));
			GPRSStat = GPRS_CHECK_CPSI_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_CHECK_CPSI_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_CHECK_CMD_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_CHECK_CMD_SEND; //是否检测正确都进行下一条指令
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;
	case GPRS_CHECK_CMD_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS("AT+CGREG?\r", strlen("AT+CGREG?\r"));
			GPRSStat = GPRS_CHECK_CMD_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_CHECK_CMD_ACK: //
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if ((strstr((const char*)GPRS_ReceiveData, "CGREG: 0,1\r\n\r\nOK") != NULL) || (strstr((const char*)GPRS_ReceiveData, "CGREG: 0,5\r\n\r\nOK") != NULL)) //开机状态
			{
				GPRSStat = GPRS_TCP_conType_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
				GPRSErrorCnt = 0;
				GPRSCheckRstcnt = 0;
			}
			else {
				if (GPRSErrorCnt < 20) {
					GPRSErrorCnt++;
					GPRSStat = GPRS_CHECK_CSQ_A;
					sGPRSTimeDelay = WAIT_ACK;
				}
				else {
					GPRSErrorCnt = 0;
					GPRSCheckRstcnt = 0; //三次无效后重启
					GPRSStat = GPRS_POWER_ON_START;
					sGPRSTimeDelay = 12000; //4分钟后重试
				}
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;
	//链接TCP-------------------------------------------------------------------
	case GPRS_TCP_conType_SEND:
		if (sGPRSTimeDelay == 0) {
			GPRSErrorCnt = 0;
			USART3_SendDataToGPRS("AT+CIPMODE=0\r", strlen("AT+CIPMODE=0\r"));
			GPRSStat = GPRS_TCP_conType_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_TCP_conType_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_TCP_NETOPEN_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;
	//NETOPEN-------------------------------------------------------------------
	case GPRS_TCP_NETOPEN_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS("AT+NETOPEN\r", strlen("AT+NETOPEN\r"));
			GPRSStat = GPRS_TCP_NETOPEN_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_TCP_NETOPEN_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_TCP_BUFFERMODE_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;

	//BUFFERMODE-------------------------------------------------------------------
	case GPRS_TCP_BUFFERMODE_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS("AT+CIPRXGET=1\r", strlen("AT+CIPRXGET=1\r"));
			GPRSStat = GPRS_TCP_BUFFERMODE_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_TCP_BUFFERMODE_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSStat = GPRS_TCP_Name_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;

	//----------------------TCP--------------------------------
	case GPRS_TCP_Name_SEND: //APN
		if (sGPRSTimeDelay == 0) {
			uint8_t i = 0;
			uint8_t APN_Name[64] = "AT+CIPOPEN=0,\"TCP\",";
			uint8_t Len1 = 0;
			uint8_t Len2 = 0;
			uint8_t Length = 0;

			i = 0;
			Len1 = user_Set.ip_len;
			Len2 = user_Set.port_len;
			Length = strlen("AT+CIPOPEN=0,\"TCP\",");
			while (Len1--) {
				APN_Name[i + Length] = user_Set.ip_info[i];
				i++;
			}
			APN_Name[i + Length] = ',';
			i++;
			while (Len2--) {
				APN_Name[i + Length] = user_Set.port_info[i];
				i++;
			}
			APN_Name[i + Length] = '\r';
			i++;

			USART3_SendDataToGPRS(APN_Name, i + Length);
			GPRSStat = GPRS_TCP_Name_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_TCP_Name_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //开机状态
			{
				GPRSErrorCnt = 0; //配置成功后清除错误计数
				GPRSOpenErrorCnt = 0;
				GPRSInitTxBuf();
				GPRSStat = GPRS_RUN_Rxdata_CMD;
				sGPRSTimeDelay = NEXT_CMD_DLY;
				if (HeartTime > 500)
					BeatCnt = HeartTime - 500;
				else
					BeatCnt = 0;
			}
			else {
				if (GPRSOpenErrorCnt > 1) {
					GPRSOpenErrorCnt = 0;
					GPRSStat = GPRS_POWER_ON_START;
					sGPRSTimeDelay = 50 * 60 * 2; //等2分钟重启
				}
				else {
					GPRSOpenErrorCnt++;
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
			}
		}
		else if (sGPRSTimeDelay == 0) {
			if (GPRSOpenErrorCnt > 1) {
				GPRSOpenErrorCnt = 0;
				GPRSStat = GPRS_POWER_ON_START;
				sGPRSTimeDelay = 50 * 60 * 2;
			}
			else {
				GPRSOpenErrorCnt++;
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		break;
		/*
			//------------------------user------------------------------------------------
		case GPRS_TCP_User_SEND:	//User
			if(sGPRSTimeDelay == 0){
				uint16_t i = 0;
				uint8_t APN_User[64] = "at^sics=0,user,";
				uint16_t Len;
				uint16_t Length;
				i = 0;
				Len = user_Set.user_len;
				Length = strlen("at^sics=0,user,");
				while(Len--)
				{
					APN_User[i + Length] = user_Set.user_info[i];
					i++;
				}
				APN_User[i + Length] = '\r';
				i++;
				USART3_SendDataToGPRS(APN_User,i + Length);
				GPRSStat = GPRS_TCP_User_ACK;
				sGPRSTimeDelay = WAIT_ACK;		//5S
			}
		break;
		case GPRS_TCP_User_ACK:
			if(GPRS_ReceiveLength != 0){//接收到一帧数据
				if(strstr((const char *)GPRS_ReceiveData,"OK") != NULL)//开机状态
				{
					GPRSStat = GPRS_TCP_Password_SEND;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}else{
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
			}else if(sGPRSTimeDelay == 0){
				GPRSStat = GPRS_IDLE;
			}
		break;
		//----------------------Password-------------------------------
		case GPRS_TCP_Password_SEND:	//Password
		if(sGPRSTimeDelay == 0){

			uint16_t i = 0;
			uint8_t APN_Password[64] = "at^sics=0,passwd,";
			uint16_t Len;
			uint16_t Length;
			i = 0;
			Len = user_Set.password_len;
			Length = strlen("at^sics=0,passwd,");
			while(Len--)
			{
				APN_Password[i+Length] = user_Set.password_info[i];
				i++;
			}
			APN_Password[i+Length] = '\r';
			i++;
			//USART3_SendDataToGPRS("at^sics=0,passwd,gprs\r\n",strlen("at^sics=0,passwd,gprs\r"));
			USART3_SendDataToGPRS(APN_Password,i + Length);
			GPRSStat = GPRS_TCP_Password_ACK;
			sGPRSTimeDelay = WAIT_ACK;		//5S
		}
		break;
		case GPRS_TCP_Password_ACK:
			if(GPRS_ReceiveLength != 0){//接收到一帧数据
				if(strstr((const char *)GPRS_ReceiveData,"OK") != NULL)//开机状态
				{
					GPRSStat = GPRS_TCP_srvType_SEND;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}else{
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
			}else if(sGPRSTimeDelay == 0){
				GPRSStat = GPRS_IDLE;
			}
		break;
		case GPRS_TCP_srvType_SEND:	//srvType
			if(sGPRSTimeDelay == 0){
				USART3_SendDataToGPRS("at^siss=0,srvType,socket\r",strlen("at^siss=0,srvType,socket\r"));
				GPRSStat = GPRS_TCP_srvType_ACK;
				sGPRSTimeDelay = WAIT_ACK;		//5S
			}
		break;
		case GPRS_TCP_srvType_ACK:
			if(GPRS_ReceiveLength != 0){//接收到一帧数据
				if(strstr((const char *)GPRS_ReceiveData,"OK") != NULL)//开机状态
				{
					GPRSStat = GPRS_TCP_IP_SEND;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}else{
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
			}else if(sGPRSTimeDelay == 0){
				GPRSStat = GPRS_IDLE;
			}
		break;
		case GPRS_TCP_IP_SEND:	//IP
		if(sGPRSTimeDelay == 0){
			uint8_t i = 0;
			uint8_t j = 0;
			uint8_t IP_Data[64] = "at^siss=0,address,\"socktcp://\"";
			uint8_t Len = 0;
			uint8_t Length = 0;

			Len = user_Set.ip_len;
			Length = strlen("at^siss=0,address,\"socktcp://");
			while(Len--)
			{
				IP_Data[i+Length] = user_Set.ip_info[i]; //将IP复制到数组中合适的位置
				i++;
			}
			IP_Data[i+Length] = ':';
			i++;
			Len = user_Set.port_len;
			while(Len--)
			{
				IP_Data[i+Length] = user_Set.port_info[j]; //将端口信息复制到数组中IP信息之后
				j++;
				i++;
			}
			IP_Data[i+Length] = '"';
			i++;
			IP_Data[i+Length] = '\r';
			i++;
			USART3_SendDataToGPRS(IP_Data,i + Length);
			GPRSStat = GPRS_TCP_IP_ACK;
			sGPRSTimeDelay = WAIT_ACK;		//5S
		}
		break;
		case GPRS_TCP_IP_ACK:
			if(GPRS_ReceiveLength != 0){//接收到一帧数据
				if(strstr((const char *)GPRS_ReceiveData,"OK") != NULL)//开机状态
					{
						GPRSStat = GPRS_TCP_SISO_SEND;
						sGPRSTimeDelay = NEXT_CMD_DLY;
					}else{
						GPRSStat = GPRS_POWER_RST;
						sGPRSTimeDelay = NEXT_CMD_DLY;
					}
				}else if(sGPRSTimeDelay == 0){
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
		break;
		case GPRS_TCP_SISO_SEND:	//SISO
			if(sGPRSTimeDelay == 0){
				USART3_SendDataToGPRS("AT^SISO=0\r",strlen("AT^SISO=?\r"));
				GPRSStat = GPRS_TCP_SISO_ACK;
				sGPRSTimeDelay = 500;//WAIT_ACK;		//5S
			}
		break;
		case GPRS_TCP_SISO_ACK:
			if(GPRS_ReceiveLength != 0){//接收到一帧数据
				if(strstr((const char *)GPRS_ReceiveData,"OK") != NULL)//开机状态
				{
					GPRSErrorCnt = 0;					//配置成功后清除错误计数
					GPRSOpenErrorCnt = 0;
					GPRSInitTxBuf();
					GPRSStat = GPRS_RUN_Rxdata_CMD;
					sGPRSTimeDelay = NEXT_CMD_DLY;
					if(HeartTime>500)	BeatCnt = HeartTime - 500;
					else				BeatCnt = 0;
				}else{
					if(GPRSOpenErrorCnt > 1 ){
						GPRSOpenErrorCnt = 0;
						GPRSStat = GPRS_POWER_ON_START;
						sGPRSTimeDelay = 50 * 60 *2;//等2分钟重启
					}else{
						GPRSOpenErrorCnt++;
						GPRSStat = GPRS_POWER_RST;
						sGPRSTimeDelay = NEXT_CMD_DLY;
					}
				}
			}else if(sGPRSTimeDelay == 0){
				if(GPRSOpenErrorCnt > 1 ){
					GPRSOpenErrorCnt = 0;
					GPRSStat = GPRS_POWER_ON_START;
					sGPRSTimeDelay = 50 * 60 * 2;
				}else{
					GPRSOpenErrorCnt++;
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
			}
		break;*/
		/*************************************************************************
					正常程序运行下面的部分

**************************************************************************/
	case GPRS_RUN_Rxdata_CMD: //接收数据指令
		if (ReqGPRSConfigflg) {
			GPRSStat = GPRS_POWER_RST;
			ReqGPRSConfigflg = 0;
		}
		else if (GPRS_Tx0.TxNum > 0) {
			if (sGPRSTimeDelay == 0) {
				GPRSStat = GPRS_RUN_Txdata_CMD;
				GPRSSendBeatDataflg = 0;
			}
			BeatCnt = 0;
		}
		else if (BeatCnt >= HeartTime) { //缓冲区中有数据时进入发送状态
			BeatCnt = 0;
			GPRSSendBeatDataflg = 1;
			GPRSStat = GPRS_RUN_Txdata_CMD;
			sGPRSTimeDelay = NEXT_CMD_DLY;
		}
		else {
			if (sGPRSSentdataDelay == 0) { //2S钟读一次，去缓冲区读数据
				USART3_SendDataToGPRS("AT+CIPRXGET=2,0\r", strlen("AT+CIPRXGET=2,0\r"));
				GPRSStat = GPRS_RUN_Rxdata;
				sGPRSTimeDelay = WAIT_ACK;
			}
		}
		break;
	case GPRS_RUN_Rxdata: //接收数据
		if (GPRS_ReceiveLength != 0) { //缓冲区中有数据
			if (strstr((char*)GPRS_ReceiveData, "ERROR")) {
				if (GPRSFaultcnt < 1) {
					GPRSFaultcnt++;
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
				else {
					GPRSFaultcnt = 0;
					GPRSStat = GPRS_POWER_ON_START;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
			}
			else {
				pReceiveData = strstr((char*)GPRS_ReceiveData, "+CIPRXGET:2,0,");
				if (pReceiveData != NULL) {
					if (pReceiveData[15] == 0x0D) {
						pReceiveLength = (pReceiveData[14] - 0x30);
						for (i = 0; i < pReceiveLength; i++) {
							pRecBuffer[i] = pReceiveData[i + 14];
						}
					}
					else {
						pReceiveLength = (pReceiveData[14] - 0x30) * 10 + (pReceiveData[15] - 0x30);
						for (i = 0; i < pReceiveLength; i++) {
							pRecBuffer[i] = pReceiveData[i + 15];
						}
					}
				}
				if ((pReceiveLength == 0) && (sGPRSNotCallRstDelay == 0)) {
					if (GPRSFaultcnt < 1) {
						GPRSFaultcnt++;
						GPRSStat = GPRS_POWER_RST;
						sGPRSTimeDelay = NEXT_CMD_DLY;
					}
					else {
						GPRSFaultcnt = 0;
						GPRSStat = GPRS_POWER_ON_START;
						sGPRSTimeDelay = NEXT_CMD_DLY;
					}
				}
				else {
					if (pReceiveLength != 0) { //接收到数据后清除错误标志
						GPRSFaultcnt = 0;
					}
					GPRSStat = GPRS_RUN_Rxdata_CMD;
					sGPRSSentdataDelay = 100; //2S钟读一次缓冲
					sGPRSTimeDelay = NEXT_CMD_DLY;
					if (pReceiveLength != 0) {
						sGPRSNotCallRstDelay = TIME_GPRSNotCallRstDelay;
					}
				}
			}
		}
		else if (sGPRSTimeDelay == 0) { //接收超时
			if (sGPRSNotCallRstDelay != 0) {
				GPRSStat = GPRS_POWER_RST;
				sGPRSSentdataDelay = 100; //2S钟读一次缓冲
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				if (GPRSFaultcnt < 1) {
					GPRSFaultcnt++;
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
				else {
					GPRSFaultcnt = 0;
					GPRSStat = GPRS_POWER_ON_START;
					sGPRSTimeDelay = NEXT_CMD_DLY;
				}
			}
		}

		break;
	case GPRS_RUN_Txdata_CMD: //发送数据指令
	{
		uint8_t i = 0;
		uint8_t AT_Cmd[20] = {0x00};
		uint16_t pLength;
		uint8_t LengthString[6] = {0x00};
		if (GPRSSendBeatDataflg) { //发送心跳数据
			strcpy((char*)AT_Cmd, "AT+CIPSEND=1,");
			Int2Str((char*)LengthString, user_Set.heart_len);
			while (LengthString[i]) {
				AT_Cmd[i + 10] = LengthString[i];
				i++;
			}
		}
		else { //否则发送数据
			BeatCnt = 0;
			pLength = GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut];
			strcpy((char*)AT_Cmd, "AT+CIPSEND=0,");
			Int2Str((char*)LengthString, pLength);
			while (LengthString[i]) {
				AT_Cmd[i + 10] = LengthString[i];
				i++;
			}
		}
		AT_Cmd[i + 10] = '\r';
		i++;
		AT_Cmd[i + 10] = '\0'; //字符串结束符
		USART3_SendDataToGPRS(AT_Cmd, strlen((const char*)AT_Cmd));
		//USART3_SendDataToGPRS("AT^SISW=0,5\r",strlen((const char *)"AT^SISW=0,5\r"));
		GPRSStat = GPRS_RUN_Txdata;
		sGPRSTimeDelay = WAIT_ACK;
		GPRSLoadStatBuf(GPRS_LED_START);
	} break;
	case GPRS_RUN_Txdata: //心跳包指令应答信号
		if ((GPRS_ReceiveLength != 0)) {
			//接收到数据就开始发送数据，并不管是否正确
			if (GPRSSendBeatDataflg) {
				USART3_SendDataToGPRS(user_Set.heart_info, user_Set.heart_len);
				USART3_SendDataToGPRS("\r", 1);
				GPRSStat = GPRS_RUN_Txdata_ACK;
				sGPRSTimeDelay = WAIT_ACK; //两个数据的间隔
			}
			else {
				BeatCnt = 0;
				USART3_SendDataToGPRS((uint8_t*)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut], GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut]);
				USART3_SendDataToGPRS("\r", 1);
				GPRSStat = GPRS_RUN_Txdata_ACK;
				sGPRSTimeDelay = WAIT_ACK;
			}
			if ((strstr((char*)GPRS_ReceiveData, "\r\nOK\r\n") != NULL) || (strstr((char*)GPRS_ReceiveData, "+CIPSEND:"))) {
				GPRSSendFrrorFlg = 0;
				GPRSLoadStatBuf(GPRS_LED_DATA); //发送数据
			}
			else { //
				GPRSSendFrrorFlg = 1;
				GPRSLoadStatBuf(GPRS_LED_CMD_ERROR);
			}
		}
		else if (sGPRSTimeDelay == 0) { //发送指令失败时依然发送数据，否则会出现下一次指令当数据发的情况
			if (GPRSSendBeatDataflg) {
				USART3_SendDataToGPRS(user_Set.heart_info, user_Set.heart_len);
				USART3_SendDataToGPRS("\r", 1);
				GPRSStat = GPRS_RUN_Txdata_ACK;
				sGPRSTimeDelay = WAIT_ACK; //两个数据的间隔
			}
			else {
				BeatCnt = 0;
				USART3_SendDataToGPRS((uint8_t*)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut], GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut]);
				USART3_SendDataToGPRS("\r", 1);
				GPRSStat = GPRS_RUN_Txdata_ACK;
				sGPRSTimeDelay = WAIT_ACK;
			}
			GPRSSendFrrorFlg = 1;
			GPRSLoadStatBuf(GPRS_LED_CMD_TO);
			;
		}
		break;
	case GPRS_RUN_Txdata_ACK:
		if (GPRS_ReceiveLength != 0) { //接收到一帧数据
			if (strstr((const char*)GPRS_ReceiveData, "OK") != NULL) //
			{
				GPRSSendFrrorFlg = 0;
				GPRSErrorCnt = 0; //配置成功后清除错误计数
				GPRSStat = GPRS_RUN_Rxdata_CMD;
				sGPRSTimeDelay = NEXT_CMD_DLY; //两个数据的间隔;
				BeatCnt = 0;
				if (GPRSSendBeatDataflg == 0) //发送数据成功
					rmDataFromGPRSTxbuf();
				GPRSLoadStatBuf(GPRS_LED_DATA_OK);
			}
			else {
				if (GPRSErrorCnt < 2) {
					GPRSErrorCnt++;
					GPRSStat = GPRS_RUN_Rxdata_CMD;
					sGPRSTimeDelay = NEXT_CMD_DLY;
					if (HeartTime > NEXT_CMD_DLY)
						BeatCnt = HeartTime - NEXT_CMD_DLY;
					else
						BeatCnt = 0;
				}
				else {
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY; //
				}
				GPRSLoadStatBuf(GPRS_LED_DATA_ERROR);
			}
		}
		else if (sGPRSTimeDelay == 0) {
			if (GPRSErrorCnt < 2) {
				GPRSErrorCnt++;
				GPRSStat = GPRS_RUN_Rxdata_CMD;
				sGPRSTimeDelay = NEXT_CMD_DLY;
				if (HeartTime > NEXT_CMD_DLY)
					BeatCnt = HeartTime - NEXT_CMD_DLY;
				else
					BeatCnt = 0;
			}
			else {
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY; //
			}

			GPRSLoadStatBuf(GPRS_LED_DATA_TO);
		}

		break;
	default: //
		GPRSStat = GPRS_IDLE;
		break;
	}
	return pReceiveLength; //返回接收数据的长度
}

void SendDataToGPRSbuf(char* pDataBuf, uint16_t pLength) {
	uint16_t i;
	if (GPRS_Tx0.TxNum < GPRS_TX_BUF_NUM) {
		GPRS_Tx0.TxNum++;

		if (GPRS_Tx0.TxPtrIn > (GPRS_TX_BUF_NUM - 1)) //指针超出范围时清零
		{
			GPRSInitTxBuf();
		}
		for (i = 0; i < pLength; i++) {
			GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrIn][i] = pDataBuf[i];
		}
		GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrIn] = pLength;

		if (GPRS_Tx0.TxPtrIn < (GPRS_TX_BUF_NUM - 1)) {
			GPRS_Tx0.TxPtrIn++;
		}
		else {
			GPRS_Tx0.TxPtrIn = 0;
		}
	} //数据放满时丢弃
	//等于3时直接退出
}
//只读数据不增加指针，发送正常时再增加指针
/*uint16_t GetDataToGPRSTxbuf(char *pdata)
{
	if(GPRS_Tx0.TxNum > 0){
		pdata = (char *)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut];
		return GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut];
	}else{
		return 0;
	}
}*/
/*数据正常发出后调用本函数，目的是可以多次读同一个缓冲数据，以防读出后数据没有正常发出?
需要此功能还要检测数据发出返回值*/
void rmDataFromGPRSTxbuf(void) {
	if (GPRS_Tx0.TxNum > 0) {
		GPRS_Tx0.TxNum--;
		if (GPRS_Tx0.TxPtrOut >= (GPRS_TX_BUF_NUM - 1)) {
			GPRS_Tx0.TxPtrOut = 0;
		}
		else {
			GPRS_Tx0.TxPtrOut++;
		}
	}
	else if (GPRS_Tx0.TxPtrIn != GPRS_Tx0.TxPtrOut) {
		GPRSInitTxBuf();
	}
}
void GPRSLoadStatBuf(uint16_t pDataBuf) {
	if (GPRSStatBuf0.PtrIn > (GPRS_STAT_BUF_NUM - 1)) //指针超出范围时清零
	{
		GPRSInitTxStatBuf();
	}
	if (GPRSStatBuf0.Num < GPRS_STAT_BUF_NUM) {
		GPRSStatBuf0.Num++;

		GPRSStatBuf0.Buf[GPRSStatBuf0.PtrIn] = pDataBuf;

		if (GPRSStatBuf0.PtrIn < (GPRS_STAT_BUF_NUM - 1)) {
			GPRSStatBuf0.PtrIn++;
		}
		else {
			GPRSStatBuf0.PtrIn = 0;
		}
	} //数据放满时丢弃
	//等于3时直接退出
}
uint16_t GPRSGetStatBuf(void) {
	uint16_t pDataBuf;

	if (GPRSStatBuf0.Num > 0) {
		GPRSStatBuf0.Num--;

		if (GPRSStatBuf0.PtrOut > (GPRS_STAT_BUF_NUM - 1)) //指针超出范围时清零
		{
			GPRSInitTxStatBuf();
		}
		pDataBuf = GPRSStatBuf0.Buf[GPRSStatBuf0.PtrOut];

		if (GPRSStatBuf0.PtrOut < (GPRS_STAT_BUF_NUM - 1)) {
			GPRSStatBuf0.PtrOut++;
		}
		else {
			GPRSStatBuf0.PtrOut = 0;
		}
	}
	else {
		if (GPRSStatBuf0.PtrOut != GPRSStatBuf0.PtrIn) {
			GPRSInitTxStatBuf();
		}
		pDataBuf = GPRS_LED_IDLE;
	}
	return pDataBuf;
	//数据放满时丢弃
	//等于3时直接退出
}
