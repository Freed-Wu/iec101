#ifndef _conf_sys_h
#define _conf_sys_h
#include "stm32f10x.h"

void IWDG_Init(u8 prer, u16 rlr);

void IWDG_Feed(void);
void SysTick_Init(void);

void SysTick_Handler(void);
void SYSCLKConfig_STOP(void);

#endif
