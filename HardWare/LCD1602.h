#ifndef LCD1602_H
#define LCD1602_H

#include "stm32f10x_gpio.h"

/***********************************引脚定义********************************************/
#define BUSY 0x80		//忙标志
#define RS GPIO_Pin_5	//a5 设置PB5为RS
#define RW GPIO_Pin_6	//a6 PB6为RW
#define EN GPIO_Pin_4	//a7 PB7为EN使能
    //  0   1   2   3    4   5   6   7
    //  a7  a1  b0  c15  b1  c14 b11  b10
#define SET_D0() GPIO_SetBits(GPIOA, GPIO_Pin_7)
#define CLR_D0() GPIO_ResetBits(GPIOA, GPIO_Pin_7)
#define SET_D1() GPIO_SetBits(GPIOA, GPIO_Pin_1)
#define CLR_D1() GPIO_ResetBits(GPIOA, GPIO_Pin_1)
#define SET_D2() GPIO_SetBits(GPIOB, GPIO_Pin_0)
#define CLR_D2() GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define SET_D3() GPIO_SetBits(GPIOC, GPIO_Pin_15)
#define CLR_D3() GPIO_ResetBits(GPIOC, GPIO_Pin_15)
#define SET_D4() GPIO_SetBits(GPIOB, GPIO_Pin_1)
#define CLR_D4() GPIO_ResetBits(GPIOB, GPIO_Pin_1)
#define SET_D5() GPIO_SetBits(GPIOC, GPIO_Pin_14)
#define CLR_D5() GPIO_ResetBits(GPIOC, GPIO_Pin_14)
#define SET_D6() GPIO_SetBits(GPIOB, GPIO_Pin_11)
#define CLR_D6() GPIO_ResetBits(GPIOB, GPIO_Pin_11)
#define SET_D7() GPIO_SetBits(GPIOB, GPIO_Pin_10)
#define CLR_D7() GPIO_ResetBits(GPIOB, GPIO_Pin_10)
/***********************************引脚定义********************************************/

/***********************************函数定义********************************************/
void ReadBusy(void);
void LCD_WRITE_CMD( unsigned char CMD );
void LCD_WRITE_StrDATA( unsigned char *StrData, unsigned char row, unsigned char col );
void LCD_WRITE_ByteDATA( unsigned char ByteData );
void LCD_INIT(void);
void GPIO_INIT(void);
void WUserImg(unsigned char pos,unsigned char *ImgInfo);
__INLINE void LCD_DATA_WRITE(unsigned char data);
/***********************************函数定义********************************************/
#endif
