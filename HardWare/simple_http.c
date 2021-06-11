#include <string.h>
#include <rtthread.h>
#include "my_type.h"
#include "simple_http.h"
#include "simple_socket.h"

void raise_err(uInt8 *err_information)
{

}

uInt8 *header2str(httpHeader *header)
{
    
}

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
    #define REC_RESPONSE 9
    #define REC_TIMEOUT -1

    const uInt32 MAX_WAIT_TIME = 1000;
    httpResponse response;
    uInt8 data_ch;
    sInt32 status = REC_VERSION;
    uInt32 index = 0;
    uInt8 head_word_name[10][15];
    uInt8 head_word_value[10][15];
    uInt8 response_data[1024];
    uInt32 head_word_index = 0;
    uInt32 begain_time   = rt_tick_get();
    

    
    while ((rt_tick_get() - begain_time < MAX_WAIT_TIME))
    {
        if(socket->read(socket, &data_ch, 1) > 0)
        {
            switch(status)
            {
                case REC_VERSION:
                    if(data_ch != ' ')
                    {
                        response.version_num[index++] = data_ch;
                        if(index > 10)
                        {
                            raise_err("ver_num too long");
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
                        if(index > 4)
                        {
                            raise_err("stacode too long");
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
                        raise_err("to head word err");
                    }else
                    {
                        status = REC_CODEDDIS_N;
                        index  = 0;
                        head_word_index = 0;
                    }
                    break;
                case REC_CODEDDIS_N:
                    if(data_ch != ':')
                    {
                        head_word_name[head_word_index][index++] = data_ch;
                        if(index > 8)
                        {
                            raise_err("head word name too long");
                        }
                    }else
                    {
                        head_word_name[head_word_index][index] = '\0';
                        status = REC_HEADWORD_1;
                        index  = 0;
                    }
                    break;
                case REC_HEADWORD_1:
                    if(data_ch != '\r')
                    {
                        head_word_value[head_word_index][index++] = data_ch;
                        if(index > 13)
                        {
                            raise_err("head word value too long");
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
                        raise_err("head word \\n lost");
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
                        head_word_index++;
                        head_word_name[head_word_index][index++] = data_ch;
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
                        raise_err("to response err");
                    }else
                    {
                        status = REC_RESPONSE;
                        index = 0;
                    }
                    break;
                case REC_RESPONSE:
                    response_data[index++] = data_ch;
                    if(index >= 1023)
                    {
                        raise_err("response too long");
                    }
                    break;
                default:
                    raise_err("sta err");
            }
        }else
        {
            //ณ๖ดํมห;
        }
    }
    #undef REC_VERSION     
    #undef REC_STACODE 
    #undef REC_CODEDEIS 
    #undef REC_HEADNAME 
    #undef REC_HEADVALUE 
    #undef REC_RESPONSE 
    #undef REC_TIMEOUT
    
    return response;
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
    calloc_len     = rt_strlen(url) + rt_strlen(istr_header) + data_len + 30;
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

