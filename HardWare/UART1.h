#ifndef __UART1_H
#define __UART1_H

typedef enum
{
    EMPTY = 0,
    FULL  = !EMPTY
}BUFF_STATUS;

int Initial_UART1(void);
void UART1_Put_Char(unsigned char DataToSend);
unsigned int UART1_Get_Data(void *idata, unsigned int length);
void UART1_Put_String(unsigned char *Str);
#endif

//------------------End of File----------------------------

