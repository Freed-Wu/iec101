#include "SIM7600.h"
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

#define ERRMAXNUM 20

uint32_t HeartTime = 3000; //����������ʱ��Ĭ��ֵΪ30s
uint16_t GPRSStat = GPRS_IDLE;
//uint16_t GPRSSubStat = GPRS_IDLE_SUB;
uint16_t sGPRSTimeDelay;
uint16_t sGPRSSentdataDelay;
uint32_t sGPRSNotCallRstDelay;
uint16_t GPRSErrorCnt = 0;
uint16_t GPRSOpenErrorCnt;
uint16_t GPRSSendBeatDataflg = 0;
uint16_t GPRSFaultcnt; //���ղ������ٻ�������ÿ���ղ������ٻ���λ301��3�θ�λ���ɹ���ػ�����
uint16_t GPRSCheckRstcnt;
uint16_t GPRSSendFrrorFlg; //�������ݴ���ָ��


extern DEVICE_SET user_Set;

//�ڲ���������
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

	GPIO_ResetBits(GSM_PWUP, GSM_PWUP_PIN); //�͵�ѹʱ������Ϊ�ߣ�������

	memset(GPRS_ReceiveData, 0, GPRS_RCV_BUF);
	GPRS_ReceiveLength = 0;

	//�����ǽ����ֲ������ַ���תΪ��������
	//��������ʱ����ַ���תΪ����
	HeartTime = Str2Int((char*)user_Set.heart_time_info);
	HeartTime *= 100; //�����ms

	//��ʼ�����ͻ�����
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
* ����������TCPͨ���������ӣ�һ����⵽�Ͽ���������
* ���������
* ���ز�����
* ˵    ����
****************************************************************************************************
*/

/*******************************************************************************
					����TCP����������
********************************************************************************/

