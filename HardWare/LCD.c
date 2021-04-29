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
/***********************************GPIO��ʼ��********************************************/
void GPIO_INIT(void){		//GPIO��ʼ��
	GPIO_InitTypeDef PB;
	GPIO_InitTypeDef PA;
    GPIO_InitTypeDef PC;
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);	//����jtag����Ȼд�����ͳ���ִ�ж�����Ӱ��
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );		//��GPIOA~C
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
/***********************************GPIO��ʼ��********************************************/


/***********************************LCD��ʼ��********************************************/
void LCD_INIT(void){	//��ʼ��
	GPIO_INIT();	//GPIO�ĳ�ʼ������LCD_INIT������ʱ�Զ�����
    rt_thread_mdelay(500);
	LCD_WRITE_CMD( 0x38 );
	LCD_WRITE_CMD( 0x0c );	//����������˸
	LCD_WRITE_CMD( 0x06 );
	LCD_WRITE_CMD( 0x01 );
    LCD_WRITE_CMD( 0x80 );
}
/***********************************LCD��ʼ��********************************************/


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

/***********************************д�������********************************************/
void LCD_WRITE_CMD( unsigned char CMD ){		//д�������
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
/***********************************д�������********************************************/

/***********************************д�뵥��Byte����********************************************/
void LCD_WRITE_ByteDATA( unsigned char ByteData ){	//д�뵥��Byte����
//	ReadBusy();
	GPIO_SetBits( GPIOA, RS );
	GPIO_ResetBits( GPIOA, RW );
	GPIO_ResetBits( GPIOA, EN );
    
	//GPIO_Write( GPIOA, ByteData );//������ʵ��
    LCD_DATA_WRITE(ByteData);
    rt_thread_mdelay(1);
	GPIO_SetBits( GPIOA, EN );
    rt_thread_mdelay(5);
	GPIO_ResetBits( GPIOA, EN );
}

/***********************************д�뵥��Byte����********************************************/

/***********************************д���ַ�������********************************************/

void LCD_WRITE_StrDATA( unsigned char *StrData, unsigned char row, unsigned char col ){//д���ַ���
	unsigned char baseAddr = 0x00;			//����256λ��ַ
	if ( row ){
		baseAddr = 0xc0;
	}else{
		baseAddr = 0x80;																				   
	} 	//rowΪ1�û�ѡ��ڶ���
		//rowΪ0�û�ѡ���һ��
	baseAddr += col;

	while ( *StrData != '\0' ){

		LCD_WRITE_CMD( baseAddr );
		LCD_WRITE_ByteDATA( *StrData );
	
		baseAddr++;			   //ÿ��ѭ����ַ��һ������ָ���һ
		StrData++;
	}
}
/***********************************д���ַ�������********************************************/

/***********************************��æ����********************************************/
void ReadBusy(void){		//��æ��������æ֮ǰ�ǵø������ŵĹ�����ʽ��������ΪSTM32��IO����׼˫��IO
	//GPIO_Write( GPIOA, 0x00ff );//�Ķ�����	
	LCD_DATA_WRITE(0xff);
    
    //B8<--A7
	GPIO_InitTypeDef p;
    GPIO_InitTypeDef PB;
	GPIO_InitTypeDef PA;
    GPIO_InitTypeDef PC;
    
	p.GPIO_Pin = GPIO_Pin_10;		//ѡ��GPIOA�ĵ���Pin
	p.GPIO_Mode = GPIO_Mode_IPD;	//����Pin�Ĺ�����ʽΪ��������ģʽ�����ڼ��LCD1602��æ״̬
	p.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOB, &p );
	
	GPIO_ResetBits( GPIOA, RS );//RS����
	GPIO_SetBits( GPIOA, RW );//RW����
	
    GPIO_SetBits( GPIOA, EN );//ʹ�ܹ�
	while( GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_10 ) );	//������Pin״̬�����һֱΪ1��ѭ���ȴ�
    GPIO_ResetBits( GPIOA, EN );	//ʹ�ܿ�
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
//				  GPIO_Pin_6|GPIO_Pin_7;		//ʹGPIOA��״̬��ԭ������ģʽ
//	p.GPIO_Mode = GPIO_Mode_Out_PP;
//	p.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init( GPIOA, &p  );
}
/***********************************��æ����********************************************/

/***********************************д���û��Զ���ͼ��********************************************/
void WUserImg(unsigned char pos,unsigned char *ImgInfo){ //д���û��Զ���ͼ��
	unsigned char cgramAddr;			//CGRAM���û��Զ����ַ�λ
	
	if( pos <= 1 ) cgramAddr = 0x40;		// 
	if( pos > 1 && pos <= 3 ) cgramAddr = 0x50;
	if( pos > 3 && pos <= 5 ) cgramAddr = 0x60;
	if( pos > 5 && pos <= 7 ) cgramAddr = 0x70;

	LCD_WRITE_CMD( (cgramAddr + (pos%2) * 8) );	//ָ����ģд��ĵ�ַ��һ���0x40��ʼ��0x78����
	
	while( *ImgInfo != '\0' ){		//ѭ��д��tem����,���û�ȡģ������
		LCD_WRITE_ByteDATA( *ImgInfo );
		ImgInfo++;
	}
}
/***********************************д���û��Զ���ͼ��********************************************/
