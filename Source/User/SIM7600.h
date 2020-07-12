#ifndef __SIM7600_H__
#define __SIM7600_H__
#include "stm32f10x.h"

//#define GPRS_IDLE 			(uint16_t)0x0000		//����״̬
//#define GPRS_POWER   		(uint16_t)0x0100		//����
//#define GPRS_CHECK_GPRS		(uint16_t)0x0200		//���GPRS�Ƿ�����
//#define	GPRS_CONNECT_TCP	(uint16_t)0x0300		//����TCP
//define GPRS_RUNNING		(uint16_t)0x0400		//��������
//#define GPRS_ECHO_CLOSE		(uint16_t)0x0500

#define GPRS_IDLE (uint16_t)0x0000 //
#define GPRS_POWER_CMD_SEND (uint16_t)0x0101
#define GPRS_POWER_CMD_ACK (uint16_t)0x0102
#define GPRS_POWER_ON_START (uint16_t)0X0104
#define GPRS_POWER_WAIT_START (uint16_t)0X0106
#define GPRS_POWER_RST (uint16_t)0X0107
#define GPRS_POWER_RST_ACK (uint16_t)0X0108
#define GPRS_APN_CMD_SEND (uint16_t)0X0201
#define GPRS_APN_CMD_ACK (uint16_t)0X0202

#define GPRS_CHECK_CSQ_A (uint16_t)0x0301 //GPRS���Ӽ��
#define GPRS_CHECK_CSQ_ACK_A (uint16_t)0x0302
#define GPRS_CHECK_CMD_SEND (uint16_t)0x0303 //GPRS���Ӽ��
#define GPRS_CHECK_CMD_ACK (uint16_t)0x0304
#define GPRS_CHECK_CSQ_B (uint16_t)0x0305 //GPRS���Ӽ��
#define GPRS_CHECK_CSQ_ACK_B (uint16_t)0x0306
#define GPRS_CHECK_CREG_SEND (uint16_t)0x0307 //��ѯע������״̬
#define GPRS_CHECK_CREG_ACK (uint16_t)0x0308
#define GPRS_CHECK_CPSI_SEND (uint16_t)0x0309 //ע����Ϣ
#define GPRS_CHECK_CPSI_ACK (uint16_t)0x030A

#define GPRS_TCP_conType_SEND (uint16_t)0x0403 //conType
#define GPRS_TCP_conType_ACK (uint16_t)0x0404
#define GPRS_TCP_Name_SEND (uint16_t)0x0405 //APN
#define GPRS_TCP_Name_ACK (uint16_t)0x0406
#define GPRS_TCP_User_SEND (uint16_t)0x0407 //User
#define GPRS_TCP_User_ACK (uint16_t)0x0408
#define GPRS_TCP_Password_SEND (uint16_t)0x0409 //Password
#define GPRS_TCP_Password_ACK (uint16_t)0x040A
#define GPRS_TCP_NETOPEN_SEND (uint16_t)0x040B //NETOPEN
#define GPRS_TCP_NETOPEN_ACK (uint16_t)0x040C
#define GPRS_TCP_BUFFERMODE_SEND (uint16_t)0x040D //BUFFERMODE
#define GPRS_TCP_BUFFERMODE_ACK (uint16_t)0x040E

#define GPRS_TCP_srvType_SEND (uint16_t)0x0501 //srvType
#define GPRS_TCP_srvType_ACK (uint16_t)0x0502
//#define GPRS_TCP_conId_SEND		(uint16_t)0x030B	//conId
//#define GPRS_TCP_conId_ACK		(uint16_t)0x030C
#define GPRS_TCP_IP_SEND (uint16_t)0x0503 //IP
#define GPRS_TCP_IP_ACK (uint16_t)0x0504
#define GPRS_TCP_SISC_SEND (uint16_t)0x0601
#define GPRS_TCP_SISC_ACK (uint16_t)0x0602
#define GPRS_TCP_SISO_SEND (uint16_t)0x0603 //SISO
#define GPRS_TCP_SISO_ACK (uint16_t)0x0604

