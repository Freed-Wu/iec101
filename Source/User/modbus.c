#include "stm32f10x.h"
#include "string.h"
#include "mydef.h"
#include "conf_USART_RS485.h"
#include "Modbus.h"
#include "crc.h"
#include "sim7600.h"

extern uint16_t ReceiveDataFromGPRSflg;
extern uint8_t DataFromGPRSBuffer[GPRS_RCV_BUF];
extern GPRS_Tx GPRS_Tx0;

ModbusStatBuf ModbusStatBuf0;
//uint16_t DEVICE_ADDRESS;
uint8_t ModbusStat;
uint8_t databuf[20];
static uint16_t ModbusDigital(void);
static uint16_t ModbusAnalog(void);
static uint16_t ModbusDigitalSet(void);//telecontrol

uint8_t CSD_485_YC[8];  //遥测数据寄存器
uint8_t CSD_485_YX[8];  //遥信数据寄存器
uint16_t HTRxTime;
/*串口中断长时间没有接收到数据时，清除之前接收到的不完整数据*/


volatile uint32_t CSD_485_COM_RxTimeoutCnt = 0;
volatile uint16_t CSD_485_COM_RxFlag = 0;
volatile uint16_t CSD_485_COM_RxLength = 0; 			//定义接收数据的长度
uint8_t  CSD_485_COM_RxBuf[CSD_485_RX_BUF];		//在中断中接收GPRS发送过来的数据

uint8_t  CSD_485_COM_TxBuf[CSD_485_TX_BUF];		//在中断中发送数据

__IO uint16_t CSD_485_COM_TxCnt;
__IO uint16_t CSD_485_COM_TxTop;
__IO uint16_t HTComm_Addr;

static void Modbus_initData(void);
	
extern uint16_t  DebugDly;	//用于串口调试延时
extern uint8_t Info[16];//开关量数量 0/1/2 跌落 3跌落状态 4温度状态 5 欠压 6/7/8/9 漏保
extern uint8_t InfoTemp[8];//温度
/******************************************************************************
*
*
*
*
*
*
*******************************************************************************
*/
static void CSD_485_InitRXbuf(void){
	
    USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);
	CSD_485_COM_RxFlag = 0;
	CSD_485_COM_RxLength = 0;
	CSD_485_COM_RxTimeoutCnt = 0;
	USART_ClearFlag(CSD_485_COM,USART_IT_RXNE);
    USART_ITConfig(CSD_485_COM,USART_IT_RXNE,ENABLE);
}

void Supervise_CSD_485_COM(void)
{
	//接收到非完整数据清除
    if (CSD_485_COM_RxFlag == 1)//GPRS通过串口下发数据时就会置此位
    {
        CSD_485_COM_RxFlag = 0;
		CSD_485_COM_RxTimeoutCnt = 0;		//每接收到一个数据，这个都会被清零
    }else{
		CSD_485_COM_RxTimeoutCnt++;
	}
    if (CSD_485_COM_RxTimeoutCnt == 3)	//接收数据后30ms	
    {
        CSD_485_COM_RxFlag = 0;
        CSD_485_COM_RxTimeoutCnt = 0;
        CSD_485_COM_RxLength = 0;
		CSD_485_ENABLE_RX();
    }else if(CSD_485_COM_RxTimeoutCnt == 3 * 60 * 100){//3分钟没有数据则清
		CSD_485_InitRXbuf();
		USART2_Configuration();
	}
	//长时间接收不到数据初始化串口
	
	
}






/**********************************************************************

          Modbus 中断处理
串口号 USART2					
***********************************************************************/

