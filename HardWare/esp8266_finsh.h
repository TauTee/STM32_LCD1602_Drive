#ifndef __ESP8266_FINSH_H
#define __ESP8266_FINSH_H

#include "my_type.h"
#include "esp8266_at.h"

void finsh_esp_test(void);
void finsh_esp_rst(void);
void finsh_esp_get_ip(void);
void finsh_esp_disconnect_wifi(void);
void finsh_esp_stop_TCPUDP(void);
void finsh_esp_set_wifi_mode(int argc, char **argv);
void finsh_esp_connect_wifi(int argc, char **argv);
void finsh_esp_set_ip(int argc, char **argv);
void finsh_esp_build_TCPUDP(int argc, char **argv);
void finsh_esp_set_tcpmode(int argc, char **argv);
void finsh_esp_ipsend(int argc, char **argv);

#endif
