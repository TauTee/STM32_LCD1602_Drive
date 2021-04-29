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
#include "my_terminal.h"
#include <stdarg.h>
#include <string.h>


/*       ˽�з�������             */
/*---------------------------------------------------------------------------------*/
static boollen is_buffer_empty  (cycleBuffer* ibuffer);
static uInt16  get_buffer_len   (cycleBuffer *i_buffer);
static uInt16  write_buffer     (cycleBuffer *i_buffer,  void *datas, uInt16 len);
static uInt16  read_buffer      (struct _buffer *i_buffer, void *datas, uInt16 len);
/*---------------------------------------------------------------------------------*/



/*       ˽�����ݳ�Ա����        */
/*---------------------------------------------------------------------------------*/

/*send_cycle_buffer�Ƿ��ͻ��λ�����

  datas  -->   send_cycle_buffer  -->  dma_buffer  -->  usart
       fuction                   auto             auto
     when called             when dma free     when dma free     
*/
static uInt8 send_cycle_buffer [SEND_BUFFER_SIZE];                       //������������ݵĻ��λ�����
static uInt8 dma_buffer        [DMA_BUFFER__SIZE];                       //��DMA��������ʹ�õĻ�����    

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
    read_buffer
};

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
static uInt16 get_buffer_len(cycleBuffer *i_buffer)
{
    uInt16 len = 0;
    
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
static uInt16 write_buffer(cycleBuffer *i_buffer,  void *datas, uInt16 len)
{
    uInt16 i, r_len      = 0;
    uInt8 (*wdatas)[len] = datas;       //��datasת����ָ�������ָ��
    
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
    
    return r_len;
}

/*
���ƣ�read_buffer
��������ȡ����������
���룺i_buffer:ָ��ѭ����������ָ��, datas:ָ�������ַ
        len:��ȡ���ݳ��ȣ��ֽ�����
        ���������ж�����ȡ���ݳ����Ƿ񳬹����������ݳ��ȣ�����ǰ�������ж�
        ������������ڳ���ʱ����mERROR
���: mSUCCESS/mERROR
*/
static uInt16 read_buffer(struct _buffer *i_buffer, void *datas, uInt16 len)
{
    uInt16 i, r_len       = 0;
    uInt8  (*rdatas)[len] = datas;       //��datasת����ָ�������ָ��
    
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
    return r_len;
}

/*
���ƣ�get_buffer_len
�������õ�����������
���룺ָ��ѭ����������ָ��
���������������
*/
void init_cycle_buffer(cycleBuffer* ibuffer, uInt8 *buffs, uInt16 size)
{
    ibuffer->buffer   = buffs;
    ibuffer->size     = size;
    ibuffer->front    = 0;
    ibuffer->rear     = 0;
    
    ibuffer->get_len  = get_buffer_len;
    ibuffer->is_empty = is_buffer_empty;
    ibuffer->read     = read_buffer;
    ibuffer->write    = write_buffer;
}

/*
���ƣ�t_send
������ʹ���ն˷�������
���룺datas: ��д������ָ�룬len: ��д���ֽ���
�����ʵ��д���ֽ���
*/
uInt16 t_send(void *datas, uInt16 len)
{
    unsigned int ret;
     
    ret = sysTransmitBuffer.write(&sysTransmitBuffer, datas, len);
    DMA_Cmd(DMA2_Stream7, ENABLE);
    
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
    va_start (args, format);
    vsprintf (buf, format, args);       
    va_end (args);    
    t_putstr(buf);
}

/*
���ƣ�t_receive
���������ն˶�����
���룺datas��д������ָ�룬len: �����ֽ���
�����ʵ�ʶ����ֽ���
*/
uInt16 t_receive(void *datas, uInt16 len)
{
    return 0;
}

/*
���ƣ�init_terminal_usart
��������ʼ���ն����贮��
���룺baudrate: ������
�������
*/
static void init_terminal_usart(uInt32 baudrate)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

#if !TEST_ON_JIANGXIN
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    
    GPIO_InitStructure. GPIO_Pin   = GPIO_Pin_9 ;
    GPIO_InitStructure. GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure. GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure. GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure. GPIO_OType = GPIO_OType_PP;    
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin    = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
#else
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
    
    GPIO_InitStructure. GPIO_Pin   = GPIO_Pin_6 ;
    GPIO_InitStructure. GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure. GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure. GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure. GPIO_OType = GPIO_OType_PP;    
    GPIO_Init(GPIOB, &GPIO_InitStructure);  
    
    GPIO_InitStructure.GPIO_Pin    = GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
    
    USART_DeInit(USART1);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_InitStructure. USART_BaudRate            = baudrate;  
    USART_InitStructure. USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure. USART_StopBits            = USART_StopBits_1;
    USART_InitStructure. USART_Parity              = USART_Parity_No;
    USART_InitStructure. USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure. USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
 
//    NVIC_InitStructure. NVIC_IRQChannel = USART1_IRQn;
//    NVIC_InitStructure. NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure. NVIC_IRQChannelSubPriority = 0;
//    NVIC_InitStructure. NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure); 
  
    //USART_DMACmd(USART1, USART_DMAReq_Rx,ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);   
    //USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);  
//    USART_ITConfig(USART1, USART_IT_TC,  ENABLE);
    
    //USART_ITConfig(USART1, USART_IT_ERR, ENABLE);    
    USART_Cmd(USART1, ENABLE);  
}