void CSD_485_UARTIsp(void)
{
	uint16_t da;
	uint16_t Tempdata;

	// we get data from GPRS (sim7600), and copy data to CSD_485_COM_RxBuf, then process the data
	memcpy(CSD_485_COM_RxBuf,DataFromGPRSBuffer, ReceiveDataFromGPRSflg);
	CSD_485_COM_RxLength = ReceiveDataFromGPRSflg;
	
  //CSD_485_COM_RxTimeoutCnt = 0;		//一直接收时清除延时
	//da = USART_ReceiveData(USART2);
	/*if((CSD_485_COM_RxLength == 0)  //检验地址
	{
		CSD_485_COM_RxLength = 1;//启动中断的超时记数，每接收一组数据，这个都要置位一次
		CSD_485_COM_RxBuf[0] = da;
		
		HTRxTime=0;
		//调试MODbus数据可以从rs232输出
		if(DebugDly > 0){					//接收到的信号通过调试串口发出
			DEBUG_COM->DR =  CSD_485_COM_RxBuf[0];
		}
	}
	else
	{
		 CSD_485_COM_RxBuf[CSD_485_COM_RxLength++] = USART_ReceiveData(USART2);;
	}*/		
	//
	if(CSD_485_COM_RxLength >= 8)
	{
		Tempdata = CRC16(&CSD_485_COM_RxBuf[0],8);
		if(Tempdata == 0)											  
		{
			CSD_485_COM_TxTop = 0;								
			
			if(CSD_485_COM_RxBuf[1]==0x02)
				CSD_485_COM_TxTop = ModbusDigital();	
			else if(CSD_485_COM_RxBuf[1]==0x03)
				CSD_485_COM_TxTop = ModbusAnalog();
			else if(CSD_485_COM_RxBuf[1]==0x05)// origin code doesn't contain this line
				CSD_485_COM_TxTop = ModbusDigitalSet();
			if(CSD_485_COM_TxTop != 0)
			{
				CSD_485_COM_TxCnt=0;
				CSD_485_ENABLE_TX();
				//USART_SendData(USART2, CSD_485_COM_TxBuf[0]);//启动发送
				memcpy(GPRS_Tx0.TxBuf[GPRS_Tx0.TxPtrOut],CSD_485_COM_TxBuf,  CSD_485_COM_TxTop);// copy data which will be responsed to server to GPRS_Tx0.TxBuf in order to send
				SendDataToGPRSbuf((char *)CSD_485_COM_TxBuf,CSD_485_COM_TxTop);
			}
			
			CSD_485_COM_RxFlag=0;
			CSD_485_COM_RxLength=0;												
		}
	}		

}
/*
 *@description telecontrol
 */
uint16_t ModbusDigitalSet(void){
	uint16_t start,output,len=1;
	
	Modbus_initData();
	
	start = CSD_485_COM_RxBuf[2]*256+CSD_485_COM_RxBuf[3];
	output = CSD_485_COM_RxBuf[4]*256+CSD_485_COM_RxBuf[5];
	if (output==0xff00)
		output=1;
	else if (output==0x0000)
	  output=0;
	else
		return 0;// if output is not 0xff or 0x00, server sended an incorrect data, so we return 0
	
	// we need change the digital register in address of start according to output
	// after array info being changed, we can write the data to flash.
	Info[start]=output;
	
	// MCU will return a same data to server according to 3rd part of modbus protocol
	memcpy(CSD_485_COM_TxBuf,CSD_485_COM_RxBuf,  8);
		
	return 8;// the length of data
}

/**********************************************************************

MODBUS数字量信号生成

***********************************************************************/
static uint16_t ModbusDigital(void)
{
	uint16_t cnt;
	uint16_t bitoff;
	uint16_t top;
	uint16_t start,len;
	uint16_t temp;
	
	Modbus_initData();
	
	start = CSD_485_COM_RxBuf[2]*256+CSD_485_COM_RxBuf[3];
	len = CSD_485_COM_RxBuf[4]*256+CSD_485_COM_RxBuf[5];
	
	//
	if((start+len)>16)
		return (0);
	
	//
	CSD_485_COM_TxBuf[0]=CSD_485_COM_RxBuf[0];  //??
	CSD_485_COM_TxBuf[1]=0x02;					//???
		
	if(start%8==0)
		{		
			//
			start/=8;
			top=(uint16_t)(len/8);
			if((len%8)!=0)	
				top++;	
					
			//
			for(cnt=0;cnt<top;cnt++)	
				CSD_485_COM_TxBuf[cnt+3] = CSD_485_YX[start++];
		}
	else
		{
			//
			bitoff=start%8;			
			start/=8;
			top=(uint16_t)(len/8);				
			if((len%8)!=0)	
				top++;		
										
			//????
			for(cnt=0;cnt<top;cnt++)	
				{
					CSD_485_COM_TxBuf[cnt+3]=(CSD_485_YX[start]>>bitoff);
					start++;
					CSD_485_COM_TxBuf[cnt+3]|=(CSD_485_YX[start]<<(8-bitoff));
				}
		}
		
	if(cnt<3)	
		{
			CSD_485_COM_TxBuf[2] = cnt;		//??????
			temp=CRC16(&CSD_485_COM_TxBuf[0],cnt+3);	//CRC16??	
			CSD_485_COM_TxBuf[cnt+3]=temp >> 8;
			CSD_485_COM_TxBuf[cnt+4]=temp & 0xff;
			return (cnt + 5);
		}
	else
	return (0);

		
}

