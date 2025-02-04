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
#include "cycle_buffer_io.h"
#include <stdarg.h>
#include <string.h>
#include <rtthread.h>
#include <stdio.h>

#include "stm32f10x_dma.h"
#include "UART2_DMA.h"

#define WAIT_LOCK_TIME_MAX RT_WAITING_FOREVER

struct rt_semaphore sem_transmit_complete;
struct rt_semaphore sem_receive_complete;

ALIGN(RT_ALIGN_SIZE)
static char thread_transmit_stack[1024];
static struct rt_thread thread_transmit;

static void thread_transmit_entry(void *param);

ALIGN(RT_ALIGN_SIZE)
static char thread_receive_stack[1024];
static struct rt_thread thread_receive;


unsigned int receive_length;
static void thread_receive_entry(void *param);

/*       私有方法声明             */
/*---------------------------------------------------------------------------------*/
static boollen is_buffer_empty  (cycleBuffer* ibuffer);
static uInt32  get_buffer_len   (cycleBuffer *i_buffer);
static uInt32  write_buffer     (cycleBuffer *i_buffer,  void *datas, uInt32 len);
static uInt32  read_buffer      (struct _buffer *i_buffer, void *datas, uInt32 len);
__inline void begain_transmit(uInt32 length);
__inline void stop_transmit(void);
__inline void transmit_little(uInt32 length);
__inline void rise_erro(void *err);
static void init_hardware(void);
static void os_enter_lock(cycleBuffer *thisbuff, enum TRLOCK lock_type);
static void os_leave_lock(cycleBuffer *thisbuff, enum TRLOCK lock_type);
static void noos_enter_lock(cycleBuffer *thisbuff, enum TRLOCK lock_type);
static void noos_leave_lock(cycleBuffer *thisbuff, enum TRLOCK lock_type);
/*---------------------------------------------------------------------------------*/



/*       私有数据成员声明        */
/*---------------------------------------------------------------------------------*/

/*send_cycle_buffer是发送环形缓冲区

  datas  -->   send_cycle_buffer  -->  transmit_buffer  -->  usart
       fuction                   auto             auto
     when called             when dma free     when dma free     
*/
static uInt8 send_cycle_buffer [SEND_BUFFER_SIZE]; 
static uInt8 transmit_buffer   [TRANSMIT_BUFFER_SIZE];                       //供DMA发送数据使用的缓冲区    

static uInt8 get_cycle_buffer [GET_BUFFER_SIZE];                       //储存待接收数据的环形缓冲区
uInt8 receive_buffer   [RECEIVE_BUFFER_SIZE*2] = {0};                       //供DMA接收数据使用的缓冲区  

static flagStatus is_send_close = mSET;

/*系统terminal发送用的环形缓冲区
*/
cycleBuffer sysTransmitBuffer = 
{
    send_cycle_buffer, 
    SEND_BUFFER_SIZE, 
    0, 0, 
    is_buffer_empty, 
    get_buffer_len, 
    write_buffer,
    read_buffer,
    os_enter_lock,
    os_leave_lock
};

/*系统terminal接收用的环形缓冲区
*/
cycleBuffer sysReceiveBuffer = 
{
    get_cycle_buffer, 
    RECEIVE_BUFFER_SIZE, 
    0, 0, 
    is_buffer_empty, 
    get_buffer_len, 
    write_buffer,
    read_buffer,
    os_enter_lock,
    os_leave_lock
};

/*---------------------------------------------------------------------------------*/
//需要实现的函数
/*---------------------------------------------------------------------------------*/

//开始发送
//需要将transmit_buffer内的数据发送出去
__inline void begain_transmit(uInt32 length)
{
    uart2_tx_now(length);
}

//停止发送
//需要停止对transmit_buffer内数据的发送
__inline void stop_transmit(void)
{
    uart2_tx_stop();
}

//发送少量数据
//这个是底层用于清空缓冲区的
__inline void transmit_little(uInt32 length)
{
    uart2_tx_now(length);
}

//引起异常
//主要应对接收缓冲区满的处理
__inline void rise_erro(void *err)
{
    ;
}

//初始化串口硬件
static void init_hardware(void)
{
    set_buffer(transmit_buffer, TRANSMIT_BUFFER_SIZE, receive_buffer, RECEIVE_BUFFER_SIZE);
    Initial_DMA();
    Initial_UART2();
}

#ifdef NORTOS
static flagStatus write_lock_flag = mRESET;
static flagStatus read_lock_flag = mRESET;
#else
static struct rt_mutex write_lock_mutex;
static struct rt_mutex read_lock_mutex; 
#endif

static void noos_enter_lock(cycleBuffer *thisbuff, enum TRLOCK lock_type)
{
    ;
}

static void noos_leave_lock(cycleBuffer *thisbuff, enum TRLOCK lock_type)
{
    ;
}

