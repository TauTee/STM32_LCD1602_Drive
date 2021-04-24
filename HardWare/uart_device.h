#ifndef __UART_DEVICE_H
#define __UART_DEVICE_H

typedef struct
{
    unsigned long baudrate;
    
}uartDevice;

int Initial_UART(uartDevice idevice);
void UART1_Put_Char(unsigned char DataToSend);
unsigned char UART1_Get_Char(void);
void UART1_Put_String(unsigned char *Str);

#endif