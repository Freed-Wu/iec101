//#include "101_Protocol.h"
#include "SIM7600.h"
#include "conf_USART.h"
#include "ds3231.h"
#include "flash.h"
#include "main.h"
#include "modbus.h"
#include "stm32f10x.h"

uint16_t LINK_ADDRESS = 0x0021; //定义链路地址
uint8_t ProtocolRxBuffer[64] = {0}; //存储主站发过来的命令
uint8_t TxBuffer[64] = {0};
uint8_t TxAppBuffer[64] = {0};
uint8_t RxControlField = 0; //接收到数据中的控制域
uint8_t RxDIR, RxPRM, RxFCB, RxFCV, RxFunctionCode;
uint8_t LastFCB;
uint8_t TxDIR, TxPRM, TxFCB, TxFCV, TxFunctionCode;
TimeStructure NowTimeStruct;

extern uint8_t Info[16]; //开关量数量 0/1/2 跌落 3跌落状态 4温度状态 5 欠压 6/7/8/9 漏保
extern uint8_t InfoTemp[8]; //温度
extern uint8_t DataFromGPRSBuffer[128];
extern uint8_t moduleMaskEn; // 模块故障MASK
/*
 ****************************************************************************************************
 * 功能描述：初始化链路层参数
 * 输入参数：
 * 返回参数：
 * 说    明：
 ****************************************************************************************************
 */
void LinkInit(void) {
	RxDIR = M2S_DIR; //固定值，从站接收到主站的数据时DIR为0，
	RxPRM = 0; //非固定值
	TxPRM = 0; //非固定值
	RxFCB = 0;
	RxFCV = 0;
	TxDIR = S2M_DIR; //固定值从站发出数据时DIR为1
}

//此函数用于CheckSum函数之后，即已经判断为有效帧后再校验地址是否正确
//返回SUCCESS地址正确，返回ERROR地址不正确
uint8_t CheckLinkAddress(uint8_t* pBuffer) {
	uint16_t LinkAddressLow = 0;
	uint16_t LinkAddressHigh = 0;
	uint16_t LinkAddress = 0;

	if (pBuffer[0] == 0x10) {
		LinkAddressLow = (uint16_t)pBuffer[2];
		LinkAddressHigh = (uint16_t)pBuffer[3];
	}
	else if (pBuffer[0] == 0x68) {
		LinkAddressLow = (uint16_t)pBuffer[5];
		LinkAddressHigh = (uint16_t)pBuffer[6];
	}
	else {
		return ERROR; //链路地址不正确
	}

	LinkAddress = LinkAddressHigh << 8;
	LinkAddress += LinkAddressLow;

	if (LinkAddress == LINK_ADDRESS) {
		return SUCCESS; //链路地址正确
	}
	else {
		return ERROR; //链路地址不正确
	}
}

/*
 ****************************************************************************************************
 * 功能描述：
 * 输入参数：
 * 返回参数：校验和的值
 * 说    明：
 ****************************************************************************************************
 */
uint8_t GetCheckSum(uint8_t* pBuffer) {
	uint8_t i;
	uint8_t TempSum = 0;
	uint8_t DataLength = 0;

	if (pBuffer[0] == 0x10)
		TempSum = pBuffer[1] + pBuffer[2] + pBuffer[3];

	else if (pBuffer[0] == 0x68) {
		DataLength = pBuffer[1]; //获取可变帧数据长度

		for (i = 0; i < DataLength; i++)
			TempSum += pBuffer[i + 4];
	}
	else
		TempSum = 0;

	return TempSum;
	//////////////////////////////////////////////////////////////////////////
}

/*
 ****************************************************************************************************
 * 功能描述：
 * 输入参数：
 * 返回参数：
 * 说    明：
 ****************************************************************************************************
 */