static void os_enter_lock(cycleBuffer *thisbuff, enum TRLOCK lock_type)
{
#ifdef NORTOS
    if(lock_type == WRITE)
    {
        if(write_lock_flag == mRESET)
        {
            write_lock_flag = mSET;
        }else
        {
            return;
        }
    }else
    {
        if(read_lock_flag == mRESET)
        {
            read_lock_flag = mSET;
        }else
        {
            return;
        }
    }
#else
    if(lock_type == WRITE)
    {
        rt_mutex_take(&write_lock_mutex, WAIT_LOCK_TIME_MAX);
    }else
    {
        rt_mutex_take(&read_lock_mutex, WAIT_LOCK_TIME_MAX);
    }
#endif

}

static void os_leave_lock(cycleBuffer *thisbuff, enum TRLOCK lock_type)
{
#ifdef NORTOS
    if(lock_type == WRITE)
    {
        write_lock_flag = mRESET;
    }else
    {
        read_lock_flag = mRESET;
    }
#else
    if(lock_type == WRITE)
    {
        rt_mutex_release(&write_lock_mutex);
    }else
    {
        rt_mutex_release(&read_lock_mutex);
    }
#endif
}


/*---------------------------------------------------------------------------------*/
//结束
/*---------------------------------------------------------------------------------*/

/*
名称：is_buffer_empty
描述：判断循环缓冲区是否为空
输入：指向循环缓冲区的指针
输出：mTRUE/mFALSE
*/
static boollen is_buffer_empty(cycleBuffer* ibuffer)
{
    if(ibuffer->front == ibuffer->rear)
    {
        return mTRUE;
    }else
    {
        return mFALSE;
    }
}

/*
名称：get_buffer_len
描述：得到缓冲区长度
输入：指向循环缓冲区的指针
输出：缓冲区长度
*/
static uInt32 get_buffer_len(cycleBuffer *i_buffer)
{
    uInt32 len = 0;
    
    //len      = (rear - front + size) % size
    len        = (i_buffer->rear - i_buffer->front + i_buffer->size) % i_buffer->size;
    
    return len;
}

/*
名称：write_buffer
描述：向缓冲区写入一定长度的内容，该函数不判断写入内容长度，调用时须注意
        是否有足够空间写完所有内容，如果过长将在溢出时返回错误
输入：i_buffer:指向循环缓冲区的指针, datas:指向待写入内容的指针
        len:待写入内容的长度（字节数）
输出：实际写入的字节数
*/
static uInt32 write_buffer(cycleBuffer *i_buffer,  void *datas, uInt32 len)
{
    uInt32 i, r_len      = 0;
    uInt8 (*wdatas)[len] = datas;       //将datas转换成指向数组的指针
    
    i_buffer->enter_lock(i_buffer, WRITE);
    for(i = 0; i < len; i++)
    {
        if((i_buffer->rear + 1) % i_buffer->size == i_buffer->front)
        {
            //上溢
            break;
        }else
        {
            i_buffer->rear                   = (i_buffer->rear + 1) % i_buffer->size;
            i_buffer->buffer[i_buffer->rear] = (*wdatas)[i];
            r_len ++;
        }
    }
    i_buffer->leave_lock(i_buffer, WRITE);
    return r_len;
}

/*
名称：read_buffer
描述：读取缓冲区内容
输入：i_buffer:指向循环缓冲区的指针, datas:指向输出地址
        len:读取数据长度（字节数）
        本函数不判断欲读取数据长度是否超过缓冲区内容长度，调用前须自行判断
        如果超出，将在超出时返回mERROR
输出: 读到的长度
*/
static uInt32 read_buffer(struct _buffer *i_buffer, void *datas, uInt32 len)
{
    uInt32 i, r_len       = 0;
    uInt8  (*rdatas)[len] = datas;       //将datas转换成指向数组的指针
    
    i_buffer->enter_lock(i_buffer, READ);
    /*循环队列出队操作
    */
    for(i = 0; i < len; i++)
    {
        if(i_buffer->rear == i_buffer->front)
        {
            //下溢
            break;
        }else
        {
            i_buffer->front = (i_buffer->front + 1) % i_buffer->size;
            (*rdatas)[i]    = i_buffer->buffer[i_buffer->front];
            r_len ++;
        }
    }
    i_buffer->leave_lock(i_buffer, READ);
    return r_len;
}

/*
名称：get_buffer_len
描述：得到缓冲区长度
输入：指向循环缓冲区的指针
输出：缓冲区长度
*/
void init_cycle_buffer(cycleBuffer* ibuffer, uInt8 *buffs, uInt32 size)
{
    ibuffer->buffer   = buffs;
    ibuffer->size     = size;
    ibuffer->front    = 0;
    ibuffer->rear     = 0;
    
    ibuffer->get_len  = get_buffer_len;
    ibuffer->is_empty = is_buffer_empty;
    ibuffer->read     = read_buffer;
    ibuffer->write    = write_buffer;
    ibuffer->enter_lock = noos_enter_lock;
    ibuffer->leave_lock = noos_leave_lock;
}

