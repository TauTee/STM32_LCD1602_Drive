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
#ifndef __MYTERMINAL_H
#define __MYTERMINAL_H

#include "sys.h"
#include "my_type.h"

/*
 *���д����͵����ݶ������뻷�λ�����
 *��ÿ�δ��ڷ��ͽ��������жϣ���ӻ��λ�������ȡ�����ݷ���DMA������
 *DMA���Զ���������������ͨ�����ڷ��ͳ�ȥ
*/
#define SEND_BUFFER_SIZE 32768     //���ͻ��λ�����
#define DMA_BUFFER__SIZE 16384     //DMA���ͻ�����

/*
������cycleBuffer
���������λ�����
*/
typedef struct _buffer
{
    uInt8  *buffer;      //��������
    uInt16 size;        //���г���
    uInt16 front;       //����ͷ�±�
    uInt16 rear;        //����β�±�
    boollen (*is_empty)(struct _buffer *this_buffer);      //�жϻ������Ƿ�Ϊ��
    uInt16  (*get_len)(struct _buffer *this_buffer);        //���ػ��������ݳ���
    uInt16  (*write)(struct _buffer *this_buffer,  void *datas, uInt16 len);    //д������
    uInt16  (*read)(struct _buffer *this_buffer, void *datas, uInt16 len);      //��������
}cycleBuffer;

/*          ����ӿ�            */

void init_sys_terminal(uInt32 bound);       //��ʼ��ϵͳ�ն�   
void t_putstr(const char *str);             //����ַ���
void t_print(const char *format, ...);      //��ʽ�����
uInt16 t_send(void *datas, uInt16 len);     //����һ���ֽڳ�������
uInt16 t_receive(void *datas, uInt16 len);  //��һ���ֽڳ������� 

void init_cycle_buffer(cycleBuffer* ibuffer, uInt8 *buffs, uInt16 size);        //��ʼ��cycleBuffer��

#endif