uint8_t CheckError(uint8_t* pBuffer) {
	uint8_t Length1, Length2;

	if ((pBuffer[0] == 0x68) && (pBuffer[3] == 0x68)) {
		//可变帧长
		Length1 = pBuffer[1];
		Length2 = pBuffer[2];

		if (Length1 != Length2)
			return ERROR; //两个长度字节不相等,返回错误
		else
			//校验和相等且最后一个字节为0x16
			if ((GetCheckSum(pBuffer) == pBuffer[4 + Length1]) && (pBuffer[5 + Length1] == 0x16)) {
			RxControlField = pBuffer[4]; //帧校验成功后取得控制域数据
			return SUCCESS;
		}
		else
			return ERROR;
	}

	/* 固定帧长第一字节和最后字节校验 */
	else if ((pBuffer[0] == 0x10) && (pBuffer[5] == 0x16)) {
		//检验和是否相等
		if (GetCheckSum(pBuffer) == pBuffer[4]) {
			RxControlField = pBuffer[1]; //取得控制域数据
			return SUCCESS;
		}
		else
			return ERROR;
	}
	else
		//无效帧，丢弃
		return ERROR;
}

/*
 ****************************************************************************************************
 * 功能描述：
 * 输入参数：
 * 返回参数：
 * 说    明：
 ****************************************************************************************************
 */
uint8_t Protocol101_RxLink(void) {
	RxDIR = (RxControlField >> 7) & 0x01; //取得DIR位
	RxPRM = (RxControlField >> 6) & 0x01; //取得PRM位
	RxFCB = (RxControlField >> 5) & 0x01; //取得FCB位
	RxFCV = (RxControlField >> 4) & 0x01; //取得FCV位
	RxFunctionCode = RxControlField & 0x0F; //取得功能码

	if ((RxDIR == M2S_DIR) && (RxPRM == 1))
		return SUCCESS; //接收标志位判断
	else {
		// newFrame = 1;
		LastFCB = RxFCB;
		return ERROR;
	}
}

/*
 ****************************************************************************************************
 * 功能描述：
 * 输入参数：
 * 返回参数：
 * 说    明：
 ****************************************************************************************************
 */
uint8_t SetTxControlField(uint8_t PRM, uint8_t FCB, uint8_t FCV, uint8_t FuncCode) {
	uint8_t Temp = 0;
	Temp |= ((S2M_DIR & 0x01) << 7);
	Temp |= ((PRM & 0x01) << 6);
	Temp |= ((FCB & 0x01) << 5);
	Temp |= ((FCV & 0x01) << 4);
	Temp |= (FuncCode & 0x0F);
	return Temp;
}

void SendStableData(uint8_t PRM, uint8_t FCB, uint8_t FCV, uint8_t FuncCode) {
	uint8_t TxCrtlField = 0;
	uint8_t CheckSum = 0;
	uint8_t FrameData[6] = {0x00};
	TxCrtlField = SetTxControlField(PRM, FCB, FCV, FuncCode);
	FrameData[0] = 0x10;
	FrameData[1] = TxCrtlField;
	FrameData[2] = LINK_ADDRESS & 0xFF;
	FrameData[3] = LINK_ADDRESS >> 8;
	CheckSum = FrameData[1] + FrameData[2] + FrameData[3];
	FrameData[4] = CheckSum;
	FrameData[5] = 0x16;
	SendDataToGPRSbuf((char*)FrameData, 6); //串口发送
}

#if 0
void SendVariableData( uint8_t PRM, uint8_t FCB, uint8_t FCV, uint8_t FuncCode, uint8_t* pBuffer )
{
	ASDU_DataStructure ASDU_DataStruct;
	ASDU_DataStruct.TypeID = 100;
	ASDU_DataStruct.Qualifier = 1;
	ASDU_DataStruct.Reason = 7;
	ASDU_DataStruct.ASDU_Address = LINK_ADDRESS;
	ASDU_DataStruct.InfoAddress1 = 0x0000;
	ASDU_DataStruct.InfoData1 = 0X14;
	ASDU_Init( ASDU_DataStruct, TxAppBuffer );
}
#endif

#if 0
/*
 ****************************************************************************************************
 * 功能描述：
 * 输入参数：TxAppLength:应用层数据不为0，指的是ASDU的长度，发送的是可变帧长数据
 *           pTxBuffer:链路层数据缓存，最后通过物理通道发送出去
 *           pTxAppBuffer:应用层数据缓冲区，会在应用层将数据转移到pTxBuffer中
 * 返回参数：无
 * 说    明：
 ****************************************************************************************************
 */