void init_tx_rx(void)
{
    rt_err_t rt_e;
    init_hardware();
#ifndef NORTOS
    rt_e = rt_thread_init(&thread_receive, "rx", thread_receive_entry, 
        RT_NULL, thread_receive_stack, sizeof(thread_receive_stack), 6, 5);
    if(rt_e == RT_EOK)
    {
        rt_thread_startup(&thread_receive);
    }
    rt_e = rt_thread_init(&thread_transmit, "tx", thread_transmit_entry, 
            RT_NULL, thread_transmit_stack, sizeof(thread_transmit_stack), 6, 5);
    if(rt_e == RT_EOK)
    {
        rt_thread_startup(&thread_transmit);
    }
    
    rt_mutex_init(&write_lock_mutex, "txesp", RT_IPC_FLAG_FIFO);
    rt_mutex_init(&read_lock_mutex,  "rxesp", RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem_receive_complete, "rxok", 1, RT_IPC_FLAG_FIFO);
    rt_sem_take(&sem_receive_complete, RT_WAITING_FOREVER);
    rt_sem_init(&sem_transmit_complete, "txok", 1, RT_IPC_FLAG_FIFO);
    rt_sem_take(&sem_transmit_complete, RT_WAITING_FOREVER);   
#endif    
}

/*
名称：t_send
描述：使用终端发送数据
输入：datas: 待写入数据指针，len: 欲写入字节数
输出：实际写入字节数
*/
uInt32 t_send(void *datas, uInt32 len)
{
    unsigned int ret;
    unsigned char *write_datas = (unsigned char *)datas; 
    
    ret = sysTransmitBuffer.write(&sysTransmitBuffer, write_datas, len);
    if(is_send_close == mSET)
    {
        sysTransmitBuffer.read(&sysTransmitBuffer, &transmit_buffer, TRANSMIT_BUFFER_SIZE);       
        is_send_close = mRESET;
        begain_transmit(ret);
    }
    
    return ret;
}

/*
名称：t_putstr
描述：输出字符串
输入：str，待输出字符串指针
输出：无
*/
void t_putstr(const char *str)
{
    t_send((void *)str, strlen(str));
}

/*
名称：t_print
描述：使用终端格式化输出
输入：参考printf
输出：无
*/
void t_print(const char *format, ...)
{
    va_list args;
    char buf[256];
    va_start(args, format);
    vsprintf(buf, format, args);       
    va_end (args);    
    t_putstr(buf);
}

/*
名称：t_receive
描述：从终端读数据
输入：datas：写入数据指针，len: 欲读字节数
输出：实际读到字节数
*/
uInt32 t_receive(void *datas, uInt32 len)
{
    unsigned int ret;
     
    ret = sysReceiveBuffer.read(&sysReceiveBuffer, datas, len);
    
    return ret;
}

/*
名称：t_putstr
描述：读字符串
输入：str，待输出字符串指针
输出：读到多少字符
*/
int t_getstr(char *string)
{
    char buf[2];
    int i = 0;
    
    if(string == NULL)
    {
        return -1;
    }
    while(t_receive(buf, 1) > 0)
    {
        string[i++] = buf[0];
        if(buf[0] == '0')
        {
            break;
        }    
    }
    
    return i;
}

static void thread_transmit_entry(void *param)
{
    uInt32 len = 0;
    
    while(1)
    {
        rt_sem_take(&sem_transmit_complete, RT_WAITING_FOREVER);

        len = sysTransmitBuffer.read(&sysTransmitBuffer, &transmit_buffer, TRANSMIT_BUFFER_SIZE);
        if (len != 0)
        {             
            is_send_close = mRESET;
            transmit_little(len);
        }
        else
        {
            //关闭发送
            stop_transmit();
            is_send_close = mSET;
        }
    }
}

static void thread_receive_entry(void *param)
{
    uInt32 actual_len = 0;
    while(1)
    {
        rt_sem_take(&sem_receive_complete, RT_WAITING_FOREVER);
        
        if(receive_length > RECEIVE_BUFFER_SIZE)
        {
            receive_length = RECEIVE_BUFFER_SIZE;
            rise_erro("2big2write");
        }
        
        actual_len = sysReceiveBuffer.write(&sysReceiveBuffer, &receive_buffer, receive_length);
        if (actual_len != receive_length)
        {                
            //异常，有一部分数据没能写入，丢失了数据
            rise_erro("lost_data");
        }else
        {
            //正常写入了一次
            ;
        }
    }
}

/*
名称：transmit_finish_handler
描述：
输入：无
输出：无
*/
void transmit_finish_handler()
{
    rt_sem_release(&sem_transmit_complete);
}


/*
名称：transmit_finish_handler
描述：
输入：无
输出：无
*/
void receive_finish_handler(unsigned int length)
{
    receive_length = length;
    rt_sem_release(&sem_receive_complete);
}


