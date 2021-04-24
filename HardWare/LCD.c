#include "LCD1602.h"
#include "stm32f10x_rcc.h"
#include <rtthread.h>

//void delay_us(unsigned int us){
//	unsigned int  i;
//	
//	do{
//		i = 10;
//		while(i--) __nop();
//	} while (--us);

//}
/***********************************GPIO初始化********************************************/
void GPIO_INIT(void){		//GPIO初始化
	GPIO_InitTypeDef PB;
	GPIO_InitTypeDef PA;
    GPIO_InitTypeDef PC;
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);	//禁用jtag，不然写入程序和程序执行都会受影响
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );		//打开GPIOA~C
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );
	
	PA.GPIO_Pin   = EN|RW|RS;
	PA.GPIO_Mode  = GPIO_Mode_Out_PP;
	PA.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &PA);
	
    PA.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_1;
	PA.GPIO_Mode = GPIO_Mode_Out_PP;
	PA.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &PA);
    
    PB.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_11|GPIO_Pin_0|GPIO_Pin_10;
	PB.GPIO_Mode = GPIO_Mode_Out_PP;
	PB.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &PB);
    
    PC.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;
	PC.GPIO_Mode = GPIO_Mode_Out_PP;
	PC.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &PC);
	
}
/***********************************GPIO初始化********************************************/


/***********************************LCD初始化********************************************/
void LCD_INIT(void){	//初始化
	GPIO_INIT();	//GPIO的初始化，在LCD_INIT被调用时自动调用
    rt_thread_mdelay(500);
	LCD_WRITE_CMD( 0x38 );
	LCD_WRITE_CMD( 0x0c );	//开启光标和闪烁
	LCD_WRITE_CMD( 0x06 );
	LCD_WRITE_CMD( 0x01 );
    LCD_WRITE_CMD( 0x80 );
}
/***********************************LCD初始化********************************************/


void LCD_DATA_WRITE(unsigned char data)
{
    //  0   1   2   3    4   5   6   7
    //  a7  a1  b0  c15  a4  c14 b7  b8
    if(data & 0x01)
    {
        SET_D0();
    }else
    {
        CLR_D0();
    }
    if(data & 0x02)
    {
        SET_D1();
    }else
    {
        CLR_D1();
    }
    if(data & 0x04)
    {
        SET_D2();
    }else
    {
        CLR_D2();
    }
    if(data & 0x08)
    {
        SET_D3();
    }else
    {
        CLR_D3();
    }
    if(data & 0x10)
    {
        SET_D4();
    }else
    {
        CLR_D4();
    }
    if(data & 0x20)
    {
        SET_D5();
    }else
    {
        CLR_D5();
    }
    if(data & 0x40)
    {
        SET_D6();
    }else
    {
        CLR_D6();
    }
    if(data & 0x80)
    {
        SET_D7();
    }else
    {
        CLR_D7();
    }
}

/***********************************写入命令函数********************************************/
void LCD_WRITE_CMD( unsigned char CMD ){		//写入命令函数
//	ReadBusy();
	GPIO_ResetBits( GPIOA, RS );
	GPIO_ResetBits( GPIOA, RW );
	GPIO_ResetBits( GPIOA, EN );
    
    LCD_DATA_WRITE(CMD);
    rt_thread_mdelay(1);
	GPIO_SetBits( GPIOA, EN );
    rt_thread_mdelay(5);
	GPIO_ResetBits( GPIOA, EN );
}
/***********************************写入命令函数********************************************/

/***********************************写入单个Byte函数********************************************/
void LCD_WRITE_ByteDATA( unsigned char ByteData ){	//写入单个Byte函数
//	ReadBusy();
	GPIO_SetBits( GPIOA, RS );
	GPIO_ResetBits( GPIOA, RW );
	GPIO_ResetBits( GPIOA, EN );
    
	//GPIO_Write( GPIOA, ByteData );//需重新实现
    LCD_DATA_WRITE(ByteData);
    rt_thread_mdelay(1);
	GPIO_SetBits( GPIOA, EN );
    rt_thread_mdelay(5);
	GPIO_ResetBits( GPIOA, EN );
}