void Protocol101_TxLink( uint8_t TxAppLength, uint8_t* pTxBuffer, uint8_t* pTxAppBuffer )
{
	uint8_t i;
	uint8_t TempSum = 0;
	uint8_t TxCounter = 0;

	//如果应用层数据不为0，则表示发送的是可变帧长数据
	if ( TxAppLength != 0 ) {
		pTxBuffer[0] = 0x68;
		pTxBuffer[1] = TxAppLength + 3;
		pTxBuffer[2] = TxAppLength + 3;
		pTxBuffer[3] = 0x68;
		pTxBuffer[4] = SetTxControlField();
		TempSum += pTxBuffer[4];
		pTxBuffer[5] = LINK_ADDRESS & 0xff;
		TempSum += pTxBuffer[5];
		pTxBuffer[6] = ( LINK_ADDRESS >> 8 ) & 0xff;
		TempSum += pTxBuffer[6];
		TxCounter = 7;

		for ( i = 0; i <= TxAppLength - 1; i++ ) {
			pTxBuffer[TxCounter] = pTxAppBuffer[i];
			TempSum += pTxBuffer[TxCounter++];
		}

		TempSum &= 0xff;
		pTxBuffer[TxCounter++] = TempSum;
		pTxBuffer[TxCounter++] = 0x16;

	} else {
		pTxBuffer[0] = 0x10;
		pTxBuffer[1] = GetTxControlField();
		pTxBuffer[2] = LINK_ADDRESS & 0xff;
		pTxBuffer[3] = ( LINK_ADDRESS >> 8 ) & 0xff;

		//计算校验和
		for ( i = 1; i < 4; i++ )
			TempSum += pTxBuffer[i];

		pTxBuffer[4] = TempSum;
		pTxBuffer[5] = 0x16;
		TxCounter = 6;
	}

	USART_SendString( pTxBuffer, TxCounter ); //串口发送
}

#endif
uint8_t ASDU_Init(ASDU_DataStructure* ASDU_Struct, uint8_t NumOfInfo, uint8_t* pBuffer) {
	uint8_t i = 0;
	uint8_t ASDU_Length = 0;
	uint8_t ASDU_AddressLow = 0;
	uint8_t ASDU_AddressHigh = 0;
	ASDU_AddressLow = LINK_ADDRESS;
	ASDU_AddressHigh = (LINK_ADDRESS >> 8);
	pBuffer[0] = ASDU_Struct->TypeID;
	pBuffer[1] = ASDU_Struct->Qualifier;
	pBuffer[2] = ASDU_Struct->Reason;
	pBuffer[3] = ASDU_AddressLow;
	pBuffer[4] = ASDU_AddressHigh;
	pBuffer[5] = ASDU_Struct->InfoAddress & 0xFF;
	pBuffer[6] = ASDU_Struct->InfoAddress >> 8;
	ASDU_Length = 7;

	while (NumOfInfo--) {
		pBuffer[i + 7] = ASDU_Struct->InfoData[i];
		ASDU_Length += 1;
		i++;
	}

	return ASDU_Length;
}

#if 0
/*
 ****************************************************************************************************
 * 功能描述：
 * 输入参数：
 *
 *

 *
 * 返回参数：
 * 说    明：
 ****************************************************************************************************
 */
void Protocol101_TxApp( uint8_t TypeID, uint8_t Qual, uint8_t SendReason, uint8_t Func, uint8_t TxAppLength )
{
	TxFunctionCode = Func & 0x0F;

	if ( TxAppLength != 0 ) {
		TxAppBuffer[0] = TypeID;                            //类型标识
		TxAppBuffer[1] = Qual;    //信息体数量
		TxAppBuffer[2] = ( SendReason & 0xff );
		TxAppBuffer[3] = LINK_ADDRESS & 0xFF;         //公共地址低位
		TxAppBuffer[4] = ( LINK_ADDRESS >> 8 ) & 0xFF;  //公共地址高位
	}

	Protocol101_TxLink( TxAppLength, TxBuffer, TxAppBuffer );
}

#endif
uint8_t ResponseLinkStatus(void) {
	uint8_t Temp = 11;
	// txAppLength = 0;
	// i = linkState;
	Temp &= 0x0F;

	switch (Temp) {
	case 11:
		TxFCB = 0;
		SendStableData(0, 0, 0, 11);
		// Protocol101_TxApp(0,0,0,0x0b,0);		/* 0x0b 链路状态响应 */
		break;

	case 14:
		// Protocol101_TxApp(0,0,0,0x0e,0);		/* 0x0e 链路服务未工作 */
		break;

	default:
		// Protocol101_TxApp(0,0,0,0x0f,0);		/* 0x0f 链路服务未完成 */
		break;
	}

	// linkState = 0;
	return 1;
}

