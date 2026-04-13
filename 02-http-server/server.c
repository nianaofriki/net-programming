// 简易 HTTP 静态文件服务器
// 功能：
// 1. 监听端口 8080
// 2. 解析 GET 请求的路径
// 3. 返回对应文件（如果存在）或 404
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096
#define WEB_ROOT "./www"   // 网站根目录
//  获取文件扩展名对应的 MIME 类型
const char* get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');  // 找到最后一个点
    if (!ext) return "text/plain";
    
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    return "text/plain";
}
// 发送 HTTP 响应
void send_response(SOCKET client, int status_code, const char *status_text, //..._code是状态码
                   const char *content_type, const char *body, int body_len) {//body是响应体（实际内容数据）的指针
    char header[BUFFER_SIZE];
    // 构造响应头
    sprintf(header, 
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n",
            status_code, status_text, content_type, body_len);
    
    send(client, header, strlen(header), 0);
    send(client, body, body_len, 0);
}
// 发送文件内容
void send_file(SOCKET client, const char *filepath, const char *content_type)  { 
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return;
    
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    
    // 分配缓冲区并读取文件
    char *buffer = (char*)malloc(file_size);
    fread(buffer, 1, file_size, fp);
    fclose(fp);
    
    // 发送响应
    char header[BUFFER_SIZE];
    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n",
            content_type, file_size);
    send(client, header, strlen(header), 0);
    send(client, buffer, file_size, 0);
    
    free(buffer);
}
// 发送 404 错误页面
void send_404(SOCKET client) {
    // 定义 HTML 响应体（多行字符串）
    const char *body = "<!DOCTYPE html><html><head><title>404 Not Found</title></head>"
                       "<body><h1>404 - 文件未找到</h1><p>请求的资源不存在。</p></body></html>";
    // 调用 send_response 发送响应
    send_response(client, 404, "Not Found", "text/html", body, strlen(body));
}
// 处理 HTTP 请求
void handle_request(SOCKET client, const char *request) {
    // 解析请求行
    char method[16], path[256], version[16];
    sscanf(request, "%s %s %s", method, path, version);// sscanf：从字符串中按格式提取数据

    // 检查是否为 GET 请求
    if (strcmp(method, "GET") != 0) {
       const char *body405 = "<html><body><h1>405 - Method Not Allowed</h1></body></html>";
        send_response(client, 405, "Method Not Allowed", "text/html", body405, strlen(body405));
        return;
    }
    // 如果请求根路径 "/"，默认返回 /index.html
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }

    // 构造文件路径
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s", WEB_ROOT, path);

    for (char *p = filepath; *p; p++) {
        if (*p == '/') *p = '\\';
    }// Windows 文件系统使用反斜杠 \，将 URL 中的正斜杠 / 替换为 \
    // 检查文件是否存在
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        send_404(client);
        return;
    }
    fclose(fp);

    // 获取文件扩展名并确定 MIME 类型
    const char *content_type = get_mime_type(filepath);

    // 发送文件内容
    send_file(client, filepath, content_type);
}
int main() {
    // 初始化 Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 创建套接字
    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 绑定套接字
    bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server, 5);
    printf("服务器已启动，监听端口 %d...\n", PORT);

    while (1) {
        // 接受客户端连接
        SOCKET client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) continue;

        // 接收请求
        char buffer[BUFFER_SIZE];
        int recv_len = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (recv_len > 0) {
            buffer[recv_len] = '\0'; // 确保字符串以 null 结尾
            handle_request(client, buffer);
        }
        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}