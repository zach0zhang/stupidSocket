#ifndef __STUPID_SOCKET_H__
#define __STUPID_SOCKET_H__

#include <stdint.h>

#define USE_OUT_LOG_MODULE  0
#if USE_OUT_LOG_MODULE
    // implement _log
#else
#include <stdio.h>
#define _log(fmt, ...) do {   \
    printf(fmt, ##__VA_ARGS__);     \
    } while(0)
#endif

enum {
    NO_ERROR            = 0,
    ERROR_COMMOND       = 0x80000001,
    BAD_PARAM
};

int32_t tcp_client_init(const char* server_ip, const int server_port);
int32_t udp_client_init(const char* server_ip, const int server_port);
void tcp_client_deinit(int32_t fd);
void udp_client_deinit(int32_t fd);
int32_t tcp_client_send(int32_t fd, const void* buf, const int32_t len);
int32_t udp_client_send(int32_t fd, const void* buf, const int32_t len);
int32_t tcp_client_recv(int32_t fd, void* const buf, const int32_t len);
int32_t udp_client_recv(int32_t fd, void* buf, const int32_t len, int8_t* const from_addr, int32_t* const from_port);

#endif