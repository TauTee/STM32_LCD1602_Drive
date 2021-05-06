/**
******************************************************************************
*���ƣ�my_foc
*�������ռ�ʸ������������ͬ������ϵ�ʵ��
*���أ�STM32F405RGT6
*���ڣ�2020��8��19��
*���ߣ��ſ�ǿ
******************************************************************************
*������һ�����ڴ��ڵļ��ն�ʵ��
*���ڣ�2020��9��30��
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

/*       ˽�з�������             */
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



/*       ˽�����ݳ�Ա����        */
/*---------------------------------------------------------------------------------*/

/*send_cycle_buffer�Ƿ��ͻ��λ�����

  datas  -->   send_cycle_buffer  -->  transmit_buffer  -->  usart
       fuction                   auto             auto
     when called             when dma free     when dma free     
*/
static uInt8 send_cycle_buffer [SEND_BUFFER_SIZE]; 
static uInt8 transmit_buffer   [TRANSMIT_BUFFER_SIZE];                       //��DMA��������ʹ�õĻ�����    

static uInt8 get_cycle_buffer [GET_BUFFER_SIZE];                       //������������ݵĻ��λ�����
uInt8 receive_buffer   [RECEIVE_BUFFER_SIZE*2] = {0};                       //��DMA��������ʹ�õĻ�����  

static flagStatus is_send_close = mSET;

/*ϵͳterminal�����õĻ��λ�����
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

/*ϵͳterminal�����õĻ��λ�����
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
//��Ҫʵ�ֵĺ���
/*---------------------------------------------------------------------------------*/

//��ʼ����
//��Ҫ��transmit_buffer�ڵ����ݷ��ͳ�ȥ
__inline void begain_transmit(uInt32 length)
{
    uart2_tx_now(length);
}

//ֹͣ����
//��Ҫֹͣ��transmit_buffer�����ݵķ���
__inline void stop_transmit(void)
{
    uart2_tx_stop();
}

//������������
//����ǵײ�������ջ�������
__inline void transmit_little(uInt32 length)
{
    uart2_tx_now(length);
}

//�����쳣
//��ҪӦ�Խ��ջ��������Ĵ���
__inline void rise_erro(void *err)
{
    ;
}

//��ʼ������Ӳ��
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
//����
/*---------------------------------------------------------------------------------*/

/*
���ƣ�is_buffer_empty
�������ж�ѭ���������Ƿ�Ϊ��
���룺ָ��ѭ����������ָ��
�����mTRUE/mFALSE
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
���ƣ�get_buffer_len
�������õ�����������
���룺ָ��ѭ����������ָ��
���������������
*/
static uInt32 get_buffer_len(cycleBuffer *i_buffer)
{
    uInt32 len = 0;
    
    //len      = (rear - front + size) % size
    len        = (i_buffer->rear - i_buffer->front + i_buffer->size) % i_buffer->size;
    
    return len;
}

/*
���ƣ�write_buffer
�������򻺳���д��һ�����ȵ����ݣ��ú������ж�д�����ݳ��ȣ�����ʱ��ע��
        �Ƿ����㹻�ռ�д���������ݣ���������������ʱ���ش���
���룺i_buffer:ָ��ѭ����������ָ��, datas:ָ���д�����ݵ�ָ��
        len:��д�����ݵĳ��ȣ��ֽ�����
�����ʵ��д����ֽ���
*/
static uInt32 write_buffer(cycleBuffer *i_buffer,  void *datas, uInt32 len)
{
    uInt32 i, r_len      = 0;
    uInt8 (*wdatas)[len] = datas;       //��datasת����ָ�������ָ��
    
    i_buffer->enter_lock(i_buffer, WRITE);
    for(i = 0; i < len; i++)
    {
        if((i_buffer->rear + 1) % i_buffer->size == i_buffer->front)
        {
            //����
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
���ƣ�read_buffer
��������ȡ����������
���룺i_buffer:ָ��ѭ����������ָ��, datas:ָ�������ַ
        len:��ȡ���ݳ��ȣ��ֽ�����
        ���������ж�����ȡ���ݳ����Ƿ񳬹����������ݳ��ȣ�����ǰ�������ж�
        ������������ڳ���ʱ����mERROR
���: �����ĳ���
*/
static uInt32 read_buffer(struct _buffer *i_buffer, void *datas, uInt32 len)
{
    uInt32 i, r_len       = 0;
    uInt8  (*rdatas)[len] = datas;       //��datasת����ָ�������ָ��
    
    i_buffer->enter_lock(i_buffer, READ);
    /*ѭ�����г��Ӳ���
    */
    for(i = 0; i < len; i++)
    {
        if(i_buffer->rear == i_buffer->front)
        {
            //����
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
���ƣ�get_buffer_len
�������õ�����������
���룺ָ��ѭ����������ָ��
���������������
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
���ƣ�t_send
������ʹ���ն˷�������
���룺datas: ��д������ָ�룬len: ��д���ֽ���
�����ʵ��д���ֽ���
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
���ƣ�t_putstr
����������ַ���
���룺str��������ַ���ָ��
�������
*/
void t_putstr(const char *str)
{
    t_send((void *)str, strlen(str));
}

/*
���ƣ�t_print
������ʹ���ն˸�ʽ�����
���룺�ο�printf
�������
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
���ƣ�t_receive
���������ն˶�����
���룺datas��д������ָ�룬len: �����ֽ���
�����ʵ�ʶ����ֽ���
*/
uInt32 t_receive(void *datas, uInt32 len)
{
    unsigned int ret;
     
    ret = sysReceiveBuffer.read(&sysReceiveBuffer, datas, len);
    
    return ret;
}

/*
���ƣ�t_putstr
���������ַ���
���룺str��������ַ���ָ��
��������������ַ�
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
            //�رշ���
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
            //�쳣����һ��������û��д�룬��ʧ������
            rise_erro("lost_data");
        }else
        {
            //����д����һ��
            ;
        }
    }
}

/*
���ƣ�transmit_finish_handler
������
���룺��
�������
*/
void transmit_finish_handler()
{
    rt_sem_release(&sem_transmit_complete);
}


/*
���ƣ�transmit_finish_handler
������
���룺��
�������
*/
void receive_finish_handler(unsigned int length)
{
    receive_length = length;
    rt_sem_release(&sem_receive_complete);
}