/***********************************写入单个Byte函数********************************************/

/***********************************写入字符串函数********************************************/

void LCD_WRITE_StrDATA( unsigned char *StrData, unsigned char row, unsigned char col ){//写入字符串
	unsigned char baseAddr = 0x00;			//定义256位地址
	if ( row ){
		baseAddr = 0xc0;
	}else{
		baseAddr = 0x80;																				   
	} 	//row为1用户选择第二行
		//row为0用户选择第一行
	baseAddr += col;

	while ( *StrData != '\0' ){

		LCD_WRITE_CMD( baseAddr );
		LCD_WRITE_ByteDATA( *StrData );
	
		baseAddr++;			   //每次循环地址加一，数据指针加一
		StrData++;
	}
}
/***********************************写入字符串函数********************************************/

/***********************************读忙函数********************************************/
void ReadBusy(void){		//读忙函数，读忙之前记得更改引脚的工作方式！！！因为STM32的IO不是准双向IO
	//GPIO_Write( GPIOA, 0x00ff );//改动以下	
	LCD_DATA_WRITE(0xff);
    
    //B8<--A7
	GPIO_InitTypeDef p;
    GPIO_InitTypeDef PB;
	GPIO_InitTypeDef PA;
    GPIO_InitTypeDef PC;
    
	p.GPIO_Pin = GPIO_Pin_10;		//选定GPIOA的第七Pin
	p.GPIO_Mode = GPIO_Mode_IPD;	//第七Pin的工作方式为浮空输入模式，用于检测LCD1602的忙状态
	p.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOB, &p );
	
	GPIO_ResetBits( GPIOA, RS );//RS拉低
	GPIO_SetBits( GPIOA, RW );//RW拉高
	
    GPIO_SetBits( GPIOA, EN );//使能关
	while( GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_10 ) );	//读第七Pin状态，如果一直为1则循环等待
    GPIO_ResetBits( GPIOA, EN );	//使能开
	/*do{
		GPIO_ResetBits( GPIOB, EN );
		GPIO_SetBits( GPIOB, EN );
		Flag = GPIO_ReadInputDataBit( GPIOA, GPIO_Pin_7 ) & BUSY;
		GPIO_ResetBits( GPIOB, EN );
	}while( Flag );*/
	
    PA.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_1;
	PA.GPIO_Mode = GPIO_Mode_Out_PP;
	PA.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &PA);
    
    PB.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_11|GPIO_Pin_0|GPIO_Pin_10;
	PB.GPIO_Mode = GPIO_Mode_Out_PP;
	PB.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &PB);
    
    PC.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;
	PC.GPIO_Mode = GPIO_Mode_Out_PP;
	PC.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &PC);
    
//	p.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|
//				  GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|
//				  GPIO_Pin_6|GPIO_Pin_7;		//使GPIOA的状态还原成推挽模式
//	p.GPIO_Mode = GPIO_Mode_Out_PP;
//	p.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init( GPIOA, &p  );
}
/***********************************读忙函数********************************************/

/***********************************写入用户自定义图像********************************************/
void WUserImg(unsigned char pos,unsigned char *ImgInfo){ //写入用户自定义图像
	unsigned char cgramAddr;			//CGRAM的用户自定义字符位
	
	if( pos <= 1 ) cgramAddr = 0x40;		// 
	if( pos > 1 && pos <= 3 ) cgramAddr = 0x50;
	if( pos > 3 && pos <= 5 ) cgramAddr = 0x60;
	if( pos > 5 && pos <= 7 ) cgramAddr = 0x70;

	LCD_WRITE_CMD( (cgramAddr + (pos%2) * 8) );	//指定字模写入的地址，一般从0x40开始，0x78结束
	
	while( *ImgInfo != '\0' ){		//循环写入tem数据,即用户取模的数据
		LCD_WRITE_ByteDATA( *ImgInfo );
		ImgInfo++;
	}
}
/***********************************写入用户自定义图像********************************************/
