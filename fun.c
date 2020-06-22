#include "stm32f10x.h"

extern volatile uint16_t SysTick_10msflg; //10ms溢出标志，主程序10ms运行一次

/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
* Input          : nCount: specifies the delay time length.
* Output         : None
* Return         : None
*******************************************************************************/
void Delay10ms(vu32 p10msCount) {
	while (p10msCount--) {
		while (SysTick_10msflg == 0)
			;
		SysTick_10msflg = 0;
	}
}

void Delay(vu32 nCount) {
	for (; nCount != 0; nCount--)
		;
}

/*
****************************************************************************************************
* 功能描述：
* 输入参数：
* 返回参数：
* 说    明：
****************************************************************************************************
*/
uint32_t Str2Int(char* str) {
	int i = 0;
	uint32_t temp = 0;

	while (str[i] != '\0') {
		if ((str[i] >= '0') && (str[i] <= '9')) {
			temp = temp * 10 + (str[i] - '0');
		}
		i++;
	}
	return temp;
}

/*
****************************************************************************************************
* 功能描述：
* 输入参数：
* 返回参数：
* 说    明：
****************************************************************************************************
*/
void Int2Str(char* str, uint32_t Data) {
	uint8_t len = 0;
	uint8_t i = 0;
	char str_tab[32] = {0};

	while (Data / 10) {
		str_tab[len++] = Data % 10 + '0';
		Data = Data / 10;
	}
	if (Data < 10) {
		str_tab[len] = Data + '0';
	}
	for (i = 0; i <= len; i++) {
		*str = str_tab[len - i];
		str++;
	}
}

void hex2Str(char* CharData, unsigned char* HexData, unsigned char pCharLength) {
	uint16_t i;
	uint8_t byTemp;

	for (i = 0; i < pCharLength; i++) {
		byTemp = HexData[i];
		byTemp &= 0x0f;
		if (byTemp >= 10) // show 'A' - 'F'
		{
			CharData[2 * i + 1] = byTemp - 0xa + 0x41;
		}
		else // show '0' - '9'
		{
			CharData[2 * i + 1] = byTemp + '0';
		}
		byTemp = HexData[i];
		byTemp >>= 0x4;
		byTemp &= 0xf;
		if (byTemp >= 10) // show 'A' - 'F'
		{
			CharData[2 * i] = byTemp - 0xa + 0x41;
		}
		else // show '0' - '9'
		{
			CharData[2 * i] = byTemp + '0';
		}
	}
}
