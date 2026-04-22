// 文件传输客户端
// 功能：发送指定文件到服务器
// 通信协议：
// 1. 先发送文件名
// 2. 再发送文件大小
// 3. 最后循环发送文件内容  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8889
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 4096

// 发送文件到服务器
int sendFile(SOCKET sock, const char *filepath) {
    FILE *fp;
    long fileSize;
    char filename[256];
    char buffer[BUFFER_SIZE];
    long totalSent = 0;
    int readLen;

    // 1. 打开文件
    fp = fopen(filepath, "rb");
    if (fp == NULL) {
        perror("无法打开文件");
        return -1;
    }
    // 2. 获取文件名（不包含路径）
    const char *baseName = strrchr(filepath, '\\');
    if (baseName == NULL) {
        baseName = filepath; // 没有路径，直接使用文件名
    } else {
        baseName++; // 跳过路径分隔符
    }
    strcpy(filename, baseName);
    // 3. 获取文件大小
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    printf("文件名: %s\n", filename);
    printf("文件大小: %ld 字节 (%.2f KB)\n", fileSize, fileSize / 1024.0);
    // 4. 发送文件名
    if (send(sock, filename, strlen(filename), 0) == -1) {
        perror("发送文件名失败");
        fclose(fp);
        return -1;
    }
    send(sock, filename, strlen(filename), 0);
    // 5. 发送文件大小
    send(sock, (char*)&fileSize, sizeof(long), 0);
    // 6. 循环发送文件内容
    while (totalSent < fileSize) {
        readLen = fread(buffer, 1, BUFFER_SIZE, fp);
        if (readLen <= 0) break;
        
        send(sock, buffer, readLen, 0);
        totalSent += readLen;
        
        // 打印进度
        printf("\r发送进度: %.1f%%", (float)totalSent / fileSize * 100);
    }
    fclose(fp);
    printf("\n文件发送完成\n");
    return 1;
}
int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serverAddr;
    char filepath[256];
    char ack[10];
    int result;

    // 初始化 Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // 创建 socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("创建socket失败\n");
        WSACleanup();
        return -1;
    }

    // 配置服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    // 连接服务器
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("连接服务器失败\n");
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    printf("已连接到文件服务器\n");

    // 输入要发送的文件路径
    printf("请输入要发送的文件路径: ");
    fgets(filepath, sizeof(filepath), stdin);
    filepath[strcspn(filepath, "\n")] = 0;  // 去掉换行符

    // 发送文件
    result = sendFile(sock, filepath);

    // 接收服务器确认
    recv(sock, ack, sizeof(ack), 0);
    printf("服务器确认: %s\n", ack);

    // 关闭连接
    closesocket(sock);
    WSACleanup();
    
    printf("按回车键退出...");
    getchar();
    return 0;
}