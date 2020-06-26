#ifndef _fun_h_
#define _fun_h_
#include "stm32f10x.h"

void Delay10ms(vu32 p10msCount);
void Delay(vu32 nCount);

uint32_t Str2Int(char* str);
void Int2Str(char* str, uint32_t Data);
void hex2Str(char* CharData, unsigned char* HexData, unsigned char pCharLength);
#endif