//主站复位远方链路
uint8_t ResponseResetRemoteLink(void) {
	SendStableData(0, 0, 0, 0);
	return SUCCESS;
}

//确认总召唤
uint8_t ResponseCallAll(void) {
	uint8_t i = 0;
	uint8_t TxCtrlField = 0;
	uint8_t TempCallAllBuf[64] = {0x00};
	uint8_t TempASDU_Buf[32] = {0x00};
	uint8_t ASDU_Length = 0; // ASDU的长度，并非可变帧长中的L值
	// uint8_t SendLength = 0; //可变帧长总长度，最后通过串口发送
	uint8_t CheckSum = 0;
	ASDU_DataStructure ASDU_DataStruct;
	//获取控制域的值
	TxCtrlField = SetTxControlField(0, 0, 0, 0x00);
	ASDU_DataStruct.TypeID = 0x64;
	ASDU_DataStruct.Qualifier = 1;
	ASDU_DataStruct.Reason = 7;
	ASDU_DataStruct.ASDU_Address = LINK_ADDRESS;
	ASDU_DataStruct.InfoAddress = 0x0000;
	ASDU_DataStruct.InfoData[0] = 0x14;
	ASDU_Length = ASDU_Init(&ASDU_DataStruct, 1,
							TempASDU_Buf); //填充ASDU数据并返回ASDU的长度（此值并非L）
	TempCallAllBuf[0] = 0x68;
	TempCallAllBuf[1] = ASDU_Length + 3;
	TempCallAllBuf[2] = ASDU_Length + 3;
	TempCallAllBuf[3] = 0x68;
	TempCallAllBuf[4] = TxCtrlField;
	TempCallAllBuf[5] = LINK_ADDRESS & 0xFF;
	TempCallAllBuf[6] = LINK_ADDRESS >> 8;

	for (i = 0; i < ASDU_Length; i++)
		TempCallAllBuf[i + 7] = TempASDU_Buf[i];

	for (i = 0; i < (ASDU_Length + 3); i++)
		CheckSum += TempCallAllBuf[i + 4]; //计算校验和

	TempCallAllBuf[ASDU_Length + 7] = CheckSum;
	TempCallAllBuf[ASDU_Length + 8] = 0x16;
	SendDataToGPRSbuf((char*)TempCallAllBuf, (ASDU_Length + 9));
	return 1;
}

