/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
 
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>
#include "stm32f10x_usart.h"
#include "cycle_buffer_io.h"

#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(rt_uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(rt_uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(rt_uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(rt_uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(rt_uint8_t  *)(0xE000ED23UL))

// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);

// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFF)
    {
        return 1;
    }
    
    _SYSTICK_LOAD = ticks - 1; 
    _SYSTICK_PRI = 0xFF;
    _SYSTICK_VAL  = 0;
    _SYSTICK_CTRL = 0x07;  
    
    return 0;
}

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 1024*2
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 4K(1024 * 4)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

#define FINSH_RX_BUFF_SIZE 32
static cycleBuffer finsh_rx_cycle_buff;
static uInt8 finsh_rx_buff[FINSH_RX_BUFF_SIZE] = {0};
static struct rt_semaphore shell_rx_sem;

static int uart_init(void)
{
    USART_InitTypeDef usart1;
    GPIO_InitTypeDef  pa;
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    init_cycle_buffer(&finsh_rx_cycle_buff, finsh_rx_buff, FINSH_RX_BUFF_SIZE);
    rt_sem_init(&(shell_rx_sem), "sherx", 0, 0);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    pa.GPIO_Pin   = GPIO_Pin_9;
	pa.GPIO_Mode  = GPIO_Mode_AF_PP;
	pa.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &pa);
    
    pa.GPIO_Pin = GPIO_Pin_10;
    pa.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &pa);
    
    usart1.USART_BaudRate            = 115200;
    usart1.USART_WordLength          = USART_WordLength_8b;
    usart1.USART_StopBits            = USART_StopBits_1;
    usart1.USART_Parity              = USART_Parity_No;
    usart1.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usart1);
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ClearFlag(USART1, USART_FLAG_TC);
    
    USART_Cmd(USART1, ENABLE);
    
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    return 0;
}
INIT_BOARD_EXPORT(uart_init);

void USART1_IRQHandler(void)
{
    int ch = -1;
    rt_interrupt_enter();
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET && USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
    {   
        while(1)
        {
            ch = -1;
            if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
            {
                //ch = USART_ReceiveData(USART1);
                ch =(USART1->DR & 0xFF);
            }
            if(ch == -1)break;
            finsh_rx_cycle_buff.write(&finsh_rx_cycle_buff, &ch, 1);
        }
        rt_sem_release(&shell_rx_sem);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE); 
    }
    rt_interrupt_leave();
}

void rt_hw_console_output(const char *str)
{
    rt_size_t i = 0, size = 0;
    char a ='\r';
    
    size = rt_strlen(str);
    for(i = 0; i < size; i++)
    {
        if(*(str + i) == '\n')
        {
            USART_SendData(USART1, (uint16_t)a);
        }
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
        USART_SendData(USART1, (uint16_t)(*(str + i)));
    }
}

char rt_hw_console_getchar(void)
{
    int ch = -1;
    
    while(finsh_rx_cycle_buff.read(&finsh_rx_cycle_buff, &ch, 1) != 1)
    {
        rt_sem_take(&shell_rx_sem, RT_WAITING_FOREVER);
    }
    return ch;
}

/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
    /* System Clock Update */
    SystemCoreClockUpdate();
    
    /* System Tick Configuration */
    _SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}