#define GPRS_RUN_IDLE (uint16_t)0x0701
#define GPRS_RUN_Rxdata_CMD (uint16_t)0x0702
#define GPRS_RUN_Rxdata (uint16_t)0x0703
#define GPRS_RUN_Txdata_CMD (uint16_t)0x0704
#define GPRS_RUN_Txdata_CMD_ACK (uint16_t)0x0707
#define GPRS_RUN_Txdata (uint16_t)0x0705
#define GPRS_RUN_Txdata_ACK (uint16_t)0x0706
#define GPRS_RUN_Txheart_ACK (uint16_t)0x0708
#define WAIT_ACK (uint16_t)10 //6S
#define NEXT_CMD_DLY (uint16_t)20 //100ms
#define WAIT_START (uint16_t)150

#define GPRS_LED_IDLE 0
#define GPRS_LED_START 1
#define GPRS_LED_CMD_ERROR 3
#define GPRS_LED_CMD_TO 4
#define GPRS_LED_DATA 5
#define GPRS_LED_DATA_OK 6
#define GPRS_LED_DATA_ERROR 7
#define GPRS_LED_DATA_TO 8

#define SIZEOF(x) (sizeof(x) - 1)

#define GPRS_TX_BUF_NUM 8
#define GPRS_STAT_BUF_NUM 32

typedef struct {
	uint16_t TxNum;
	uint16_t TxPtrIn;
	uint16_t TxPtrOut;
	uint8_t TxBuf[GPRS_TX_BUF_NUM][64];
	uint16_t TxLength[GPRS_TX_BUF_NUM];

} GPRS_Tx;

typedef struct {
	uint16_t Num;
	uint16_t PtrIn;
	uint16_t PtrOut;
	uint16_t Buf[GPRS_STAT_BUF_NUM];
} GPRSStatBuf;

//extern uint8_t ReceiveData[GPRS_RCV_BUF];
//extern uint8_t ReceiveLength;

//extern uint16_t TimeDelay;
////extern uint8_t FrameFlag;

//extern uint8_t RecDataFlag;
//extern uint8_t GPRS_SysStartCpltFlag; //GPRSģ��������ɱ�־λ
//extern uint8_t BeatFlag; //������������־����1��ʾ���Է���

//extern uint8_t ConfigMode; //Ĭ��Ϊ������ģʽ

//extern uint8_t ConnectTCPFlag; //�ѽ���TCP����

//extern uint32_t HeartTime;

//extern const uint32_t ReConnectTimeOut; //�����������ʱ�䣬Ĭ��Ϊ30����

////����������־λ,Ϊ1ʱ��ʾ����ʱ�䵽��Ĭ��Ϊ1�����һ�ζ��ߺ����������
//extern uint8_t ReConnectFlag;

//extern void Int2Str(uint32_t Data,char *str);
//extern uint32_t Str2Int(char *str);

//extern int my_memcpy(char *address1,char *address2,uint16_t num);

void SIM7600_SW_Init(void);

void SIM7600_Reset(void);

void LED_Toggle(void);

void SIM7600_Delay(uint16_t DelayMs);

uint8_t AT_Cmd_Test(void);
uint8_t SIM7600_Echo_Close(void);

uint8_t ConnectToGPRS(void);

uint8_t DefinePDP(void);

uint8_t ConnectToTCP(void);

uint8_t Send_AT_Command(uint8_t* pCmd, uint8_t WaitTime);

uint16_t ReceiveDataFromGPRS(uint8_t* pRecBuffer);

uint16_t SuperviseTCP(uint8_t* pReveiveBuf);

void SendDataToGPRSbuf(char* pDataBuf, uint16_t pLength);

void GPRS_init(void);

uint16_t GPRSGetStatBuf(void);
void GPRSLoadStatBuf(uint16_t pDataBuf);
#endif //__SIM7600_H__
