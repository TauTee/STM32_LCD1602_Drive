#ifndef __UART2_H
#define __UART2_H

void Initial_DMA(void);
int Initial_UART2(void);
void UART2_Put_Char(unsigned char DataToSend);
unsigned char UART2_Get_Data(void *idata, unsigned int length);
void UART2_Put_String(unsigned char *Str);
void set_buffer(unsigned char *txbuf, unsigned int txbufmax, unsigned char *rxbuf, unsigned int rxbufmax);

void uart2_tx_now(unsigned int length);
void uart2_tx_stop(void);

#endif

//------------------End of File----------------------------

