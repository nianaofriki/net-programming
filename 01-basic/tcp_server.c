  
// TCP 服务器功能：启动后等待客户端连接，接收消息并回复
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>// Windows Socket：网络编程的核心头文件

#pragma comment(lib, "ws2_32.lib")// 链接 Winsock 库
#define PORT 8888
#define BUFFER_SIZE 1024// 缓冲区大小
int main()
{
    // 1. Windows Socket 相关变量定义
    WSADATA wsa; //WSADATA 是一个 Windows 定义好的结构体（数据结构）
    SOCKET server_fd;//创建服务端套接字，用于监听客户端连接
    SOCKET client_fd;
    struct sockaddr_in server_addr;// sockaddr_in 是一个结构体，包含了网络地址信息
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE]={0};//接收缓冲区，全初始化为0
    char response[] = "消息已收到！";//回复消息
    // 2. 初始化 Winsock 库
    WSAStartup(MAKEWORD(2, 2), &wsa) != 0;// MAKEWORD(2, 2) 表示 Winsock 版本号,&wsa 是传出参数，WSAStartup 会把信息填进去
    // 3. 创建 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);//socket() 函数：创建一个通信端点,创建 TCP 套接字，AF_INET 表示 IPv4，SOCK_STREAM 表示 TCP,0：自动选择协议（TCP）,socket 句柄（失败返回 INVALID_SOCKET）
    // 4. 配置服务器地址
    server_addr.sin_family = AF_INET;//sin_family：地址族，AF_INET 表示 IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;//sin_addr.s_addr：IP 地址，INADDR_ANY 表示监听所有可用接口
    server_addr.sin_port = htons(PORT);//sin_port：端口号，htons() 函数将主机字节序转换为网络字节序
    // 5. 绑定 socket 到地址
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));// bind() 函数：将套接字绑定到指定的地址和端口
    // 6. 开始监听
    listen(server_fd, 3);// listen() 函数：将套接字设置为监听状态，参数 3 是连接队列的最大长度
    printf("TCP 服务器已启动，等待客户端连接...\n");
    printf("端口: %d\n", PORT);
    //  7. 接受客户端连接
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);// accept() 函数：接受一个连接请求，返回一个新的套接字用于通信
    printf("客户端连接成功！\n");
    // 8. 接收消息并回复
    recv(client_fd, buffer, BUFFER_SIZE, 0);// recv() 函数：从套接字接收数据，参数 0 表示默认行为
    printf("客户端说: %s\n", buffer);
    // 9. 发送回复消息
    send(client_fd, response, strlen(response), 0);// send() 函数：向套接字发送数据，参数 0 表示默认行为
    printf("已回复: %s\n", response);
    // 10. 关闭套接字和清理 Winsock
    closesocket(client_fd);
    closesocket(server_fd);
    WSACleanup(); // 释放 Windows Socket 库占用的资源
    return 0;

}