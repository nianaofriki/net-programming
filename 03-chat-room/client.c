// 聊天室客户端
// 功能：
//     1. 连接服务器
//     2. 一个线程接收服务器广播的消息并显示
//     3. 主线程等待用户输入并发送消息

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>//创建线程

#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024

SOCKET sock;
int g_running = 1;

//接收线程：不断接收服务器消息并打印
unsigned int __stdcall receiveThread(void *arg) {
    Message msg;  // 改为Message类型
    int recvLen;

    while (g_running) {
        recvLen = recv(sock, (char*)&msg, sizeof(Message), 0);  // 直接收Message
        if (recvLen <= 0) {
            printf("[系统] 与服务器断开连接\n");
            g_running = 0;
            break;
        }

        // 根据消息类型显示（保留原switch结构）
        switch (msg.type) {
            case MSG_CHAT:
                printf("[%s] %s\n", msg.name, msg.content);
                break;
            case MSG_SYSTEM:
                printf("[系统] %s\n", msg.content);
                break;
        }
    }
    return 0;
}

int main() {
    WSADATA wsa;
    struct sockaddr_in serverAddr;
    char message[BUFFER_SIZE];

    // 初始化 Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // 创建 socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    // 连接服务器
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("连接服务器失败\n");
        return -1;
    }
    printf("已连接到聊天室！\n");

    // 输入昵称并发送登录消息（新增）
    char nickname[32];
    printf("请输入你的昵称: ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = 0;

    // 发送登录消息（新增）
    Message loginMsg;
    loginMsg.type = MSG_LOGIN;
    strcpy(loginMsg.name, nickname);
    strcpy(loginMsg.content, "");
    send(sock, (char*)&loginMsg, sizeof(Message), 0);

    printf("输入消息（输入 /quit 退出）\n");

    // 启动接收线程
    _beginthreadex(NULL, 0, receiveThread, NULL, 0, NULL);

    // 主线程负责发送消息
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        // 去掉换行符
        message[strcspn(message, "\n")] = 0;

        if (strcmp(message, "/quit") == 0) {
            // 发送退出消息（新增）
            Message quitMsg;
            quitMsg.type = MSG_QUIT;
            strcpy(quitMsg.name, nickname);
            strcpy(quitMsg.content, "");
            send(sock, (char*)&quitMsg, sizeof(Message), 0);
            break;
        }
        
        // 发送消息给服务器（修改：使用Message结构体）
        Message chatMsg;
        chatMsg.type = MSG_CHAT;
        strcpy(chatMsg.name, nickname);  // 客户端发送自己的昵称（服务器会校验）
        strcpy(chatMsg.content, message);
        send(sock, (char*)&chatMsg, sizeof(Message), 0);
    }

    g_running = 0;  // 通知接收线程退出
    closesocket(sock);
    WSACleanup();
    return 0;
}