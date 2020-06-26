#include "stm32f10x.h"

#define USART_DMABUF_NUM 64
#define USART_DATBUF_NUM 64

vu8 USART_DMA_TXBuf[USART_DMABUF_NUM];
vu8 USART_INT_RXBuf[USART_DMABUF_NUM];

//==================================================================================
//						USART3_TX DMA配置程序
//
//==================================================================================
void DMA_USART3_TX_Configuration(void) {
	DMA_InitTypeDef DMA_InitStructure;
	/*DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel2); //复位指定通道的寄存器，清除该通道的中断标志
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR; //外设地址为ADC_DR寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&USART_DMA_TXBuf; //存放DMA数据的内存单元地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //从内存单元到外设读数据
	//这个数据应该和存储器设置的存储空间大小相同
	DMA_InitStructure.DMA_BufferSize = USART_DMABUF_NUM; //数据缓冲区大小为1
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //不执行外设地址增量方式

	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //执行存储器地址增量方式

	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据传输为16bit方式
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //存储器数据宽度为16bit
	//注意，每次读取值后应设置数据个数
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //通道优先级为高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //寄存器之间传递数据禁止
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);

	/* Enable DMA1 channel2 */
	DMA_Cmd(DMA1_Channel2, DISABLE); //dma通道开
}
