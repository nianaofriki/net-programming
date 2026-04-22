// 文件传输服务器
//     功能：接收客户端发送的文件并保存  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 8889
#define BUFFER_SIZE 4096      // 每次接收数据块大小

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];
    int addrLen = sizeof(clientAddr);
    FILE *file;

    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock 初始化失败: %d\n", WSAGetLastError());
        return 1;
    }

    // 创建服务器套接字
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("创建套接字失败: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 设置服务器地址结构
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 监听所有接口
    serverAddr.sin_port = htons(PORT);

    // 绑定套接字
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("绑定失败: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // 监听连接
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        printf("监听失败: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    printf("服务器已启动，等待连接...\n");

    while (1) {
        // 接受客户端连接
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("接受连接失败: %d\n", WSAGetLastError());
            continue; // 继续接受下一个连接
        }
        printf("客户端已连接: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // 打开文件准备写入
        file = fopen("received_file.txt", "wb");
        if (file == NULL) {
            printf("无法打开文件进行写入\n");
            closesocket(clientSocket);
            continue;
        }
        while (1) { 
            memset(buffer, 0, BUFFER_SIZE);
            // 接收数据块
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesReceived > 0) {
                // 将接收到的数据写入文件
                fwrite(buffer, sizeof(char), bytesReceived, file);
            } else if (bytesReceived == 0) {
                printf("客户端已断开连接\n");
                break; // 客户端断开连接
            } else {
                printf("接收数据失败: %d\n", WSAGetLastError());
                break; // 接收数据出错
            }
        }
        // 关闭文件
        fclose(file);
        // 关闭客户端套接字
        closesocket(clientSocket);
    }

    // 关闭服务器套接字
    closesocket(serverSocket);
    // 清理 Winsock
    WSACleanup();
    return 0;
}