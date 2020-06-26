#ifndef _ds3231_h
#define _ds3231_h

unsigned char ReadDATATime(void);
//unsigned char TWI_ReadByte( unsigned char ReadAddress);
//unsigned char TWI_WriteByte(unsigned char SendByte, unsigned char WriteAddress);

unsigned char WriteDATATime(void);
unsigned char ReadDATATime(void);

#endif