void UpdateDataForCallAll(void) {
	uint8_t i = 0;
	uint8_t TxCtrlField = 0;
	uint8_t TempCallAllBuf[64] = {0x00};
	uint8_t TempASDU_Buf[32] = {0x00};
	uint8_t ASDU_Length = 0; // ASDU的长度，并非可变帧长中的L值
	// uint8_t SendLength = 0; //可变帧长总长度，最后通过串口发送
	uint8_t CheckSum = 0;
	ASDU_DataStructure ASDU_DataStruct;
	//获取控制域的值
	TxCtrlField = SetTxControlField(0, 0, 0, 0x00);
	ASDU_DataStruct.TypeID = 0x1; //无时标的单点信息
	ASDU_DataStruct.Qualifier = 0x8a; //单地址，6个数据
	ASDU_DataStruct.Reason = 0x14; //响应总召唤
	ASDU_DataStruct.ASDU_Address = LINK_ADDRESS;
	ASDU_DataStruct.InfoAddress = 0x0001;

	if (moduleMaskEn == 0) { //非屏蔽状态及时发送状态
		if (CheckInfoCRCIsOK() == 0) { //读数据之前先检测RAM数据的有校性，CRC失败时则从FLASH中重新读取
			FLASH_RD_Module_Status();
			RefreshInfoCRC();
		}

		ASDU_DataStruct.InfoData[0] = Info[0];
		ASDU_DataStruct.InfoData[1] = Info[1];
		ASDU_DataStruct.InfoData[2] = Info[2];
		ASDU_DataStruct.InfoData[3] = Info[3];
		ASDU_DataStruct.InfoData[4] = Info[4];
		ASDU_DataStruct.InfoData[5] = Info[5];
		ASDU_DataStruct.InfoData[6] = Info[6]; //漏保1
		ASDU_DataStruct.InfoData[7] = Info[7]; //漏保2
		ASDU_DataStruct.InfoData[8] = Info[8]; //漏保3
		ASDU_DataStruct.InfoData[9] = Info[9]; //漏保4
	}
	else {
		ASDU_DataStruct.InfoData[0] = 0; //屏蔽状态都按正常处理
		ASDU_DataStruct.InfoData[1] = 0; //屏蔽状态都按正常处理
		ASDU_DataStruct.InfoData[2] = 0; //屏蔽状态都按正常处理
		ASDU_DataStruct.InfoData[3] = 0; //屏蔽状态都按正常处理
		ASDU_DataStruct.InfoData[4] = 0; //屏蔽状态都按正常处理
		ASDU_DataStruct.InfoData[5] = 0; //屏蔽状态都按正常处理
		ASDU_DataStruct.InfoData[6] = 0; //漏保1
		ASDU_DataStruct.InfoData[7] = 0; //漏保2
		ASDU_DataStruct.InfoData[8] = 0; //漏保3
		ASDU_DataStruct.InfoData[9] = 0; //漏保4
	}

	TxCtrlField = SetTxControlField(0, 0, 0, 0x00);
	ASDU_Length = ASDU_Init(&ASDU_DataStruct, 10,
							TempASDU_Buf); //填充ASDU数据并返回ASDU的长度（此值并非L）
	TempCallAllBuf[0] = 0x68;
	TempCallAllBuf[1] = ASDU_Length + 3;
	TempCallAllBuf[2] = ASDU_Length + 3;
	TempCallAllBuf[3] = 0x68;
	TempCallAllBuf[4] = TxCtrlField;
	TempCallAllBuf[5] = LINK_ADDRESS & 0xFF;
	TempCallAllBuf[6] = LINK_ADDRESS >> 8;

	for (i = 0; i < ASDU_Length; i++)
		TempCallAllBuf[i + 7] = TempASDU_Buf[i];

	for (i = 0; i < (ASDU_Length + 3); i++)
		CheckSum += TempCallAllBuf[i + 4]; //计算校验和

	TempCallAllBuf[ASDU_Length + 7] = CheckSum;
	TempCallAllBuf[ASDU_Length + 8] = 0x16;
	SendDataToGPRSbuf((char*)TempCallAllBuf, (ASDU_Length + 9));
}

///遥测1:温度
void UpdateTempForCallAll(void) {
	uint8_t i = 0;
	uint8_t TempCallAllBuf[64] = {0x00};
	uint8_t TxCtrlField = 0;
	ASDU_DataStructure ASDU_DataStruct;
	uint8_t TempASDU_Buf[32] = {0x00};
	uint8_t ASDU_Length = 0; // ASDU的长度，并非可变帧长中的L值
	uint8_t CheckSum = 0;
	//获取控制域的值
	TxCtrlField = SetTxControlField(0, 0, 0, 0x00);
	ASDU_DataStruct.TypeID = 0x15; //类型标示，不带品质因数的遥测量
	ASDU_DataStruct.Qualifier = 0x83; //品质因数，单地址3个数据
	ASDU_DataStruct.Reason = 0x14; //响应总召
	ASDU_DataStruct.ASDU_Address = LINK_ADDRESS;
	ASDU_DataStruct.InfoAddress = 0x0001;

	if (moduleMaskEn == 0) { //非屏蔽状态及时发送状态
		ASDU_DataStruct.InfoData[0] = InfoTemp[0];
		ASDU_DataStruct.InfoData[1] = InfoTemp[1];
		ASDU_DataStruct.InfoData[2] = InfoTemp[2];
	}
	else {
		ASDU_DataStruct.InfoData[0] = 20; //屏蔽状态都按正常处理
		ASDU_DataStruct.InfoData[1] = 20; //屏蔽状态都按正常处理
		ASDU_DataStruct.InfoData[2] = 20; //屏蔽状态都按正常处理
	}

	TxCtrlField = SetTxControlField(0, 0, 0, 0x00);
	ASDU_Length = ASDU_Init(&ASDU_DataStruct, 3,
							TempASDU_Buf); //填充ASDU数据并返回ASDU的长度（此值并非L）
	TempCallAllBuf[0] = 0x68;
	TempCallAllBuf[1] = ASDU_Length + 3;
	TempCallAllBuf[2] = ASDU_Length + 3;
	TempCallAllBuf[3] = 0x68;
	TempCallAllBuf[4] = TxCtrlField;
	TempCallAllBuf[5] = LINK_ADDRESS & 0xFF;
	TempCallAllBuf[6] = LINK_ADDRESS >> 8;

	for (i = 0; i < ASDU_Length; i++)
		TempCallAllBuf[i + 7] = TempASDU_Buf[i];

	for (i = 0; i < (ASDU_Length + 3); i++)
		CheckSum += TempCallAllBuf[i + 4]; //计算校验和

	TempCallAllBuf[ASDU_Length + 7] = CheckSum;
	TempCallAllBuf[ASDU_Length + 8] = 0x16;
	SendDataToGPRSbuf((char*)TempCallAllBuf, (ASDU_Length + 9));
}

