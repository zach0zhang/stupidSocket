#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "stupidSocket.h"

typedef struct {
    int32_t fd;
    struct sockaddr_in server_addr;
} stupid_client_t;

stupid_client_t stupid_client = {0};

int32_t tcp_client_init(const char* server_ip, const int server_port)
{
    stupid_client.server_addr.sin_family = AF_INET;
    stupid_client.server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &stupid_client.server_addr.sin_addr) == 0) {
        _log("Invalid IP address\n");
        return BAD_PARAM;
    }

    stupid_client.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (stupid_client.fd < 0) {
        _log("Failed to Create client socket");
        return ERROR_COMMOND;
    }
    
    if (connect(stupid_client.fd, (struct sockaddr*)&stupid_client.server_addr, sizeof(stupid_client.server_addr))) {
        _log("%s", strerror(errno));
        return ERROR_COMMOND;
    }

    return stupid_client.fd;
}

int32_t udp_client_init(const char* server_ip, const int server_port)
{
    stupid_client.server_addr.sin_family = AF_INET;
    stupid_client.server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &stupid_client.server_addr.sin_addr) == 0) {
        _log("Invalid IP address\n");
        return BAD_PARAM;
    }

    stupid_client.fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (stupid_client.fd < 0) {
        _log("Failed to Create client socket");
        return ERROR_COMMOND;
    }

    return stupid_client.fd;
}

int32_t udp_client_send(int32_t fd, const void* buf, const int32_t len)
{
    int32_t ret;
    if (fd <= 0 || buf == NULL || len <= 0) {
        _log("%s: bad param", __func__);
        return BAD_PARAM;
    }

    ret = sendto(fd, buf, len, 0, (struct sockaddr*)&stupid_client.server_addr,sizeof(stupid_client.server_addr));
    return ret;
}

