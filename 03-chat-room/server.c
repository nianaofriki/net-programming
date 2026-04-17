// 聊天室服务器
// 功能：
//     1. 监听端口，接受多个客户端连接
//     2. 每个客户端分配一个线程处理
//     3. 收到消息后广播给所有其他客户端
//     4. 客户端断开时从列表中移除

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>//提供进程、线程创建、程序退出、环境变量操作等系统级函数声明。
#include "protocol.h"
#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 50// 最大支持同时连接的客户端数

typedef struct {
    SOCKET sock;
    char name[32];  // 改为32字节，与protocol.h一致，存储真实昵称
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
CRITICAL_SECTION cs; // 互斥锁，保护客户端列表

// 广播给所有人（新增，用于系统消息）
void broadcastAll(Message *msg) {
    EnterCriticalSection(&cs);      // 加锁
    for (int i = 0; i < client_count; i++) {
        send(clients[i].sock, (char*)msg, sizeof(Message), 0);
    }
    LeaveCriticalSection(&cs);      // 解锁
}

// 向所有其他客户端广播消息（除了发送者自己）
void broadcast(SOCKET senderSock, Message *msg) {
    EnterCriticalSection(&cs);      // 加锁
    for (int i = 0; i < client_count; i++) {
        if (clients[i].sock != senderSock) {
            send(clients[i].sock, (char*)msg, sizeof(Message), 0);
        }
    }
    LeaveCriticalSection(&cs);      // 解锁
}

// 从客户端列表中移除一个客户端
void removeClient(SOCKET sock) {
    EnterCriticalSection(&cs);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].sock == sock) {
            // 将最后一个元素移到当前位置，覆盖要删除的
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    LeaveCriticalSection(&cs);
}

// 处理单个客户端的线程函数
unsigned int __stdcall clientThread(void *arg) {
    SOCKET clientSock = (SOCKET)arg;//arg 是传进来的 "客户端身份证"
    Message msg;  // 改为Message类型，不再用buffer
    int recvLen;
    char clientName[32] = "匿名";  // 保存客户端真实昵称

    // 第一步：接收登录消息（新增）
    recvLen = recv(clientSock, (char*)&msg, sizeof(Message), 0);
    if (recvLen <= 0 || msg.type != MSG_LOGIN) {
        printf("客户端登录失败或协议错误\n");
        closesocket(clientSock);
        return 0;
    }
    // 保存昵称到本地变量和客户端列表
    strncpy(clientName, msg.name, 31);
    clientName[31] = '\0';
    EnterCriticalSection(&cs);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].sock == clientSock) {
            strcpy(clients[i].name, clientName);
            break;
        }
    }
    LeaveCriticalSection(&cs);

    // 通知所有客户端有新用户加入（修改：使用真实昵称，广播给所有人包括自己）
    Message joinMsg;
    joinMsg.type = MSG_SYSTEM;
    strcpy(joinMsg.name, "系统");
    sprintf(joinMsg.content, "%s 加入了聊天室", clientName);  // 使用真实昵称
    broadcastAll(&joinMsg);  // 改为broadcastAll，让新用户也能看到

    // 接收客户端消息并广播
    while (1) {
        recvLen = recv(clientSock, (char*)&msg, sizeof(Message), 0);  // 直接收Message
        if (recvLen <= 0) {
            // 客户端断开连接
            break;
        }
        
        // 处理不同类型的消息（新增switch）
        switch (msg.type) {
            case MSG_CHAT:
                // 使用服务器保存的真实名字，防止客户端伪造
                strcpy(msg.name, clientName);
                broadcast(clientSock, &msg);  // 广播给其他人
                break;
            case MSG_QUIT:
                // 客户端主动退出
                goto client_exit;
            default:
                break;
        }
    }

client_exit:
    // 客户端断开，移除并通知其他人
    removeClient(clientSock);
    Message leaveMsg;
    leaveMsg.type = MSG_SYSTEM;
    strcpy(leaveMsg.name, "系统");
    sprintf(leaveMsg.content, "%s 离开了聊天室", clientName);  // 使用真实昵称
    broadcast(clientSock, &leaveMsg);  // 发给剩下的人（clientSock已移除，实际发给所有人）

    closesocket(clientSock);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET listenSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);

    // 初始化 Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);
    // 初始化临界区
    InitializeCriticalSection(&cs);

    // 创建监听 socket
    listenSock = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(listenSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSock, 5);

    printf("聊天室服务器启动，端口: %d\n", PORT);
    printf("等待客户端连接...\n");

    while (1) {
        clientSock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET) continue;

        // 加入客户端列表
        EnterCriticalSection(&cs);
        if (client_count < MAX_CLIENTS) {
            clients[client_count].sock = clientSock;
            strcpy(clients[client_count].name, "未登录");  // 初始状态
            client_count++;
        } else {
            // 服务器满，拒绝连接
            Message fullMsg;
            fullMsg.type = MSG_SYSTEM;
            strcpy(fullMsg.name, "系统");
            strcpy(fullMsg.content, "服务器已满");
            send(clientSock, (char*)&fullMsg, sizeof(Message), 0);
            closesocket(clientSock);
            LeaveCriticalSection(&cs);
            continue;
        }
        LeaveCriticalSection(&cs);

        // 创建线程处理该客户端
        _beginthreadex(NULL, 0, clientThread, (void*)clientSock, 0, NULL);
        printf("新客户端连接，当前在线人数: %d\n", client_count);
    }

    // 清理（本程序不会执行到这里，但保留）
    closesocket(listenSock);
    DeleteCriticalSection(&cs);
    WSACleanup();
    return 0;
}