void EndOfCallAll(void) {
	uint8_t i = 0;
	uint8_t TxCtrlField = 0;
	uint8_t TempCallAllBuf[64] = {0x00};
	uint8_t TempASDU_Buf[32] = {0x00};
	uint8_t ASDU_Length = 0; // ASDU的长度，并非可变帧长中的L值
	// uint8_t SendLength = 0; //可变帧长总长度，最后通过串口发送
	uint8_t CheckSum = 0;
	ASDU_DataStructure ASDU_DataStruct;
	//获取控制域的值
	TxCtrlField = SetTxControlField(0, 0, 0, 0x00);
	ASDU_DataStruct.TypeID = 0x64;
	ASDU_DataStruct.Qualifier = 0x01;
	ASDU_DataStruct.Reason = 0x0A;
	ASDU_DataStruct.ASDU_Address = LINK_ADDRESS;
	ASDU_DataStruct.InfoAddress = 0x0000;
	ASDU_DataStruct.InfoData[0] = 0x14;
	ASDU_Length = ASDU_Init(&ASDU_DataStruct, 1,
							TempASDU_Buf); //填充ASDU数据并返回ASDU的长度（此值并非L）
	TempCallAllBuf[0] = 0x68;
	TempCallAllBuf[1] = ASDU_Length + 3;
	TempCallAllBuf[2] = ASDU_Length + 3;
	TempCallAllBuf[3] = 0x68;
	TempCallAllBuf[4] = TxCtrlField;
	TempCallAllBuf[5] = LINK_ADDRESS & 0xFF;
	TempCallAllBuf[6] = LINK_ADDRESS >> 8;

	for (i = 0; i < ASDU_Length; i++)
		TempCallAllBuf[i + 7] = TempASDU_Buf[i];

	for (i = 0; i < (ASDU_Length + 3); i++)
		CheckSum += TempCallAllBuf[i + 4]; //计算校验和

	TempCallAllBuf[ASDU_Length + 7] = CheckSum;
	TempCallAllBuf[ASDU_Length + 8] = 0x16;
	SendDataToGPRSbuf((char*)TempCallAllBuf, (ASDU_Length + 9));
}

void GetClockFromServer(TimeStructure* TimeStruct, uint8_t* pBuffer) {
	uint16_t Temp = 0;
	Temp = pBuffer[15] << 8;
	Temp += pBuffer[14];
	TimeStruct->MilliSec = (Temp % 1000);
	TimeStruct->Sec = (uint8_t)(Temp / 1000);
	TimeStruct->Min = pBuffer[16];
	TimeStruct->Hour = pBuffer[17];
	TimeStruct->Day = pBuffer[18];
	TimeStruct->Month = pBuffer[19];
	TimeStruct->Year = pBuffer[20];
}

// InfoAdress -- 上单元编号：1~6
// Info -- 上单元状态：0-正常；1-跌落
//*Time -- 时间结构体

