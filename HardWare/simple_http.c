#include <string.h>
#include <rtthread.h>
#include "my_type.h"
#include "simple_http.h"
#include "simple_socket.h"

httpResponse http_get_response(Socket *socket)
{
#define REC_VERSION 0    
#define REC_STACODE 1
#define REC_CODEDDIS 2
#define REC_CODEDDIS_R 3
#define REC_CODEDDIS_N 4
#define REC_HEADWORD_1 5
#define REC_HEADWORD_2 6
#define REC_HEADWORD_3 7
#define REC_HEADWORD_4 8
#define REC_RESPONSE 8
#define REC_TIMEOUT -1

#define zero_status()   {status = REC_VERSION;index = 0;}
    const uInt32 MAX_WAIT_TIME = 1000;
    httpResponse response;
    uInt8 data_ch;
    sInt32 status = REC_VERSION;
    uInt32 index = 0;
    unsigned int    begain_time   = rt_tick_get();
    

    
    while ((rt_tick_get() - begain_time < MAX_WAIT_TIME))
    {
        if(socket->read(socket, data_ch,1) > 0)
        {
            switch(status)
            {
                case REC_VERSION:
                    if(data_ch != ' ')
                    {
                        response.version_num[index++] = data_ch;
                        if(index > 10)
                        {
                            err;
                        }
                    }else
                    {
                        status = REC_STACODE;
                        index  = 0;
                    }
                    break;
                case REC_STACODE:
                    if(data_ch != ' ')
                    {
                        index++;
                        if(index > 3)
                        {
                            err;
                        }
                    }else
                    {
                        status = REC_STACODE;
                        index  = 0;
                    }
                    break;
                case REC_CODEDDIS:
                    if(data_ch != '\r')
                    {
                        ;
                    }else
                    {
                        status = REC_CODEDDIS_N;
                        index  = 0;
                    }
                    break;
                case REC_CODEDDIS_R:
                    if(data_ch != '\n')
                    {
                        zero_status();
                    }else
                    {
                        status = REC_HEADNAME;
                        index  = 0;
                    }
                    break;
                case REC_CODEDDIS_N:
                    if(data_ch != ':')
                    {
                        HEAD_KEY_NAME[index++] = data_ch;
                        if(index > 3)
                        {
                            err;
                        }
                    }else
                    {
                        HEAD_KEY_NAME[HEAD_WORD_I][index] = '\0';
                        status = REC_HEADWORD_1;
                        index  = 0;
                    }
                    break;
                case REC_HEADWORD_1:
                    if(data_ch != '\r')
                    {
                        
                        HEAD_KEY_VALUE[HEAD_WORD_I][index++] = data_ch;
                        if(index > 3)
                        {
                            err;
                        }
                    }else
                    {
                        status = REC_HEADWORD_2;
                        index  = 0;
                    }
                    break;
                case REC_HEADWORD_2:
                    if(data_ch != '\n')
                    {
                        zero_status();
                    }else
                    {
                        status = REC_HEADWORD_3;
                        index  = 0;
                        
                    }
                    break;
                case REC_HEADWORD_3:
                    if(data_ch != '\r')
                    {
                        index = 0;
                        HEAD_WORD_I ++;
                        HEAD_KEY_VALUE[HEAD_WORD_I][index++] = data_ch;
                        status = REC_HEADWORD_1;
                    }else
                    {
                        status = REC_HEADWORD_4;
                        index  = 0;
                    }
                    break;
                case REC_HEADWORD_4:
                    if(data_ch != '\n')
                    {
                        zero_status();
                    }else
                    {
                        status = REC_RESPONSE;
                        index = 0;
                    }
                    break;
                case REC_RESPONSE:
                    RESPONSE_DATA[RESPONSE_DATA_I++] = data_ch;
                    if(index >= RESPONSE_DATA_LEN)
                    {
                        END;
                    }
                    break;
                default:
                    ;
            }
        }else
        {
            //ณ๖ดํมห;
        }
    }
    
    return response;
    
#undef REC_VERSION     
#undef REC_STACODE 
#undef REC_CODEDEIS 
#undef REC_HEADNAME 
#undef REC_HEADVALUE 
#undef REC_RESPONSE 
#undef REC_TIMEOUT
}

httpResponse *post(uInt8 *url, httpHeader *header, uInt8 *data, uInt32 data_len)
{
    uInt32 send_len , calloc_len = 0;
    uInt8  *idatas = RT_NULL;
    uInt8  *istr_header = RT_NULL;
    Socket socket;
    static httpResponse response;
    
    const uInt8 GET[] = "GET ";
    const uInt8 VERSION[] = "HTTP/1.1\r\n";
    
    istr_header = header2str(header);
    calloc_len     = rt_strlen(url) + rt_strlen(iheader) + data_len + 30;
    idatas  = rt_calloc(calloc_len, sizeof(uInt8));
    
    if(idatas != RT_NULL)
    {
        strcat(idatas, GET);
        strcat(idatas, url);
        strcat(idatas, " ");
        strcat(idatas, VERSION);
        strcat(idatas, istr_header);
        strcat(idatas, "\r\n");
        
        send_len = rt_strlen(idatas) + 1;
        rt_memcpy(idatas + send_len, data, data_len);
        send_len += data_len;
        
        socket.connect(url, "TCP", 80);
        socket.write(&socket, idatas, send_len);
        response = http_get_response(&socket);
    }else
    {
        return RT_NULL;
    }
}

void head(uInt8 *url, httpHeader *header, uInt8 *data, uInt32 data_len)
{

}

void get(uInt8 *url, httpHeader *header, uInt8 *data, uInt32 data_len)
{

}