/*
���ƣ�init_terminal_usart
��������ʼ���ն����贮��
���룺baudrate: ������
�������
*/
static void init_terminal_dma()
{
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef  DMA_InitStructure;
    __IO uint32_t    Timeout = 10000;

    /* Enable DMA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    /* Reset DMA Stream registers (for debug purpose) */
    DMA_DeInit(DMA2_Stream7);

    /* Check if the DMA Stream is disabled before enabling it.
     Note that this step is useful when the same Stream is used multiple times:
     enabled, then disabled then re-enabled... In this case, the DMA Stream disable
     will be effective only at the end of the ongoing data transfer and it will 
     not be possible to re-configure it before making sure that the Enable bit 
     has been cleared by hardware. If the Stream is used only once, this step might 
     be bypassed. */
    while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE)
    {
    }

    /* Configure DMA Stream */
    DMA_InitStructure. DMA_Channel            = DMA_Channel_4;  
    DMA_InitStructure. DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);     //�ò���ʵ������const uint32_t*������FLASH��Դ
    DMA_InitStructure. DMA_Memory0BaseAddr    = (uint32_t)dma_buffer;              //�ò���ʵ������uint32*����ram�ϣ�Ŀ��
    DMA_InitStructure. DMA_DIR                = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure. DMA_BufferSize         = sizeof(dma_buffer);                            //Դ�����С����λ���ֶ����ֽ�
    DMA_InitStructure. DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure. DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure. DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure. DMA_MemoryDataSize     = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure. DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure. DMA_Priority           = DMA_Priority_High;
    DMA_InitStructure. DMA_FIFOMode           = DMA_FIFOMode_Disable;         
    DMA_InitStructure. DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
    DMA_InitStructure. DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    DMA_InitStructure. DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream7, &DMA_InitStructure);

    /* Enable DMA Stream Transfer Complete interrupt */
    DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

    /* Enable the DMA Stream IRQ Channel */
    NVIC_InitStructure. NVIC_IRQChannel                   = DMA2_Stream7_IRQn;
    NVIC_InitStructure. NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure. NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure. NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
���ƣ�init_terminal_hardware
�������й��ն˵�Ӳ����ʼ��
���룺��
�������
*/
static void init_terminal_hardware(uInt32 bound)
{
    init_terminal_usart(bound);
    init_terminal_dma();
}

/*
���ƣ�init_terminal_hardware
�������й��ն˵�Ӳ����ʼ��
���룺��
�������
*/
void init_sys_terminal(uInt32 bound)
{
    init_terminal_hardware(bound);
}

/*
���ƣ�DMA2_Stream7_IRQHandler
������DMA�жϺ��������ͽ���������ӻ�����ȡ����ͨ��DMA����
���룺��
�������
*/
void DMA2_Stream7_IRQHandler()
{
    uint16_t len = 0;
    
    if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
        len = sysTransmitBuffer.read(&sysTransmitBuffer, &dma_buffer, sizeof(dma_buffer));
        if (len != 0)
        {                
            DMA_Cmd(DMA2_Stream7, DISABLE);
            DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF3);
            DMA_SetCurrDataCounter(DMA2_Stream7, len);        
            DMA_Cmd(DMA2_Stream7, ENABLE);
        }
        else
        {
            DMA_Cmd(DMA2_Stream7, DISABLE);
        }
        DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
    }
}

