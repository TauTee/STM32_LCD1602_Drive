#ifndef __CYCLE_BUFFER_IO_H
#define __CYCLE_BUFFER_IO_H

#include "my_type.h"

/*
 *���д����͵����ݶ������뻷�λ�����
 *��ÿ�δ��ڷ��ͽ��������жϣ���ӻ��λ�������ȡ�����ݷ���DMA������
 *DMA���Զ���������������ͨ�����ڷ��ͳ�ȥ
*/
#define SEND_BUFFER_SIZE 1024     //���ͻ��λ�����
#define TRANSMIT_BUFFER_SIZE 128     //DMA���ͻ�����

#define GET_BUFFER_SIZE 1024     //���ͻ��λ�����
#define RECEIVE_BUFFER_SIZE 128     //DMA���ͻ�����

enum TRLOCK
{
    WRITE = 0,
    READ  = !WRITE
};

/*
������cycleBuffer
���������λ�����
*/
typedef struct _buffer
{
    uInt8  *buffer;      //��������
    uInt32 size;        //���г���
    uInt32 front;       //����ͷ�±�
    uInt32 rear;        //����β�±�
    boollen (*is_empty)(struct _buffer *this_buffer);      //�жϻ������Ƿ�Ϊ��
    uInt32  (*get_len)(struct _buffer *this_buffer);        //���ػ��������ݳ���
    uInt32  (*write)(struct _buffer *this_buffer,  void *datas, uInt32 len);    //д������
    uInt32  (*read)(struct _buffer *this_buffer, void *datas, uInt32 len);      //��������
    void    (*enter_lock)(struct _buffer *this_buffer, enum TRLOCK lock_type);
    void    (*leave_lock)(struct _buffer *this_buffer, enum TRLOCK lock_type);
}cycleBuffer;

/*          ����ӿ�            */

void t_putstr(const char *str);             //����ַ���
void t_print(const char *format, ...);      //��ʽ�����
uInt32 t_send(void *datas, uInt32 len);     //����һ���ֽڳ�������
uInt32 t_receive(void *datas, uInt32 len);  //��һ���ֽڳ������� 
int t_getstr(char *string);                 //���ַ���

void init_cycle_buffer(cycleBuffer* ibuffer, uInt8 *buffs, uInt32 size);        //��ʼ��cycleBuffer��
void init_tx_rx(void);

//��Ҫ���ݾ���ʵ�ַ�ʽ���õĺ���
//ÿ�η�����ɺ���øýӿ�
void transmit_finish_handler(void);
void receive_finish_handler(unsigned int length);
#endif
