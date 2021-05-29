#include <string.h>
#include <stdio.h>
#include "stupidSocket.h"

// RPC ECHO Server
#define TCP_IP      "127.0.0.1"
#define TCP_PORT    2007
#define UDP_IP      "192.168.2.189"
#define UDP_PORT    2008

int main(int argc, char *argv[])
{
    int fd[10];
    char send_buf[256] = {0};
    char recv_buf[256] = {0};
    int ret;
    char addr[64] = {0};
    int port = 0;

    for (int i = 0; i < 5; i++)
        fd[i] = tcp_client_init(TCP_IP, TCP_PORT);

    fd[5] = udp_client_init(UDP_IP, UDP_PORT);

    
    while (42) {
        printf("\ninput send string: ");
        fgets(send_buf, sizeof(send_buf), stdin);

        send_buf[strcspn(send_buf, "\n")] = '\0';

        if (!strcmp(send_buf, "quit")) {
            printf("\ninput 'quit', then exit program\n");
            break;
        }

        if (strlen(send_buf) == 0)
            continue;

        for (int i = 0; i < 5; i++)
            tcp_client_send(fd[i], send_buf, strlen(send_buf));
        
        udp_client_send(fd[5], send_buf, strlen(send_buf));

        for (int i = 0; i < 5; i++) {
            ret = tcp_client_recv(fd[i], recv_buf, sizeof(recv_buf));
            printf("\n fd: %d, receive %d bytes: %s", fd[i], ret, recv_buf);
        }


        ret = udp_client_recv(fd[5], send_buf, sizeof(send_buf), addr, &port);
        printf("\n fd: %d, receive %d bytes from %s:%d: %s", fd[5], ret, addr, port, recv_buf);

        if (!strcmp(send_buf, "quit")) {
            printf("\ninput 'quit', then exit program\n");
            break;
        }
    }


    for (int i = 0; i < 5; i++)
        tcp_client_deinit(fd[i]);
    udp_client_deinit(fd[5]);
}