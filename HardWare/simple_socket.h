#ifndef __SIMPLE_SOCKET_H
#define __SIMPLE_SOCKET_H

#include "my_type.h"

typedef struct _socket
{
    errorStatus (*connect)(uInt8 *ip_domain, uInt8 *imode, uInt32 port);
    errorStatus (*close)(void);
    uInt32 (*read)(struct _socket *this_socket, uInt8 *datas, uInt32 len);
    uInt32 (*write)(struct _socket *this_socket, uInt8 *datas, uInt32 len);
}Socket;

#endif