/**********************************************************************



***********************************************************************/
static uint16_t ModbusAnalog(void)
{
	uint16_t start,len;
	uint16_t cnt;
	uint16_t temp;
	////??????
	if(CSD_485_COM_RxBuf[4]!=0)
		return (0);
	Modbus_initData();
	////??????
	CSD_485_COM_TxBuf[0]=CSD_485_COM_RxBuf[0];		//??
	CSD_485_COM_TxBuf[1]=0x03;						//???
		
	if(CSD_485_COM_RxBuf[2]==0x01&&((CSD_485_COM_RxBuf[3]+CSD_485_COM_RxBuf[5])<=0x13))
		{
			start=(CSD_485_COM_RxBuf[3] << 1)&0xfe;	
			len=(CSD_485_COM_RxBuf[5] << 1)&0xfe;
			
			for(cnt=0;cnt<len;cnt+=2,start+=2)
			{
					CSD_485_COM_TxBuf[cnt+3]=CSD_485_YC[start+1];  //??
					CSD_485_COM_TxBuf[cnt+4]=CSD_485_YC[start];		
			}
		
			CSD_485_COM_TxBuf[2]=cnt;		//??????
			temp = CRC16(CSD_485_COM_TxBuf,cnt+3);	//CRC16??	
			CSD_485_COM_TxBuf[cnt+3]=temp>>8;
			CSD_485_COM_TxBuf[cnt+4]=temp&0xff;
		
			return (cnt+5);
		
		}
	
	
	
	return (0);	
}


/******************************************************************

添加模拟量数据到后台寄存器，字节顺序，低字节在前，高字节在后
adr ：地址
da  : 数据

*******************************************************************/
void AddAnalogDa(uint8_t adr,uint16_t da)
{
 	if(adr > 49)
		adr=49;
	*(uint16_t *)(&(CSD_485_YC[adr*2])) = da;	
}

/******************************************************************

置位数字量寄存器

*******************************************************************/
void SetB(uint16_t adr)
{
	adr&=0x3ff;
	CSD_485_YX[adr/8]|=(1<<(adr%8));	 
}

/******************************************************************

复位数字量寄存器

*******************************************************************/
void ClrB(uint16_t adr)
{
	adr&=0x3ff;
	CSD_485_YX[adr/8]&=~(1<<(adr%8));	 
}

static void Modbus_initData(void){
	int i;
	int16_t temp;
	
	CSD_485_YX[0] = 0;
	CSD_485_YX[1] = 0;
	for(i=0;i<8;i++){
		CSD_485_YC[0] = 0;
	}
	for(i=0;i<16;i++){
		if(Info[i] == 1){
			SetB(i);
		}
	}
	AddAnalogDa(0,(CSD_485_YX[1] << 8 ) | CSD_485_YX[0]);  
	
	temp = (int8_t)InfoTemp[0];//符号扩展
	AddAnalogDa(1,temp);
	temp = (int8_t)InfoTemp[1];//符号扩展
	AddAnalogDa(2,temp);
	temp = (int8_t)InfoTemp[2];//符号扩展
	AddAnalogDa(3,temp);
}

void ChangeUpdate(uint16_t addr,uint16_t data){
	databuf[addr] = data;
}

void DataProcess(void){



}

void TempChangeUpdate(uint16_t addr,uint16_t data){//温度值突发上传

}


void ModbusInitTxStatBuf(void)
{
	ModbusStatBuf0.Num = 0;
	ModbusStatBuf0.PtrIn = 0;
	ModbusStatBuf0.PtrOut = 0;
	ModbusStatBuf0.Buf[ModbusStatBuf0.PtrIn] = MODBUS_LED_IDLE;

};


void ModbusLoadStatBuf(uint16_t pDataBuf){

	if(ModbusStatBuf0.PtrIn > (MODBUS_STAT_BUF_NUM - 1))//指针超出范围时清零
	{
		ModbusInitTxStatBuf();
	}
	if(ModbusStatBuf0.Num < MODBUS_STAT_BUF_NUM){
		ModbusStatBuf0.Num ++;
	
		
		ModbusStatBuf0.Buf[ModbusStatBuf0.PtrIn] = pDataBuf;
			
		if(ModbusStatBuf0.PtrIn < (MODBUS_STAT_BUF_NUM - 1) ){
			ModbusStatBuf0.PtrIn++;
		}else{
			ModbusStatBuf0.PtrIn = 0;
		}
	}//数据放满时丢弃
	//等于3时直接退出
}
uint16_t ModbusGetStatBuf(void){
	uint16_t pDataBuf;
	
	if(ModbusStatBuf0.Num > 0){
		ModbusStatBuf0.Num --;
	
		if(ModbusStatBuf0.PtrOut > (MODBUS_STAT_BUF_NUM - 1))//指针超出范围时清零
		{
			ModbusInitTxStatBuf();
		}
		pDataBuf = ModbusStatBuf0.Buf[ModbusStatBuf0.PtrOut];
			
		if(ModbusStatBuf0.PtrOut < (MODBUS_STAT_BUF_NUM - 1) ){
			ModbusStatBuf0.PtrOut++;
		}else{
			ModbusStatBuf0.PtrOut = 0;
		}
	}else{
		if(ModbusStatBuf0.PtrOut != ModbusStatBuf0.PtrIn){
			ModbusInitTxStatBuf();
		}
		pDataBuf = MODBUS_LED_IDLE;
	}
	return pDataBuf;
	//数据放满时丢弃
	//等于3时直接退出
}
