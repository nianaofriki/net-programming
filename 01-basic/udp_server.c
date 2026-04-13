// UDP 服务器功能：启动后接收客户端发来的消息，并回复
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define PORT 8889
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET server_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE]={0};
    char response[] = "UDP 消息已收到！";
    // 创建 UDP socket
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);// 区别1：SOCK_DGRAM 而不是 SOCK_STREAM，SOCK_DGRAM：数据报套接字（UDP）
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("UDP 服务器已启动，等待客户端消息...\n");
    printf("端口: %d\n", PORT);
    // 接收数据
    recvfrom(server_fd, buffer, BUFFER_SIZE, 0, //区别2：UDP 使用 recvfrom，不需要 accept,recvfrom：接收数据的同时，能拿到发送方的地址
             (struct sockaddr*)&client_addr, &addr_len);
    printf("收到消息: %s\n", buffer);
    // 发送回复
    sendto(server_fd, response, strlen(response), 0, //区别3：UDP 使用 sendto，不需要 connect,sendto：发送数据的同时，指定目标地址
           (struct sockaddr*)&client_addr, addr_len);
    // 关闭 socket
    closesocket(server_fd);
    WSACleanup();
    return 0;

}