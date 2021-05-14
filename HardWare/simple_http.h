#ifndef __SIMPLE_HTTP_H
#define __SIMPLE_HTTP_H

#include "my_type.h"
#include "simple_socket.h"

typedef struct _http_header
{
    uInt8 *Host;
    uInt8 *connection;
    uInt8 *content_type;
    uInt32 content_length;
}httpHeader;

typedef struct _http_response
{
    uInt8 *version_num;
    uInt32 status_code;
    uInt8 *message;
}httpResponse;

typedef struct _http_interface
{
    void (*get)(struct _http_interface *http, uInt8 *url, httpHeader *header, uInt8 *data, uInt32 data_len);
    void (*head)(struct _http_interface *http, uInt8 *url, httpHeader *header, uInt8 *data, uInt32 data_len);
    void (*post)(struct _http_interface *http, uInt8 *url, httpHeader *header, uInt8 *data, uInt32 data_len);
}httpInterface;

void init_http(httpInterface *ihttp, Socket);
uInt8 *header2str(httpHeader* header);

#endif
