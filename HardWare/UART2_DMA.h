#ifndef __UART2_DMA_H
#define __UART2_DMA_H

#include "my_type.h"

int Initial_UART2(void);
void set_buffer(uInt8 *txbuf, uInt32 txbufmax, uInt8 *rxbuf, uInt32 rxbufmax);
void uart2_tx_now(uInt32 length);
void uart2_tx_stop(void);

#endif