void ChangeUpdate(uint16_t InfoAdress, uint8_t Info, TimeStructure* Time) {
	uint8_t i;
	uint8_t CheckSum = 0;
	uint8_t InfoArray[32] = {0x00};
	// static uint8_t PreInfo = 0;
	// static uint16_t PreInfoAdress = 0;
	// if ((PreInfoAdress != InfoAdress) || (PreInfo != Info))
	//{
	InfoArray[0] = 0x68;
	InfoArray[1] = 0x12;
	InfoArray[2] = 0x12;
	InfoArray[3] = 0x68;
	InfoArray[4] = SetTxControlField(1, 0, 0, 3);
	InfoArray[5] = LINK_ADDRESS & 0xFF;
	InfoArray[6] = LINK_ADDRESS >> 8;
	InfoArray[7] = 0x1E; // 0x1E==30,带CP56Time2a时标的单点信息
	InfoArray[8] = 0x01;
	InfoArray[9] = 0x03; //传送原因:突发
	InfoArray[10] = LINK_ADDRESS & 0xFF;
	InfoArray[11] = LINK_ADDRESS >> 8;
	InfoArray[12] = InfoAdress & 0xFF;
	InfoArray[13] = InfoAdress >> 8;
	InfoArray[14] = Info;
	InfoArray[15] = (uint8_t)((Time->Sec * 1000) + Time->MilliSec);
	InfoArray[16] = ((Time->Sec * 1000) + Time->MilliSec) >> 8;
	InfoArray[17] = Time->Min;
	InfoArray[18] = Time->Hour;
	InfoArray[19] = Time->Day;
	InfoArray[20] = Time->Month;
	InfoArray[21] = Time->Year;

	for (i = 0; i < 0x12; i++)
		CheckSum += InfoArray[i + 4];

	InfoArray[22] = CheckSum;
	InfoArray[23] = 0x16;
	//   PreInfoAdress = InfoAdress;
	//   PreInfo = Info;
	SendDataToGPRSbuf((char*)InfoArray, 24);
	// }
}

void TempChangeUpdate(uint16_t InfoAdress, uint8_t Info, TimeStructure* Time) {
	uint8_t i;
	uint8_t CheckSum = 0;
	uint8_t InfoArray[32] = {0x00};
	static uint8_t PreInfo = 0;
	static uint16_t PreInfoAdress = 0;
	ReadDATATime();

	if ((PreInfoAdress != InfoAdress) || (PreInfo != Info)) {
		InfoArray[0] = 0x68;
		InfoArray[1] = 0x12;
		InfoArray[2] = 0x12;
		InfoArray[3] = 0x68;
		InfoArray[4] = SetTxControlField(1, 0, 0, 3);
		InfoArray[5] = LINK_ADDRESS & 0xFF;
		InfoArray[6] = LINK_ADDRESS >> 8;
		InfoArray[7] = 0x23; // 0x23==35,带CP56Time2a时标的测量标量值
		InfoArray[8] = 0x01;
		InfoArray[9] = 0x03; //传送原因:突发
		InfoArray[10] = LINK_ADDRESS & 0xFF;
		InfoArray[11] = LINK_ADDRESS >> 8;
		InfoArray[12] = InfoAdress & 0xFF;
		InfoArray[13] = InfoAdress >> 8;
		InfoArray[14] = Info;
		InfoArray[15] = (uint8_t)((Time->Sec * 1000) + Time->MilliSec);
		InfoArray[16] = ((Time->Sec * 1000) + Time->MilliSec) >> 8;
		InfoArray[17] = Time->Min;
		InfoArray[18] = Time->Hour;
		InfoArray[19] = Time->Day;
		InfoArray[20] = Time->Month;
		InfoArray[21] = Time->Year;

		for (i = 0; i < 0x12; i++)
			CheckSum += InfoArray[i + 4];

		InfoArray[22] = CheckSum;
		InfoArray[23] = 0x16;
		PreInfoAdress = InfoAdress;
		PreInfo = Info;
		SendDataToGPRSbuf((char*)InfoArray, 24);
	}
}

