#ifndef _conf_NVIC_h
#define _conf_NVIC_h

#include "mydef.h"
#include "stm32f10x.h"

/**/
void NVIC_Configuration(void);
/*���ⲿ�ж�*/
void EnableExtINT(void);
//�ر��ⲿ�ж�
void DisableExtINT(void);
/*NRF905 �ж��źŲɼ���ӳ��ܽ�*/
void EXTI_Configuration(void);

#endif
