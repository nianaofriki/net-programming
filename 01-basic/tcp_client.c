// TCP 客户端功能：连接服务器，发送消息，接收回复  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>// Windows Socket：网络编程的核心头文件
#pragma comment(lib, "ws2_32.lib")
#define PORT 8888
#define BUFFER_SIZE 1024
int main()
{ 
    WSADATA wsa;
    SOCKET sock_fd;
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE]={0};
    // 1. 初始化 Winsock 库
    WSAStartup(MAKEWORD(2, 2), &wsa);
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);//和服务器一样：AF_INET + SOCK_STREAM = TCP
    // 2. 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");// inet_addr() 函数：将点分十进制的 IP 地址转换为网络字节序的二进制形式,"127.0.0.1" 是本地回环地址（本机）
    server_addr.sin_port = htons(PORT);// htons() 函数：将主机字节序转换为网络字节序
    // 3. 连接服务器
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("连接失败！\n");
        return -1;
    }
    printf("已连接到服务器！\n");
    // 4. 获取用户输入
    printf("请输入要发送的消息：");
    fgets(message, BUFFER_SIZE, stdin);// fgets：从键盘读取一行，包括空格，strcspn()：找到第一个换行符的位置
    message[strcspn(message, "\n")] = 0;// 将换行符替换为字符串结束符
    // 5. 发送消息
    send(sock_fd, message, strlen(message), 0);
    printf("已发送: %s\n", message);
    // 6. 接收回复
    recv(sock_fd, buffer, BUFFER_SIZE, 0);
    printf("服务器回复：%s\n", buffer);
    // 7. 关闭连接
    closesocket(sock_fd);
    WSACleanup();
    return 0;

}
