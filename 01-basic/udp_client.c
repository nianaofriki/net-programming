 #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8889
#define BUFFER_SIZE 1024
int main()
{
    WSADATA wsa;
    SOCKET sock_fd;
    struct sockaddr_in server_addr;
    int addr_len = sizeof(server_addr);
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};
    WSAStartup(MAKEWORD(2, 2), &wsa);
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    printf("UDP客户端启动\n");
    printf("请输入消息: ");
    fgets(message, BUFFER_SIZE, stdin);//stdin：标准输入（键盘）
    message[strcspn(message, "\n")] = 0;
    // 发送数据（不需要 connect）
    sendto(sock_fd, message, strlen(message), 0, (struct sockaddr *)&server_addr, addr_len);
    printf("已发送: %s\n", message);
    // 接收回复（不需要 accept）
    recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
    printf("收到回复: %s\n", buffer);
    closesocket(sock_fd);
    WSACleanup();
    return 0;
} 