uint16_t SuperviseTCP(uint8_t* pRecBuffer) {
	char* pReceiveData;
	uint8_t AT_Cmd[64] = {0x00};
	uint16_t pReceiveLength;
  uint8_t LengthString[64] = {0x00};
	uint8_t i = 0;
	pReceiveLength = 0;
	GPRS_ReceiveLength = Supervise_USART3(GPRS_ReceiveData); //���յ����ݱ�ʾ
	if (sGPRSTimeDelay > 0)
		sGPRSTimeDelay--;
	if (sGPRSSentdataDelay > 0)
		sGPRSSentdataDelay--;
	if (BeatCnt < HeartTime) {
		BeatCnt += 2; //������20ms   //ԭ��2
	}
	if (GPRSStat > 0x700) { //�����շ�����״̬5����δ�յ����ٻ�ʱ�ط�
		if (sGPRSNotCallRstDelay) {
			sGPRSNotCallRstDelay--;
		}
	}
	else {
		sGPRSNotCallRstDelay = TIME_GPRSNotCallRstDelay; //12����;
	}

	switch (GPRSStat) {
	case GPRS_IDLE:
		if (sGPRSTimeDelay == 0) {
			GPRSErrorCnt = 0;
			memset(GPRS_ReceiveData, 0, GPRS_RCV_BUF); //��ʼ��ReceiveData����
			sGPRSTimeDelay = NEXT_CMD_DLY; //
			GPRSStat = GPRS_POWER_CMD_SEND;
		}
		break;
	case GPRS_POWER_CMD_SEND:
		USART3_InitRXbuf();
		USART3_SendDataToGPRS((uint8_t*)"AT\r", strlen("AT\r")); //�Ƿ񿪻�
		sGPRSTimeDelay = WAIT_ACK;

		GPRSStat = GPRS_POWER_CMD_ACK;
		break;
	case GPRS_POWER_CMD_ACK:
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSStat = GPRS_APN_CMD_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else
				GPRSStat = GPRS_IDLE;
		}
		else if (sGPRSTimeDelay == 0)
			GPRSStat = GPRS_POWER_ON_START;
		break;
	case GPRS_POWER_ON_START:
		if (sGPRSTimeDelay == 0) {
			GPIO_SetBits(GSM_PWUP, GSM_PWUP_PIN); //����GPRSģ�鿪�����ŵ�ƽ
			Delay10ms(50);
			GPIO_ResetBits(GSM_PWUP, GSM_PWUP_PIN);
			sGPRSTimeDelay = WAIT_START; //��ʱ2S����
			GPRSStat = GPRS_POWER_WAIT_START;
			USART3_InitRXbuf();
		}
		break;
	case GPRS_POWER_WAIT_START: //�ϵ�ɹ���᷵��һ���ַ���
		/*if(GPRS_ReceiveLength){//���յ�һ֡����
				if(strstr((const char *)GPRS_ReceiveData,"SYSSTART"))//����״̬
				{
					GPRSStat = GPRS_ECHO_CMD_SEND;
					sGPRSTimeDelay = NEXT_CMD_DLY;
					GPIO_ResetBits(GSM_PWUP,GSM_PWUP_PIN);	//����GPRSģ�鿪�����ŵ�ƽ
				}else if(strstr((const char *)GPRS_ReceiveData,"SHUTDOWN")){
					GPRSStat = GPRS_POWER_ON_START;
					sGPRSTimeDelay = NEXT_CMD_DLY;
					}
				}*/
		if (sGPRSTimeDelay == WAIT_START - 125) {
			GPIO_ResetBits(GSM_PWUP, GSM_PWUP_PIN); //����GPRSģ�鿪�����ŵ�ƽ
			GPRSStat = GPRS_APN_CMD_SEND;
			sGPRSTimeDelay = NEXT_CMD_DLY;
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
			GPIO_ResetBits(GSM_PWUP, GSM_PWUP_PIN); //����GPRSģ�鿪�����ŵ�ƽ
		}
		break;
	case GPRS_POWER_RST:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS((uint8_t*)"AT+CFUN=1,1\r", strlen("AT+CFUN=1,1\r"));
			GPRSStat = GPRS_POWER_RST_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_POWER_RST_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
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
	//����APN
	case GPRS_APN_CMD_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS((uint8_t*)"AT+CGDCONT=1,\"IP\",\"3GNET\"\r", SIZEOF("AT+CGDCONT=1,\"IP\",\"3GNET\"\r"));
			GPRSStat = GPRS_APN_CMD_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_APN_CMD_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSStat = GPRS_CHECK_CSQ_A;
				sGPRSTimeDelay = 200; //��ʱ4S����ź�����
			}
			else {
				GPRSStat = GPRS_IDLE;
			}
		}
		else if (sGPRSTimeDelay == 0) {
			GPRSStat = GPRS_IDLE;
		}
		break;
		//����GPRS-------------------------------------------------------
		//��������ʾ�ź�ǿ��-------------------------------------
	case GPRS_CHECK_CSQ_A:
		if (sGPRSTimeDelay == 0) {
			//GPRSErrorCnt = 0;
			USART3_SendDataToGPRS((uint8_t*)"AT+CSQ\r", strlen("AT+CSQ\r"));
			GPRSStat = GPRS_CHECK_CSQ_ACK_A;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_CHECK_CSQ_ACK_A:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSStat = GPRS_CHECK_CREG_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_CHECK_CREG_SEND; //�Ƿ�����ȷ��������һ��ָ��
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0)
			GPRSStat = GPRS_IDLE;
		break;
	//��ѯע������״̬
	case GPRS_CHECK_CREG_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS((uint8_t*)"AT+CREG?\r", strlen("AT+CREG?\r"));
			GPRSStat = GPRS_CHECK_CREG_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_CHECK_CREG_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSStat = GPRS_CHECK_CPSI_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_CHECK_CPSI_SEND; //�Ƿ�����ȷ��������һ��ָ��
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0)
			GPRSStat = GPRS_IDLE;
		break;
	//��ѯע����Ϣ
	case GPRS_CHECK_CPSI_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS((uint8_t*)"AT+CPSI?\r", strlen("AT+CPSI?\r"));
			GPRSStat = GPRS_CHECK_CPSI_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_CHECK_CPSI_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSStat = GPRS_CHECK_CMD_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_CHECK_CMD_SEND; //�Ƿ�����ȷ��������һ��ָ��
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0)
			GPRSStat = GPRS_IDLE;
		break;
	case GPRS_CHECK_CMD_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS((uint8_t*)"AT+CGREG?\r", strlen("AT+CGREG?\r"));
			GPRSStat = GPRS_CHECK_CMD_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_CHECK_CMD_ACK: //
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "CGREG: 0,1\r\n\r\nOK") || strstr((const char*)GPRS_ReceiveData, "CGREG: 0,5\r\n\r\nOK")) //����״̬
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
					GPRSCheckRstcnt = 0; //������Ч������
					GPRSStat = GPRS_POWER_ON_START;
					sGPRSTimeDelay = 12000; //4���Ӻ�����
				}
			}
		}
		else if (sGPRSTimeDelay == 0)
			GPRSStat = GPRS_IDLE;
		break;
	//����TCP-------------------------------------------------------------------
	case GPRS_TCP_conType_SEND:
		if (sGPRSTimeDelay == 0) {
			GPRSErrorCnt = 0;
			USART3_SendDataToGPRS((uint8_t*)"AT+CIPMODE=0\r", strlen("AT+CIPMODE=0\r"));
			GPRSStat = GPRS_TCP_conType_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_TCP_conType_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSStat = GPRS_TCP_NETOPEN_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0)
			GPRSStat = GPRS_IDLE;
		break;
	//NETOPEN-------------------------------------------------------------------
	case GPRS_TCP_NETOPEN_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS((uint8_t*)"AT+NETOPEN\r", strlen("AT+NETOPEN\r"));
			GPRSStat = GPRS_TCP_NETOPEN_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_TCP_NETOPEN_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSStat = GPRS_TCP_BUFFERMODE_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0)
			GPRSStat = GPRS_IDLE;
		break;

	//BUFFERMODE-------------------------------------------------------------------
	case GPRS_TCP_BUFFERMODE_SEND:
		if (sGPRSTimeDelay == 0) {
			USART3_SendDataToGPRS((uint8_t*)"AT+CIPRXGET=1\r", strlen("AT+CIPRXGET=1\r"));
			GPRSStat = GPRS_TCP_BUFFERMODE_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_TCP_BUFFERMODE_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSStat = GPRS_TCP_Name_SEND;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		else if (sGPRSTimeDelay == 0)
			GPRSStat = GPRS_IDLE;
		break;

	//----------------------TCP--------------------------------
	case GPRS_TCP_Name_SEND: //APN
		if (sGPRSTimeDelay == 0) {
			uint8_t i, j = 0;
			uint8_t APN_Name[100] = "AT+CIPOPEN=2,\"TCP\",\"";
			uint8_t Len1 = 0;
			uint8_t Len2 = 0;
			uint8_t Length = 0;

			i = 0;
			j = 0;
			Len1 = user_Set.ip_len;
			Len2 = user_Set.port_len;
			Length = strlen("AT+CIPOPEN=2,\"TCP\",\"");
			while (Len1--) {
				APN_Name[i + Length] = user_Set.ip_info[i];
				i++;
			}
			APN_Name[i + Length] = '\"';
			i++;
			APN_Name[i + Length] = ',';
			i++;
			while (Len2--) {
				APN_Name[i + Length] = user_Set.port_info[j];
				i++;
				j++;
			}
			APN_Name[i + Length] = '\r';
			i++;

			USART3_SendDataToGPRS(APN_Name, i + Length);
			GPRSStat = GPRS_TCP_Name_ACK;
			sGPRSTimeDelay = WAIT_ACK; //5S
		}
		break;
	case GPRS_TCP_Name_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //����״̬
			{
				GPRSErrorCnt = 0; //���óɹ�������������
				GPRSOpenErrorCnt = 0;
				GPRSInitTxBuf();
#ifdef DEBUG_MODE
				GPRSStat = GPRS_RUN_Txdata_CMD;
#else
				GPRSStat = GPRS_RUN_Rxdata_CMD; //ԭΪGPRS_RUN_Rxdata_CMD
#endif
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
					sGPRSTimeDelay = 50 * 60 * 2; //��2��������
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
			if(GPRS_ReceiveLength){//���յ�һ֡����
				if(strstr((const char *)GPRS_ReceiveData,"OK"))//����״̬
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
			if(GPRS_ReceiveLength){//���յ�һ֡����
				if(strstr((const char *)GPRS_ReceiveData,"OK"))//����״̬
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
			if(GPRS_ReceiveLength){//���յ�һ֡����
				if(strstr((const char *)GPRS_ReceiveData,"OK"))//����״̬
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
				IP_Data[i+Length] = user_Set.ip_info[i]; //��IP���Ƶ������к��ʵ�λ��
				i++;
			}
			IP_Data[i+Length] = ':';
			i++;
			Len = user_Set.port_len;
			while(Len--)
			{
				IP_Data[i+Length] = user_Set.port_info[j]; //���˿���Ϣ���Ƶ�������IP��Ϣ֮��
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
			if(GPRS_ReceiveLength){//���յ�һ֡����
				if(strstr((const char *)GPRS_ReceiveData,"OK"))//����״̬
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
			if(GPRS_ReceiveLength){//���յ�һ֡����
				if(strstr((const char *)GPRS_ReceiveData,"OK"))//����״̬
				{
					GPRSErrorCnt = 0;					//���óɹ�������������
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
						sGPRSTimeDelay = 50 * 60 *2;//��2��������
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
					����������������Ĳ���

**************************************************************************/
	case GPRS_RUN_Rxdata_CMD: //��������ָ��
		//strcpy((char*)AT_Cmd, "AT+CIPSEND=0,\r");
		// uint8_t i = 0;
		//			Int2Str((char*)LengthString, user_Set.heart_len);
		//			while (LengthString[i]) {
		//				AT_Cmd[i + strlen("AT+CIPSEND=0,")] = LengthString[i];
		//				i++;
		//			}
		//   AT_Cmd[i + strlen("AT+CIPSEND=0,")] = '\r';
		//		i++;
		//		AT_Cmd[i + strlen("AT+CIPSEND=0,")] = '\0'; //�ַ���������
		//USART3_SendDataToGPRS(AT_Cmd, strlen((const char*)AT_Cmd));
		//Delay(50);

		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������

		if (ReqGPRSConfigflg) {
			GPRSStat = GPRS_POWER_RST;
			ReqGPRSConfigflg = 0;
		}
		else if (GPRS_Tx0.TxNum > 0) {
			if (sGPRSTimeDelay == 0) {
				GPRSStat = GPRS_RUN_Txdata_CMD;
				USART1_SendData((uint8_t*)"TX NUM", strlen("TX NUM")); //��ʾ�յ�������
				GPRSSendBeatDataflg = 0;
			}
			BeatCnt = 0;
		}
		else if (BeatCnt >= HeartTime) { //��������������ʱ���뷢��״̬
			BeatCnt = 0;
			GPRSSendBeatDataflg = 1;
			GPRSStat = GPRS_RUN_Txdata_CMD;
			USART1_SendData((uint8_t*)"HEART DATA", strlen("HEART DATA")); //��ʾ�յ�������
			sGPRSTimeDelay = NEXT_CMD_DLY;
		}
		else if (sGPRSSentdataDelay == 0) { //2S�Ӷ�һ�Σ�ȥ������������
			USART3_SendDataToGPRS((uint8_t*)"AT+CIPRXGET=2,2\r", strlen("AT+CIPRXGET=2,2\r"));
			GPRSStat = GPRS_RUN_Rxdata;
			sGPRSTimeDelay = WAIT_ACK;
		}
		break; 
	case GPRS_RUN_Rxdata: //��������
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //��������������
			if (strstr((char*)GPRS_ReceiveData, "No data")) {
				if (GPRSFaultcnt<ERRMAXNUM)
				{	
					GPRSFaultcnt++;
					GPRSStat = GPRS_RUN_Rxdata_CMD;
					sGPRSTimeDelay = WAIT_ACK;
				}
				else 
				{
					GPRSFaultcnt=0;
					USART3_SendDataToGPRS((uint8_t*)"AT+CIPCLOSE=2\r", strlen("AT+CIPCLOSE=2\r"));
					GPRSStat = GPRS_POWER_RST;
					sGPRSTimeDelay = NEXT_CMD_DLY;					
				}
			}
			else if (strstr((char*)GPRS_ReceiveData, "ERROR")) {
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
				if (strstr((char*)GPRS_ReceiveData, "+CIPRXGET")) {
					uint8_t i = 0;
					uint16_t len=16;
									
					USART1_SendData((uint8_t*)"jisuan lenth\r", strlen("jisuan lenth\r")); //��ʾ�յ�������
			
					if (GPRS_ReceiveData[36] == '\r') {
						pReceiveLength = (GPRS_ReceiveData[33] - '0');
						for (i = 0; i <pReceiveLength; i++)
							pRecBuffer[i] = GPRS_ReceiveData[i+38];
					}
					else {
						pReceiveLength = (GPRS_ReceiveData[33] - 0x30) * 10 + (GPRS_ReceiveData[34] - 0x30);
						for (i = 0; i < pReceiveLength; i++)
							pRecBuffer[i] =GPRS_ReceiveData[i + 39];
					}
//					pReceiveLength =34;
//						for (i = 0; i <pReceiveLength; i++)
//							pRecBuffer[i] = GPRS_ReceiveData[i+39];
					  //USART1_SendData((char*)len, strlen((const char*)len)); //��ʾ�յ�������
					  //USART1_SendData((char*)pReceiveLength, strlen((char*)pReceiveLength)); //��ʾ�յ�������
					  USART1_SendData(pRecBuffer, pReceiveLength); //��ʾ�յ�������
					  //USART1_SendData((char*)"lenth", strlen("lenth")); //��ʾ�յ�������
				}
				if ((pReceiveLength == 0) && (sGPRSNotCallRstDelay == 0)) {
					if (GPRSFaultcnt < 1) {
						GPRSFaultcnt++;
						USART3_SendDataToGPRS((uint8_t*)"AT+CIPCLOSE=2\r", strlen("AT+CIPCLOSE=2\r"));   //�ȶϿ�����������
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
					if (pReceiveLength) //���յ����ݺ���������־
						GPRSFaultcnt = 0;
					GPRSStat = GPRS_RUN_Rxdata_CMD;
					sGPRSSentdataDelay = 100; //2S�Ӷ�һ�λ���
					sGPRSTimeDelay = NEXT_CMD_DLY;
					if (pReceiveLength)
						sGPRSNotCallRstDelay = TIME_GPRSNotCallRstDelay;
				}
			}
		}
		else if (sGPRSTimeDelay == 0) { //���ճ�ʱ
			if (sGPRSNotCallRstDelay) {
				USART3_SendDataToGPRS((uint8_t*)"AT+CIPCLOSE=2\r", strlen("AT+CIPCLOSE=2\r"));   //�ȶϿ�����������
				GPRSStat = GPRS_POWER_RST;
				sGPRSSentdataDelay = 100; //2S�Ӷ�һ�λ���
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
			else {
				if (GPRSFaultcnt < 1) {
					GPRSFaultcnt++;
					USART3_SendDataToGPRS((uint8_t*)"AT+CIPCLOSE=2\r", strlen("AT+CIPCLOSE=2\r"));   //�ȶϿ�����������
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
	case GPRS_RUN_Txdata_CMD: //��������ָ��
		// uint8_t i = 0;
		// uint8_t AT_Cmd[20] = {0x00};
		// uint16_t pLength;
		// uint8_t LengthString[6] = {0x00};
		strcpy((char*)AT_Cmd, "AT+CIPSEND=2,\r");
		//		if (GPRSSendBeatDataflg) { //������������
		//			strcpy((char*)AT_Cmd, "AT+CIPSEND=1,");
		//			/*Int2Str((char*)LengthString, user_Set.heart_len);
		//			while (LengthString[i]) {
		//				AT_Cmd[i + 10] = LengthString[i];
		//				i++;
		//			}*/
		//		}
		//		else { //����������
		//			BeatCnt = 0;
		//			pLength = GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut];
		//			/*strcpy((char*)AT_Cmd, "AT+CIPSEND=0,");
		//			Int2Str((char*)LengthString, pLength);
		//			while (LengthString[i]) {
		//				AT_Cmd[i + 10] = LengthString[i];
		//				i++;
		//			}*/
		//		}
		// AT_Cmd[i + 13] = '\r';
		/*i++;
		AT_Cmd[i + 10] = '\0'; //�ַ���������*/
		USART3_SendDataToGPRS((uint8_t*)AT_Cmd, strlen((const char*)AT_Cmd));
		//USART3_SendDataToGPRS("AT^SISW=0,5\r",strlen((const char *)"AT^SISW=0,5\r"));
		GPRSStat = GPRS_RUN_Txdata_CMD_ACK;
		//GPRSStat = GPRS_RUN_Txdata;
		sGPRSTimeDelay = WAIT_ACK;
		GPRSLoadStatBuf(GPRS_LED_START);
	break;
	case GPRS_RUN_Txdata_CMD_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) {
			if (strstr((char*)GPRS_ReceiveData, ">")) {
				USART1_SendData((uint8_t*)"data cmd ack ok", strlen("data cmd ack ok")); //��ʾ�յ�������
				GPRSSendFrrorFlg = 0;
				GPRSLoadStatBuf(GPRS_LED_CMD_TO);
				GPRSStat = GPRS_RUN_Txdata;
				sGPRSTimeDelay = WAIT_ACK;
			}
			else if (sGPRSTimeDelay == 0) {
				USART3_SendDataToGPRS((uint8_t*)"AT+CIPCLOSE=2\r", strlen("AT+CIPCLOSE=2\r"));   //�ȶϿ�����������
				GPRSStat = GPRS_POWER_RST;
				GPRSSendFrrorFlg = 1;
				GPRSLoadStatBuf(GPRS_LED_CMD_ERROR);
				sGPRSTimeDelay = NEXT_CMD_DLY;
			}
		}
		break;
	case GPRS_RUN_Txdata: //������ָ��Ӧ���ź�
		//strcpy((char*)AT_Cmd, "hello"); 
		 Int2Str((char*)LengthString, user_Set.heart_len);
		if (GPRSSendBeatDataflg) //������������
		{
			//strcat((char*)AT_Cmd, (char*)user_Set.heart_info);
			//USART1_SendData(AT_Cmd, strlen((char*)AT_Cmd)); //��ʾ�յ�������
			//USART1_SendData((uint8_t*)"heart ok", strlen("heart ok")); //��ʾ�յ�������
		//}
//					Int2Str((char*)LengthString, user_Set.heart_len);
//					while (LengthString[i]) {
//						AT_Cmd[i] = LengthString[i];
//						i++;
//					}
				USART3_SendDataToGPRS(user_Set.heart_info, user_Set.heart_len);
				USART3_SendDataToGPRS("\x1a\r", 2);
				}
		else //����������
			//strcat((char*)AT_Cmd, (char*)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut]);
	    //USART1_SendData((char*)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut], strlen((char*)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut])); //��ʾ�յ�������
		//				USART3_SendDataToGPRS("\r", 1);
				{	
		      BeatCnt = 0;
					
					USART3_SendDataToGPRS((uint8_t*)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut], GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut]);
					USART3_SendDataToGPRS("\x1a\r", 2);
					rmDataFromGPRSTxbuf();
				}
		//		if (GPRS_ReceiveLength) {
		//			//���յ����ݾͿ�ʼ�������ݣ��������Ƿ���ȷ
		//			if (GPRSSendBeatDataflg) {
		//				USART3_SendDataToGPRS(user_Set.heart_info, user_Set.heart_len);
		//				USART3_SendDataToGPRS("\r", 1);
		//				GPRSStat = GPRS_RUN_Txdata_ACK;
		//				sGPRSTimeDelay = WAIT_ACK; //�������ݵļ��
		//			}
		//			else {
		//				BeatCnt = 0;
		//				USART3_SendDataToGPRS((uint8_t*)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut], GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut]);
		//				USART3_SendDataToGPRS("\r", 1);
		//				GPRSStat = GPRS_RUN_Txdata_ACK;
		//				sGPRSTimeDelay = WAIT_ACK;
		//			}
		//			if (strstr((char*)GPRS_ReceiveData, "\r\nOK\r\n") || (strstr((char*)GPRS_ReceiveData, "+CIPSEND:"))) {
		//				GPRSSendFrrorFlg = 0;
		//				GPRSLoadStatBuf(GPRS_LED_DATA); //��������
		//			}
		//			else { //
		//				GPRSSendFrrorFlg = 1;
		//				GPRSLoadStatBuf(GPRS_LED_CMD_ERROR);
		//			}
		//		}
		//		else if (sGPRSTimeDelay == 0) { //����ָ��ʧ��ʱ��Ȼ�������ݣ�����������һ��ָ����ݷ������
		//			if (GPRSSendBeatDataflg) {
		//				USART3_SendDataToGPRS(user_Set.heart_info, user_Set.heart_len);
		//				USART3_SendDataToGPRS("\r", 1);
		//				GPRSStat = GPRS_RUN_Txdata_ACK;
		//				sGPRSTimeDelay = WAIT_ACK; //�������ݵļ��
		//			}
		//			else {
		//				BeatCnt = 0;
		//				USART3_SendDataToGPRS((uint8_t*)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut], GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut]);
		//				USART3_SendDataToGPRS("\r", 1);
		//				GPRSStat = GPRS_RUN_Txdata_ACK;
		//				sGPRSTimeDelay = WAIT_ACK;
		//			}
		//			GPRSSendFrrorFlg = 1;
		//			GPRSLoadStatBuf(GPRS_LED_CMD_TO);
		//			;
		//		}
		
		//USART3_SendDataToGPRS(AT_Cmd, strlen((const char*)AT_Cmd));
		GPRSLoadStatBuf(GPRS_LED_DATA); //��������			
		GPRSStat = GPRS_RUN_Txdata_ACK;
		sGPRSTimeDelay = WAIT_ACK;
		break;
	case GPRS_RUN_Txdata_ACK:
		USART1_SendData(GPRS_ReceiveData, GPRS_ReceiveLength); //��ʾ�յ�������
		if (GPRS_ReceiveLength) { //���յ�һ֡����
			if (strstr((const char*)GPRS_ReceiveData, "OK")) //
			{
				GPRSSendFrrorFlg = 0;
				GPRSErrorCnt = 0; //���óɹ�������������
				GPRSStat = GPRS_RUN_Rxdata_CMD;
				sGPRSTimeDelay = NEXT_CMD_DLY; //�������ݵļ��;
				BeatCnt = 0;
				if (GPRSSendBeatDataflg == 0) //�������ݳɹ�
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
					USART3_SendDataToGPRS((uint8_t*)"AT+CIPCLOSE=2\r", strlen("AT+CIPCLOSE=2\r"));   //�ȶϿ�����������
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
				USART3_SendDataToGPRS((uint8_t*)"AT+CIPCLOSE=2\r", strlen("AT+CIPCLOSE=2\r"));   //�ȶϿ�����������
				GPRSStat = GPRS_POWER_RST;
				sGPRSTimeDelay = NEXT_CMD_DLY; //
			}
			GPRSLoadStatBuf(GPRS_LED_DATA_TO);
		}
		break;
	default: //
		GPRSStat = GPRS_IDLE;
	}
	return pReceiveLength; //���ؽ������ݵĳ���
}

void SendDataToGPRSbuf(char* pDataBuf, uint16_t pLength) {
	uint16_t i;
	if (GPRS_Tx0.TxNum < GPRS_TX_BUF_NUM) {
		GPRS_Tx0.TxNum++;

		if (GPRS_Tx0.TxPtrIn > GPRS_TX_BUF_NUM - 1) //ָ�볬����Χʱ����
			GPRSInitTxBuf();
		for (i = 0; i < pLength; i++)
			GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrIn][i] = pDataBuf[i];
		GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrIn] = pLength;

		if (GPRS_Tx0.TxPtrIn < GPRS_TX_BUF_NUM - 1)
			GPRS_Tx0.TxPtrIn++;
		else
			GPRS_Tx0.TxPtrIn = 0;
	} //���ݷ���ʱ����
	//����3ʱֱ���˳�
}
//ֻ�����ݲ�����ָ�룬��������ʱ������ָ��
/*uint16_t GetDataToGPRSTxbuf(char *pdata)
{
	if(GPRS_Tx0.TxNum > 0){
		pdata = (char *)GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut];
		return GPRS_Tx0.TxLength[GPRS_Tx0.TxPtrOut];
	}else{
		return 0;
	}
}*/
/*����������������ñ�������Ŀ���ǿ��Զ�ζ�ͬһ���������ݣ��Է�����������û����������?
��Ҫ�˹��ܻ�Ҫ������ݷ�������ֵ*/
void rmDataFromGPRSTxbuf(void) {
	if (GPRS_Tx0.TxNum > 0) {
		GPRS_Tx0.TxNum--;
		if (GPRS_Tx0.TxPtrOut >= GPRS_TX_BUF_NUM - 1)
			GPRS_Tx0.TxPtrOut = 0;
		else
			GPRS_Tx0.TxPtrOut++;
	}
	else if (GPRS_Tx0.TxPtrIn != GPRS_Tx0.TxPtrOut)
		GPRSInitTxBuf();
}
void GPRSLoadStatBuf(uint16_t pDataBuf) {
	if (GPRSStatBuf0.PtrIn > GPRS_STAT_BUF_NUM - 1) //ָ�볬����Χʱ����
		GPRSInitTxStatBuf();
	if (GPRSStatBuf0.Num < GPRS_STAT_BUF_NUM) {
		GPRSStatBuf0.Num++;

		GPRSStatBuf0.Buf[GPRSStatBuf0.PtrIn] = pDataBuf;

		if (GPRSStatBuf0.PtrIn < GPRS_STAT_BUF_NUM - 1)
			GPRSStatBuf0.PtrIn++;
		else
			GPRSStatBuf0.PtrIn = 0;
	} //���ݷ���ʱ����
	//����3ʱֱ���˳�
}
uint16_t GPRSGetStatBuf(void) {
	uint16_t pDataBuf;

	if (GPRSStatBuf0.Num > 0) {
		GPRSStatBuf0.Num--;

		if (GPRSStatBuf0.PtrOut > GPRS_STAT_BUF_NUM - 1) //ָ�볬����Χʱ����
			GPRSInitTxStatBuf();
		pDataBuf = GPRSStatBuf0.Buf[GPRSStatBuf0.PtrOut];

		if (GPRSStatBuf0.PtrOut < GPRS_STAT_BUF_NUM - 1)
			GPRSStatBuf0.PtrOut++;
		else
			GPRSStatBuf0.PtrOut = 0;
	}
	else {
		if (GPRSStatBuf0.PtrOut != GPRSStatBuf0.PtrIn)
			GPRSInitTxStatBuf();
		pDataBuf = GPRS_LED_IDLE;
	}
	return pDataBuf;
	//���ݷ���ʱ����
	//����3ʱֱ���˳�
}
