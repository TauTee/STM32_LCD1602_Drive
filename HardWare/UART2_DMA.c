#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "UART2_DMA.h"
#include "cycle_buffer_io.h"
#include <rtthread.h>

uInt32 TXBUFFERMAX;
uInt32 RXBUFFERMAX;

uInt8 *TxBuffer;			//[TXBUFFERMAX];
uInt8 *RxBuffer;			//[RXBUFFERMAX];

void uart2_tx_now(uInt32 length)
{
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC7);
    DMA_SetCurrDataCounter(DMA1_Channel7, length);        
    DMA_Cmd(DMA1_Channel7, ENABLE);
}

void uart2_tx_all_now(void)
{
    uart2_tx_now(TXBUFFERMAX);
}

void uart2_tx_stop(void)
{
	DMA_Cmd(DMA1_Channel7, DISABLE);
}

void set_buffer(uInt8 *txbuf, uInt32 txbufmax, uInt8 *rxbuf, uInt32 rxbufmax)
{
    TxBuffer = txbuf;
    TXBUFFERMAX = txbufmax;
    RxBuffer = rxbuf;
    RXBUFFERMAX = rxbufmax;
}

void Initial_DMA()
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    /* USART2_Tx_DMA_Channel (triggered by USARTy Tx event) Config */
    DMA_DeInit(DMA1_Channel7);//usart2 rx
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(TxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = TXBUFFERMAX;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);

    /* USART2_Rx_DMA_Channel (triggered by USARTz Rx event) Config */
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(RxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = RXBUFFERMAX;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_Cmd(DMA1_Channel6, ENABLE);
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
}

int Initial_UART2()
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
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
    
    USART_DMACmd(USART2, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
    
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	USART_ClearFlag(USART2, USART_FLAG_TC);
    
	USART_Cmd(USART2, ENABLE);
    
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    return 0;
}

void DMA1_Channel7_IRQHandler()
{
    rt_interrupt_enter();
    if(DMA_GetITStatus(DMA1_IT_TC7) == SET)
    {
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);
        transmit_finish_handler();
        
        DMA_ClearITPendingBit(DMA1_IT_TC7);
    }
    rt_interrupt_leave();
}


unsigned int in_i = 0;
void USART2_IRQHandler(void)
{
    rt_interrupt_enter();
    u8 tem;
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {   
        tem = USART2->SR;   //先读SR后读DR可以清除DR内容
        tem = USART2->DR;
        tem = tem + 1;        //防止编译器警告
        
        receive_finish_handler(RXBUFFERMAX - DMA_GetCurrDataCounter(DMA1_Channel6));
        
        USART_ClearITPendingBit(USART2, USART_IT_IDLE); 
    }
    rt_interrupt_leave();
}

