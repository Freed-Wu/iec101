#include "SIM7600.h"
#include "stm32f10x.h"
#include "string.h"

extern uint8_t FrameStartFlag;
extern uint8_t DataFromGPRSCnt;
extern uint8_t RxLength;

extern uint8_t DataFromGPRS[64];

void IWDG_Init(u8 prer, u16 rlr) {
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	IWDG_SetPrescaler(prer);

	IWDG_SetReload(rlr);

	IWDG_ReloadCounter();

	IWDG_Enable();
}

void IWDG_Feed(void) {
	IWDG_ReloadCounter();
}

//10ms÷–∂œ“ª¥Œ
void SysTick_Init(void) {
	if (SysTick_Config(SystemCoreClock / 50)) {
		/* Capture error */
		while (1)
			;
	}
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
extern volatile uint16_t SysTick_10msflg;

void SysTick_Handler(void) {
	SysTick_10msflg = 1;
}

void SYSCLKConfig_STOP(void) {
	ErrorStatus HSEStartUpStatus;

	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS) {
		RCC_PLLCmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
			;
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while (RCC_GetSYSCLKSource() != 0x08)
			;
	}
}

void RCC_Configuration(void) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
}
