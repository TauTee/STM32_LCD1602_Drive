#include "stm32f10x_it.h"
#include "LCD1602.h"
#include <rtthread.h>
#include <stdio.h>
#include "cycle_buffer_io.h"
#include "esp8266_at.h"


unsigned char tem[] = {'T','K','Q','T','E','S','T','I','N','G'};	//摄氏度符号“℃”的字模

ALIGN(RT_ALIGN_SIZE)
static char thread_hello_stack[512];
static struct rt_thread thread_hello;

static void thread_hello_entry(void *param)
{
    while(1)
    {
        //rt_kprintf("Hello,world!\n");
        rt_thread_mdelay(2000);
    }
}

static rt_thread_t thread_lcd_show = RT_NULL;

static void thread_lcd_entry(void *param)
{
    //	WUserImg( 0, tem );	//写入用户自定义字符
    //	LCD_WRITE_CMD( 0x80 );				//指定屏幕第一行第一列输出
    //	LCD_WRITE_ByteDATA( 0x00 );			//输出第一个用户自定义字符
    unsigned char time_str[] = "000000";
    while(1)
    {
        LCD_WRITE_StrDATA("Hello,", 0, 0);		//向屏幕第一行第五列输出HELLO
        sprintf(time_str, "%d!", (unsigned int)rt_tick_get());
        LCD_WRITE_StrDATA(time_str, 1, 2);		//向屏幕第一行第五列输出HELLO
        rt_thread_mdelay(5000);
    }
}

static rt_thread_t thread_led = RT_NULL;

static void thread_led_entry(void *param)
{
    while(1)
    {
        if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) != RESET)
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        }else
        {
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
        }
        rt_thread_mdelay(500);
    }
}

void led_init(void)
{
    GPIO_InitTypeDef PC;
    
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );
    
    PC.GPIO_Pin    = GPIO_Pin_13;
	PC.GPIO_Mode   = GPIO_Mode_Out_PP;
	PC.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &PC);
    
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

int main()
{	
    rt_err_t rt_e;
    
    LCD_INIT();		//LCD1602初始化
    led_init();
    init_esp_hardware();
    
    thread_lcd_show = rt_thread_create("lcd", thread_lcd_entry, RT_NULL, 512, 3, 5);
    if(thread_lcd_show != RT_NULL)
    {
        rt_thread_startup(thread_lcd_show);
    }
    
    thread_led = rt_thread_create("led", thread_led_entry, RT_NULL, 512, 4, 5);
    if(thread_led != RT_NULL)
    {
        rt_thread_startup(thread_led);
    }
    
    rt_e = rt_thread_init(&thread_hello, "hi", thread_hello_entry, RT_NULL,
                    thread_hello_stack, sizeof(thread_hello_stack), 5, 5); 
    
    if(rt_e == RT_EOK)
    {
        rt_thread_startup(&thread_hello);
    }
}
