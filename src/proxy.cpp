#include "jrpc.h"
#include "sockets.h"
#include "message.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <json/json.h>

namespace MRPC {
    Proxy::Proxy(std::string host) {

        /*((struct sockaddr_in*)&output->remote)->sin_family = AF_INET;
        ((struct sockaddr_in*)&output->remote)->sin_port = htons(port);
        inet_pton(AF_INET, host, &(((struct sockaddr_in*)&output->remote)->sin_addr));
        output->sockfd = jrpc_tcp_active_open(&output->remote, NULL, NULL);
        return output;*/
    }
}
/*int jrpc_proxy_close(struct jrpc_proxy *proxy)
{
    if(proxy->sockfd)
        close(proxy->sockfd);
    return 0;
}
int jrpc_proxy_call(struct jrpc_proxy *proxy, const char *method_name, Json::Value args)
{
    return -1;
}*/