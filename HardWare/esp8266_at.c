#include "esp8266_at.h"
#include "stdio.h"
#include "string.h"
#include "cycle_buffer_io.h"
#include <rtthread.h>

static void esp_puts(unsigned char *istr)
{   
    t_send(istr, strlen(istr));
}

__inline void esp_send(void *datas, unsigned int length)
{
    t_send(datas, length);
}

esp_erro_status esp_wait_ans(unsigned char *ians)
{
#define NOFIND 0
#define FINDED 1
    unsigned char            *wait_str     = ians;               //等待的字符串
    unsigned int    wait_str_len  = strlen(wait_str);   //等待的字符串的长度
    unsigned char            read_ch       = '\0';               //中间变量，读到的字符
    unsigned char   status        = NOFIND;             //有没有找到该字符串
    unsigned int    find_index    = 0;                  //中间变量，找到字符串的第几位了
    unsigned int    begain_time   = rt_tick_get();      //用于记录开始找的时间，做超时检测
    esp_erro_status return_status = ESP_SUC;            //用于保证程序的单一出口原则
    
    //找不到就一直找，直到超时
    //找的时候如果缓冲区已经读完了，就等一会再找
    while ((rt_tick_get() - begain_time < MAX_WAIT_TIME))
    {
        //读一个字符，根据当前状态决定对比字符串的第几个字符
        //没找到就对比第一个
        //找到了一个以后就根据find_index决定对比哪个字符
        if(t_receive(&read_ch, 1))
        {
            switch (status)
            {
            case NOFIND:
                if (read_ch == wait_str[0])
                {
                    status = FINDED;
                    find_index = 1;
                }
                break;

            case FINDED:
                if (read_ch != wait_str[find_index])
                {
                    status = NOFIND;
                    find_index = 0;
                }else
                {
                    find_index++;
//                    if(find_index >= wait_str_len)
//                    {
//                        //找到了
//                        goto WAITEND;
//                    }
                }
                break;

            default:
                status = NOFIND;
                break;
            }
            if(find_index >= wait_str_len)
            {
                //找到了
                goto WAITEND;
            }
        }else
        {
            //缓冲区读完了还没找到就歇会
            rt_thread_mdelay(WAIT_REST_TIME);
        }   
    }
    
WAITEND:
    if(status == FINDED)
    {
        return_status = ESP_SUC;
    }else
    {
        return_status = ESP_ERROR;
    }   
    return return_status;
    
#undef  NO_FIND
#undef FINDED 
}

void init_esp_hardware(void)
{
    init_tx_rx();
}

esp_erro_status esp_test(void)
{
    esp_puts("AT\r\n");
    
    return esp_wait_ans("OK");
}

esp_erro_status esp_rst(void)
{
    esp_puts("AT+RST\r\n");
    
    return esp_wait_ans("OK");
}

esp_erro_status esp_set_wifi_mode(WIFIMODE imode)
{
    unsigned char cmd[] = "AT+CWMODE_DEF=0\r\n"; 
    
    cmd[sizeof(cmd)-1-3] = imode;
    esp_puts(cmd);
    
    return esp_wait_ans("OK");
}

esp_erro_status esp_connect_wifi(unsigned char *issid, unsigned char *ipassword)
{
    unsigned char cmd[50];
    
    sprintf(cmd, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", issid, ipassword);
    esp_puts(cmd);
    
    return esp_wait_ans("OK");
}

esp_erro_status esp_disconnect_wifi(void)
{
    esp_puts("AT+CWQAP\r\n");
    
    return esp_wait_ans("OK");
}

esp_erro_status esp_get_ip(void)
{
    esp_puts("AT+CIPSTA_CUR?\r\n");
    
    return esp_wait_ans("OK");
}

esp_erro_status esp_set_ip(unsigned char *iipaddress)
{
    unsigned char cmd[50];
    
    sprintf(cmd, "AT+CIPSTA_DEF=\"%s\"\r\n", iipaddress);
    esp_puts(cmd);
    
    return esp_wait_ans("OK");
}

esp_erro_status esp_build_TCPUDP(unsigned char *ip_domain, unsigned char *imode, unsigned int port)
{
    unsigned char cmd[50];
    
    sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",%d\r\n", imode, ip_domain, port);
    esp_puts(cmd);
    
    return esp_wait_ans("OK");
}

esp_erro_status esp_set_tcpmode(TCPMODE tcp_mode)
{
    unsigned char cmd[] = "AT+CIPSTART=0\r\n";
    
    cmd[sizeof(cmd)-1-2] = tcp_mode;
    esp_puts(cmd);
    
    return esp_wait_ans("OK");    
}

esp_erro_status esp_ipsend(unsigned char *idata, unsigned int length)
{
    unsigned char cmd[20];
    esp_erro_status return_status = ESP_SUC;
    
    sprintf(cmd, "AT+CIPSEND=%d\r\n", length);
    esp_puts(cmd);
    
    if(esp_wait_ans(">") == ESP_SUC)
    {
        esp_send(idata, length);
        if(esp_wait_ans("OK") != ESP_SUC)
        {
            rt_kprintf("no ok\n");
            return_status = ESP_ERROR;
        }
    }else
    {
        rt_kprintf("no >\n");
        return_status = ESP_ERROR;
    }
    
    return return_status;
}

esp_erro_status esp_stop_TCPUDP(void)
{
    esp_puts("AT+CIPCLOSE\r\n");
    
    return esp_wait_ans("OK");
}

