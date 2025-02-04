#ifndef __ESP8266_AT_H__
#define __ESP8266_AT_H__

#define MAX_WAIT_TIME 2000
#define WAIT_REST_TIME 10

typedef enum 
{
    ESP_SUC = 0,
    ESP_ERROR = 1,
    ESP_TIMEOUT = 2
}esp_erro_status;

typedef enum
{
    STATION_MODE = '1',
    AP_MODE = '2',
    AP_STATION_MODE = '3'
}WIFIMODE;

typedef enum
{
    INITIATIVE_MODE = '0', 
    PASSIVITY_MODE = '1'
}TCPMODE;

/*����ӿ�*/
void init_esp_hardware(void);

esp_erro_status esp_test(void);
esp_erro_status esp_rst(void);
esp_erro_status esp_set_wifi_mode(WIFIMODE imode);
esp_erro_status esp_connect_wifi(unsigned char *issid, unsigned char *ipassword);
esp_erro_status esp_disconnect_wifi(void);
esp_erro_status esp_get_ip(void);
esp_erro_status esp_set_ip(unsigned char *iipaddress);
esp_erro_status esp_build_TCPUDP(unsigned char *ip_domain, unsigned char *imode, unsigned int port);
esp_erro_status esp_set_tcpmode(TCPMODE tcp_mode);
esp_erro_status esp_ipsend(unsigned char *idata, unsigned int length);
esp_erro_status esp_stop_TCPUDP(void);

#endif
