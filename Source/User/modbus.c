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

uint8_t CSD_485_YC[8];  //ң�����ݼĴ���
uint8_t CSD_485_YX[8];  //ң�����ݼĴ���
uint16_t HTRxTime;
/*�����жϳ�ʱ��û�н��յ�����ʱ�����֮ǰ���յ��Ĳ���������*/


volatile uint32_t CSD_485_COM_RxTimeoutCnt = 0;
volatile uint16_t CSD_485_COM_RxFlag = 0;
volatile uint16_t CSD_485_COM_RxLength = 0; 			//����������ݵĳ���
uint8_t  CSD_485_COM_RxBuf[CSD_485_RX_BUF];		//���ж��н���GPRS���͹���������

uint8_t  CSD_485_COM_TxBuf[CSD_485_TX_BUF];		//���ж��з�������

__IO uint16_t CSD_485_COM_TxCnt;
__IO uint16_t CSD_485_COM_TxTop;
__IO uint16_t HTComm_Addr;

static void Modbus_initData(void);
	
extern uint16_t  DebugDly;	//���ڴ��ڵ�����ʱ
extern uint8_t Info[16];//���������� 0/1/2 ���� 3����״̬ 4�¶�״̬ 5 Ƿѹ 6/7/8/9 ©��
extern uint8_t InfoTemp[8];//�¶�
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
	//���յ��������������
    if (CSD_485_COM_RxFlag == 1)//GPRSͨ�������·�����ʱ�ͻ��ô�λ
    {
        CSD_485_COM_RxFlag = 0;
		CSD_485_COM_RxTimeoutCnt = 0;		//ÿ���յ�һ�����ݣ�������ᱻ����
    }else{
		CSD_485_COM_RxTimeoutCnt++;
	}
    if (CSD_485_COM_RxTimeoutCnt == 3)	//�������ݺ�30ms	
    {
        CSD_485_COM_RxFlag = 0;
        CSD_485_COM_RxTimeoutCnt = 0;
        CSD_485_COM_RxLength = 0;
		CSD_485_ENABLE_RX();
    }else if(CSD_485_COM_RxTimeoutCnt == 3 * 60 * 100){//3����û����������
		CSD_485_InitRXbuf();
		USART2_Configuration();
	}
	//��ʱ����ղ������ݳ�ʼ������
	
	
}






/**********************************************************************

          Modbus �жϴ���
���ں� USART2					
***********************************************************************/

void CSD_485_UARTIsp(void)
{
	uint16_t da;
	uint16_t Tempdata;

	// we get data from GPRS (sim7600), and copy data to CSD_485_COM_RxBuf, then process the data
	memcpy(CSD_485_COM_RxBuf,DataFromGPRSBuffer, ReceiveDataFromGPRSflg);
	CSD_485_COM_RxLength = ReceiveDataFromGPRSflg;
	
  //CSD_485_COM_RxTimeoutCnt = 0;		//һֱ����ʱ�����ʱ
	//da = USART_ReceiveData(USART2);
	/*if((CSD_485_COM_RxLength == 0)  //�����ַ
	{
		CSD_485_COM_RxLength = 1;//�����жϵĳ�ʱ������ÿ����һ�����ݣ������Ҫ��λһ��
		CSD_485_COM_RxBuf[0] = da;
		
		HTRxTime=0;
		//����MODbus���ݿ��Դ�rs232���
		if(DebugDly > 0){					//���յ����ź�ͨ�����Դ��ڷ���
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
				//USART_SendData(USART2, CSD_485_COM_TxBuf[0]);//��������
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

MODBUS�������ź�����

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

���ģ�������ݵ���̨�Ĵ������ֽ�˳�򣬵��ֽ���ǰ�����ֽ��ں�
adr ����ַ
da  : ����

*******************************************************************/
void AddAnalogDa(uint8_t adr,uint16_t da)
{
 	if(adr > 49)
		adr=49;
	*(uint16_t *)(&(CSD_485_YC[adr*2])) = da;	
}

/******************************************************************

��λ�������Ĵ���

*******************************************************************/
void SetB(uint16_t adr)
{
	adr&=0x3ff;
	CSD_485_YX[adr/8]|=(1<<(adr%8));	 
}

/******************************************************************

��λ�������Ĵ���

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
	
	temp = (int8_t)InfoTemp[0];//������չ
	AddAnalogDa(1,temp);
	temp = (int8_t)InfoTemp[1];//������չ
	AddAnalogDa(2,temp);
	temp = (int8_t)InfoTemp[2];//������չ
	AddAnalogDa(3,temp);
}

void ChangeUpdate(uint16_t addr,uint16_t data){
	databuf[addr] = data;
}

void DataProcess(void){



}

void TempChangeUpdate(uint16_t addr,uint16_t data){//�¶�ֵͻ���ϴ�

}


void ModbusInitTxStatBuf(void)
{
	ModbusStatBuf0.Num = 0;
	ModbusStatBuf0.PtrIn = 0;
	ModbusStatBuf0.PtrOut = 0;
	ModbusStatBuf0.Buf[ModbusStatBuf0.PtrIn] = MODBUS_LED_IDLE;

};


void ModbusLoadStatBuf(uint16_t pDataBuf){

	if(ModbusStatBuf0.PtrIn > (MODBUS_STAT_BUF_NUM - 1))//ָ�볬����Χʱ����
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
	}//���ݷ���ʱ����
	//����3ʱֱ���˳�
}
uint16_t ModbusGetStatBuf(void){
	uint16_t pDataBuf;
	
	if(ModbusStatBuf0.Num > 0){
		ModbusStatBuf0.Num --;
	
		if(ModbusStatBuf0.PtrOut > (MODBUS_STAT_BUF_NUM - 1))//ָ�볬����Χʱ����
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
	//���ݷ���ʱ����
	//����3ʱֱ���˳�
}
