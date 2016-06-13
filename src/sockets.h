#ifndef _SOCKETS_H_
#define _SOCKETS_H_
#include <asm-generic/sockios.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

int jrpc_tcp_passive_open(unsigned short local_port, int backlog);
int jrpc_build_sockaddr(const char *host, unsigned short port, struct sockaddr_storage* dest);
extern int jrpc_tcp_active_open(struct sockaddr_storage* dest, const char *device, struct timeval *timeout);
int jrpc_read_from_socket(int sockfd, char *buffer, int size);
#endif //_SOCKETS_H_
