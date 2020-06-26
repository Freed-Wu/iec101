#include "stm32f10x.h"

#define USART_DMABUF_NUM 64
#define USART_DATBUF_NUM 64

vu8 USART_DMA_TXBuf[USART_DMABUF_NUM];
vu8 USART_INT_RXBuf[USART_DMABUF_NUM];

//==================================================================================
//						USART3_TX DMA���ó���
//
//==================================================================================
void DMA_USART3_TX_Configuration(void) {
	DMA_InitTypeDef DMA_InitStructure;
	/*DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel2); //��λָ��ͨ���ļĴ����������ͨ�����жϱ�־
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR; //�����ַΪADC_DR�Ĵ�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&USART_DMA_TXBuf; //���DMA���ݵ��ڴ浥Ԫ��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //���ڴ浥Ԫ�����������
	//�������Ӧ�úʹ洢�����õĴ洢�ռ��С��ͬ
	DMA_InitStructure.DMA_BufferSize = USART_DMABUF_NUM; //���ݻ�������СΪ1
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //��ִ�������ַ������ʽ

	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //ִ�д洢����ַ������ʽ

	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //���ݴ���Ϊ16bit��ʽ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //�洢�����ݿ��Ϊ16bit
	//ע�⣬ÿ�ζ�ȡֵ��Ӧ�������ݸ���
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //ͨ�����ȼ�Ϊ��
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //�Ĵ���֮�䴫�����ݽ�ֹ
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);

	/* Enable DMA1 channel2 */
	DMA_Cmd(DMA1_Channel2, DISABLE); //dmaͨ����
}