void ResponseTimeSynchronous(void) {
	uint8_t i = 0;
	uint8_t CheckSum = 0;
	uint16_t Temp = 0;
	uint8_t Time[32] = {0x00};
	Temp = NowTimeStruct.Sec * 1000 + NowTimeStruct.MilliSec;
	Time[0] = 0x68;
	Time[1] = 0x11;
	Time[2] = 0x11;
	Time[3] = 0x68;
	Time[4] = SetTxControlField(0, 0, 0, 0);
	Time[5] = LINK_ADDRESS & 0xFF;
	Time[6] = LINK_ADDRESS >> 8;
	Time[7] = 0x67;
	Time[8] = 0x01;
	Time[9] = 0x07; //激活确认
	Time[10] = LINK_ADDRESS & 0xFF;
	Time[11] = LINK_ADDRESS >> 8;
	Time[12] = 0x00;
	Time[13] = 0x00;
	Time[14] = Temp & 0xFF;
	Time[15] = Temp >> 8;
	Time[16] = NowTimeStruct.Min;
	Time[17] = NowTimeStruct.Hour;
	Time[18] = NowTimeStruct.Day;
	Time[19] = NowTimeStruct.Month;
	Time[20] = NowTimeStruct.Year;

	for (i = 0; i < 17; i++)
		CheckSum += Time[i + 4];

	Time[21] = CheckSum;
	Time[22] = 0x16;
	SendDataToGPRSbuf((char*)Time, 23);
}
/*
   更改这个结构，以实现通过GPRS发送一个数据时
   */
uint8_t DataProcess(void) {
	uint8_t TempRxFunctionCode = 0;

	if (CheckLinkAddress(DataFromGPRSBuffer) == SUCCESS) {
		if (CheckError(DataFromGPRSBuffer) == SUCCESS) {
			//数据校验通过，可以开始处理数据
			if (Protocol101_RxLink() == SUCCESS) {
				TempRxFunctionCode = RxFunctionCode;
        USART1_SendData((char*)TempRxFunctionCode, strlen((char*)TempRxFunctionCode)); //显示收到的数据
				switch (TempRxFunctionCode) {
				case 0: //主站复位远方链路
					ResponseResetRemoteLink();
					break;

				case 1: //主站复位用户进程
					break;

				case 2: //发送\确认链路测试功能
					break;

				case 3: //发送\确认用户数据
					if ((DataFromGPRSBuffer[7] == 0x64) && ((DataFromGPRSBuffer[14] == 0x14) || (DataFromGPRSBuffer[15] == 0x14))) {
						ResponseCallAll();
						// SysDelay(1000);
						UpdateDataForCallAll();
						UpdateTempForCallAll(); // SysDelay(1000);
						EndOfCallAll();
					}

					if ((DataFromGPRSBuffer[7] == 0x67)) {
						//获取时间
						GetClockFromServer(&NowTimeStruct, ProtocolRxBuffer);
						//将时间写入时钟芯片
						Time_Data[0] = NowTimeStruct.Year;
						Time_Data[1] = NowTimeStruct.Month;
						Time_Data[2] = NowTimeStruct.Day;
						Time_Data[3] = NowTimeStruct.Hour;
						Time_Data[4] = NowTimeStruct.Min;
						Time_Data[5] = NowTimeStruct.Sec;
						WriteDATATime();
						//回复确认帧
						ResponseTimeSynchronous();
					}

					break;

				case 4: //发送\无回答用户数据
					break;

				case 9: //主站请求链路状态
					ResponseLinkStatus();
					break;
				}

				return SUCCESS;
			}
		}
	}

	return ERROR;
}

//判断属于哪条命令
/*
uint8_t Protocol(uint8_t* pBuffer) {
	uint8_t pCheckErrorResult;
	pCheckErrorResult = CheckError(pBuffer);
	if (pCheckErrorResult == STABLE_DATA) {
		if (CheckLinkAddress(pBuffer) == 1) {
			if (pBuffer[1] == 0x49) {
				ResponseLinkStatus();
			}
			else if (pBuffer[1] == 0x40) {
				ResponseResetRemoteLink();
			}
			//固定帧
			return STABLE_DATA;
		}
	}
	else if (pCheckErrorResult == VARIABLE_DATA) {
		if (CheckLinkAddress(pBuffer) == 1) {
			if ((pBuffer[7] == 0x64) && (pBuffer[14] == 0x14)) //总召命令
			{
				ResponseCallAll();
			}
			//可变帧长
			return VARIABLE_DATA;
		}
	}
	else {
		return INVALID_DATA;
	}
	return INVALID_DATA;
}
*/
