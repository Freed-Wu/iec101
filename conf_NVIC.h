#ifndef _conf_NVIC_h
#define _conf_NVIC_h

#include "mydef.h"
#include "stm32f10x.h"

/**/
void NVIC_Configuration(void);
/*打开外部中断*/
void EnableExtINT(void);
//关闭外部中断
void DisableExtINT(void);
/*NRF905 中断信号采集，映射管脚*/
void EXTI_Configuration(void);

#endif
