#ifndef __CYCLE_BUFFER_IO_H
#define __CYCLE_BUFFER_IO_H

#include "my_type.h"

/*
 *所有待发送的数据都被放入环形缓冲区
 *当每次串口发送结束进入中断，会从环形缓冲区内取出数据放入DMA缓冲区
 *DMA会自动将缓冲区内数据通过串口发送出去
*/
#define SEND_BUFFER_SIZE 1024     //发送环形缓冲区
#define TRANSMIT_BUFFER_SIZE 128     //DMA发送缓冲区

#define GET_BUFFER_SIZE 1024     //发送环形缓冲区
#define RECEIVE_BUFFER_SIZE 128     //DMA发送缓冲区

enum TRLOCK
{
    WRITE = 0,
    READ  = !WRITE
};

/*
类名：cycleBuffer
描述：环形缓冲区
*/
typedef struct _buffer
{
    uInt8  *buffer;      //队列数组
    uInt32 size;        //队列长度
    uInt32 front;       //队列头下标
    uInt32 rear;        //队列尾下标
    boollen (*is_empty)(struct _buffer *this_buffer);      //判断缓冲区是否为空
    uInt32  (*get_len)(struct _buffer *this_buffer);        //返回缓冲区内容长度
    uInt32  (*write)(struct _buffer *this_buffer,  void *datas, uInt32 len);    //写缓冲区
    uInt32  (*read)(struct _buffer *this_buffer, void *datas, uInt32 len);      //读缓冲区
    void    (*enter_lock)(struct _buffer *this_buffer, enum TRLOCK lock_type);
    void    (*leave_lock)(struct _buffer *this_buffer, enum TRLOCK lock_type);
}cycleBuffer;

/*          对外接口            */

void t_putstr(const char *str);             //输出字符串
void t_print(const char *format, ...);      //格式化输出
uInt32 t_send(void *datas, uInt32 len);     //发送一定字节长度数据
uInt32 t_receive(void *datas, uInt32 len);  //读一定字节长度数据 
int t_getstr(char *string);                 //读字符串

void init_cycle_buffer(cycleBuffer* ibuffer, uInt8 *buffs, uInt32 size);        //初始化cycleBuffer类
void init_tx_rx(void);

//需要根据具体实现方式调用的函数
//每次发送完成后调用该接口
void transmit_finish_handler(void);
void receive_finish_handler(unsigned int length);
#endif
