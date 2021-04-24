/**
******************************************************************************
*名称：my_foc
*描述：空间矢量控制在永磁同步电机上的实现
*主控：STM32F405RGT6
*日期：2020年8月19日
*作者：张凯强
******************************************************************************
*描述：一个基于串口的简单终端实现
*日期：2020年9月30日
******************************************************************************
*/
#ifndef __MYTERMINAL_H
#define __MYTERMINAL_H

#include "sys.h"
#include "my_type.h"

/*
 *所有待发送的数据都被放入环形缓冲区
 *当每次串口发送结束进入中断，会从环形缓冲区内取出数据放入DMA缓冲区
 *DMA会自动将缓冲区内数据通过串口发送出去
*/
#define SEND_BUFFER_SIZE 32768     //发送环形缓冲区
#define DMA_BUFFER__SIZE 16384     //DMA发送缓冲区

/*
类名：cycleBuffer
描述：环形缓冲区
*/
typedef struct _buffer
{
    uInt8  *buffer;      //队列数组
    uInt16 size;        //队列长度
    uInt16 front;       //队列头下标
    uInt16 rear;        //队列尾下标
    boollen (*is_empty)(struct _buffer *this_buffer);      //判断缓冲区是否为空
    uInt16  (*get_len)(struct _buffer *this_buffer);        //返回缓冲区内容长度
    uInt16  (*write)(struct _buffer *this_buffer,  void *datas, uInt16 len);    //写缓冲区
    uInt16  (*read)(struct _buffer *this_buffer, void *datas, uInt16 len);      //读缓冲区
}cycleBuffer;

/*          对外接口            */

void init_sys_terminal(uInt32 bound);       //初始化系统终端   
void t_putstr(const char *str);             //输出字符串
void t_print(const char *format, ...);      //格式化输出
uInt16 t_send(void *datas, uInt16 len);     //发送一定字节长度数据
uInt16 t_receive(void *datas, uInt16 len);  //读一定字节长度数据 

void init_cycle_buffer(cycleBuffer* ibuffer, uInt8 *buffs, uInt16 size);        //初始化cycleBuffer类

#endif
