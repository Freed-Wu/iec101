#ifndef _nrf905_h
#define _nrf905_h

#ifdef _nrf905_c
#define _EXTERN
#else
#define _EXTERN extern
#endif

void nrf905_init(void);
void SpiWrite(unsigned char b);
unsigned char SpiRead(void);
void SPI_RdConfiRegs(unsigned char cmd, int length);
void TxPacket(void);
void RxPacket(void);
void Wait_Rec_Packet(void);
void PowerUp_NRF905(void);
void ProcWireLessData(void);

unsigned int checknrf905_conf(void);
unsigned int checknrf905_addr(void);

#endif
