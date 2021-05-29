#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "stupidSocket.h"

struct stupid_client_node{
    int32_t fd;
    struct sockaddr_in server_addr;
    struct stupid_client_node *next;
};

typedef struct stupid_client_node stupid_client_t;

stupid_client_t stupid_client_head = {0};
pthread_rwlock_t stupid_client_rwlock = PTHREAD_RWLOCK_INITIALIZER;

static inline void add_stupid_client_to_list(stupid_client_t *cli)
{
    pthread_rwlock_wrlock(&stupid_client_rwlock);
    stupid_client_t *p = &stupid_client_head;
    while (p->next != NULL)
        p = p->next;
    p->next = cli;
    pthread_rwlock_unlock(&stupid_client_rwlock);
}

static inline void remove_stupid_client_from_list(stupid_client_t* prev, stupid_client_t *cli)
{
    prev->next = cli->next;
}

static inline void remove_stupid_client(int32_t fd)
{
    pthread_rwlock_wrlock(&stupid_client_rwlock);
    stupid_client_t *p, *prev;
    p = prev = &stupid_client_head;
    while (p->next != NULL) {
        if (p->fd == fd) {
            remove_stupid_client_from_list(prev, p);
            free(p);
            break;
        }
        prev = p;
        p = p->next;
    }
    pthread_rwlock_unlock(&stupid_client_rwlock);
}

static inline stupid_client_t* find_stupid_client(int32_t fd)
{
    pthread_rwlock_rdlock(&stupid_client_rwlock);
    stupid_client_t *p = &stupid_client_head;
    while (p != NULL) {
        if (p->fd == fd)
            break;
        p = p->next;
    }
    pthread_rwlock_unlock(&stupid_client_rwlock);
    return p;
}

int32_t tcp_client_init(const char* server_ip, const int server_port)
{
    stupid_client_t *stupid_client = (stupid_client_t *)calloc(1, sizeof(stupid_client_t));
    if (stupid_client == NULL) {
        _log("%s: calloc error\n", __func__);
        return ERROR_COMMOND;
    }
    stupid_client->server_addr.sin_family = AF_INET;
    stupid_client->server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &stupid_client->server_addr.sin_addr) == 0) {
        _log("Invalid IP address\n");
        return BAD_PARAM;
    }

    stupid_client->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (stupid_client->fd < 0) {
        _log("Failed to Create client socket\n");
        return ERROR_COMMOND;
    }
    
    if (connect(stupid_client->fd, (struct sockaddr*)&stupid_client->server_addr, sizeof(stupid_client->server_addr))) {
        close(stupid_client->fd);
        _log("%s\n", strerror(errno));
        return ERROR_COMMOND;
    }

    add_stupid_client_to_list(stupid_client);
    return stupid_client->fd;
}

int32_t udp_client_init(const char* server_ip, const int server_port)
{
    stupid_client_t *stupid_client = (stupid_client_t *)calloc(1, sizeof(stupid_client_t));
    if (stupid_client == NULL) {
        _log("%s: calloc error\n", __func__);
        return ERROR_COMMOND;
    }

    stupid_client->server_addr.sin_family = AF_INET;
    stupid_client->server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &stupid_client->server_addr.sin_addr) == 0) {
        _log("Invalid IP address\n");
        return BAD_PARAM;
    }

    stupid_client->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (stupid_client->fd < 0) {
        _log("Failed to Create client socket\n");
        return ERROR_COMMOND;
    }

    add_stupid_client_to_list(stupid_client);
    return stupid_client->fd;
}

static inline void client_close(int32_t fd)
{
    close(fd);
}

void tcp_client_deinit(int32_t fd)
{
    client_close(fd);
    remove_stupid_client(fd);
}

void udp_client_deinit(int32_t fd)
{
    client_close(fd);
    remove_stupid_client(fd);
}

int32_t tcp_client_send(int32_t fd, const void* buf, const int32_t len)
{
    int32_t ret;
    if (fd <= 0 || buf == NULL || len <= 0) {
        _log("%s: bad param\n", __func__);
        return BAD_PARAM;
    }

    ret = send(fd, buf, len, 0);
    return ret;
}

int32_t tcp_client_recv(int32_t fd, void* const buf, const int32_t len)
{
    int32_t ret;
    if (fd <= 0 || buf == NULL || len <= 0) {
        _log("%s: bad param\n", __func__);
        return BAD_PARAM;
    }   

    ret = recv(fd, buf, len, 0);
    return ret;
}

int32_t udp_client_send(int32_t fd, const void* buf, const int32_t len)
{
    int32_t ret;
    stupid_client_t *stupid_client;
    if (fd <= 0 || buf == NULL || len <= 0) {
        _log("%s: bad param\n", __func__);
        return BAD_PARAM;
    }
    
    stupid_client = find_stupid_client(fd);
    if (stupid_client == NULL) {
        _log("%s: fd is not exist\n", __func__);
        return ERROR_COMMOND;
    }
    ret = sendto(fd, buf, len, 0, (struct sockaddr*)&stupid_client->server_addr,sizeof(stupid_client->server_addr));
    return ret;
}

int32_t udp_client_recv(int32_t fd, void* const buf, const int32_t len, int8_t* const from_addr, int32_t* const from_port)
{
    int32_t ret;
    stupid_client_t *stupid_client;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if (fd <= 0 || buf == NULL || len <= 0 || from_addr == NULL || from_port== NULL) {
        _log("%s: bad param\n", __func__);
        return BAD_PARAM;
    }

    stupid_client = find_stupid_client(fd);
    if (stupid_client == NULL) {
        _log("%s: fd is not exist\n", __func__);
        return ERROR_COMMOND;
    }

    
    ret = recvfrom(fd, buf, len, 0, (struct sockaddr*)&addr, &addr_len);
    sprintf(from_addr, "%s", inet_ntoa(addr.sin_addr));
    *from_port = ntohs(addr.sin_port);
    return ret;

}

