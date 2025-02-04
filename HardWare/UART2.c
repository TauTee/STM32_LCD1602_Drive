#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "UART2.h"

unsigned int TXBUFFERMAX;
unsigned int RXBUFFERMAX;

unsigned char *TxBuffer;			//[TXBUFFERMAX];
static unsigned int TxCounter = 0;	//用于发送的底层计数
unsigned char *RxBuffer;			//[RXBUFFERMAX];
static unsigned int RxCounter = 0;

static unsigned int put_count = 0; 	//用记录总共需要发送多少数据
static unsigned int get_index = 0;

void uart2_tx_now(unsigned int length)
{
	put_count = length;
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void uart2_tx_stop(void)
{
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);// 全部发送完成
}

void set_buffer(unsigned char *txbuf, unsigned int txbufmax, unsigned char *rxbuf, unsigned int rxbufmax)
{
    TxBuffer = txbuf;
    TXBUFFERMAX = txbufmax;
    RxBuffer = rxbuf;
    RXBUFFERMAX = rxbufmax;
}

int Initial_UART2()
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	  
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure); 
    
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);  
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);        
	USART_ClearFlag(USART2,USART_FLAG_TC);
    
	USART_Cmd(USART2, ENABLE);
    
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    return 0;
}

void UART2_Put_Char(unsigned char DataToSend)
{
	TxBuffer[put_count++] = DataToSend;  
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);  
}

void UART2_Put_String(unsigned char *Str)
{
	while(*Str)
	{
		if(*Str=='\r')UART2_Put_Char(0x0d);
			else if(*Str=='\n')UART2_Put_Char(0x0a);
				else UART2_Put_Char(*Str);
		Str++;
	}
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {   
        USART_SendData(USART2, TxBuffer[TxCounter++]); 
        if(TxCounter == put_count) 
        {
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);// 全部发送完成
        }
        USART_ClearITPendingBit(USART2, USART_IT_TXE); 
    }
	else if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
		if(RxCounter < TXBUFFERMAX-2)
        {
            RxBuffer[++RxCounter] = (unsigned char)USART2->DR;
        }else
        {
            
        }
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
	USART_ClearITPendingBit(USART2,USART_IT_ORE);

}

