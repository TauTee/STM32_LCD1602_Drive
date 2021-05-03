#include "esp8266_finsh.h"
#include "my_type.h"
#include <rtthread.h>
#include <stdlib.h>

void finsh_esp_test(void)
{
	esp_erro_status cmd_err;

    cmd_err = esp_test();
    if(cmd_err == ESP_SUC)
    {
        rt_kprintf("suc\n");
    }
}
MSH_CMD_EXPORT(finsh_esp_test, test);


void finsh_esp_rst(void)
{
	esp_erro_status cmd_err;

	cmd_err = esp_rst();
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_rst, rst);


void finsh_esp_get_ip(void)
{
	esp_erro_status cmd_err;

	cmd_err = esp_get_ip();
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_get_ip, get ip);


void finsh_esp_disconnect_wifi(void)
{
	esp_erro_status cmd_err;

	cmd_err = esp_disconnect_wifi();
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_disconnect_wifi, disconnect wifi);


void finsh_esp_stop_TCPUDP(void)
{
	esp_erro_status cmd_err;

	cmd_err = esp_stop_TCPUDP();
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_stop_TCPUDP, stop tcp/ip);


void finsh_esp_set_wifi_mode(int argc, char **argv)
{
    WIFIMODE mode;
	esp_erro_status cmd_err;

    if(argc < 2)
    {
        rt_kprintf("need more arg\n");
        return;
    }
    if(!rt_strcmp(argv[1], "station"))
    {
        mode = STATION_MODE;
    }else if(!rt_strcmp(argv[1], "ap"))
    {
        mode = AP_MODE;
    }else if(!rt_strcmp(argv[1], "ap_station"))
    {
        mode = AP_STATION_MODE;
    }
    
	cmd_err = esp_set_wifi_mode(mode);
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_set_wifi_mode, set wifi);


void finsh_esp_connect_wifi(int argc, char **argv)
{
    unsigned char *issd;
    unsigned char *password;
	esp_erro_status cmd_err;
    
    if(argc < 3)
    {
        rt_kprintf("need more arg\n");
        return;
    }
    issd = argv[1];
    password = argv[2];
	cmd_err = esp_connect_wifi(issd, password);
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_connect_wifi, connect wifi);


void finsh_esp_set_ip(int argc, char **argv)
{
    unsigned char *ip;
	esp_erro_status cmd_err;


    if(argc < 2)
    {
        rt_kprintf("need more arg\n");
        return;
    }
    ip = argv[1];
	cmd_err = esp_set_ip(ip);
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_set_ip, set ip);


void finsh_esp_build_TCPUDP(int argc, char **argv)
{
    unsigned char *ip_domain;
    unsigned char *mode;
    unsigned int  port;
	esp_erro_status cmd_err;

    if(argc < 4)
    {
        rt_kprintf("need more arg\n");
        return;
    }
    ip_domain = argv[1];
    mode = argv[2];
    port = atoi(argv[3]);
	cmd_err = esp_build_TCPUDP(ip_domain, mode, port);
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_build_TCPUDP, build tcp ip);


void finsh_esp_set_tcpmode(int argc, char **argv)
{
    TCPMODE mode;
	esp_erro_status cmd_err;

    if(argc < 2)
    {
        rt_kprintf("need more arg\n");
        return;
    }
    if(!rt_strcmp(argv[1], "initiative"))
    {
        mode = INITIATIVE_MODE;
    }else if(!rt_strcmp(argv[1], "passivity"))
    {
        mode = PASSIVITY_MODE;
    }
    
	cmd_err = esp_set_tcpmode(mode);
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_set_tcpmode, set tcp mode);


void finsh_esp_ipsend(int argc, char **argv)
{
    unsigned char *data;
    unsigned int  length;
	esp_erro_status cmd_err;

    if(argc < 3)
    {
        rt_kprintf("need more arg\n");
        return;
    }
    data = argv[1];
    length = atoi(argv[2]);
	cmd_err = esp_ipsend(data, length);
	if(cmd_err ==ESP_SUC)
	{
		rt_kprintf("suc\n");
	}else
	{
		rt_kprintf("err\n");
	}
}
MSH_CMD_EXPORT(finsh_esp_ipsend, ip send);


