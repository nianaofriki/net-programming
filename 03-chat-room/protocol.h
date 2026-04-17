// 聊天室通信协议
//     定义客户端和服务器之间消息的格式

#ifndef PROTOCOL_H
#define PROTOCOL_H

// 消息类型
#define MSG_LOGIN      1   // 登录消息（客户端发送昵称）
#define MSG_CHAT       2   // 聊天消息（普通对话）
#define MSG_QUIT       3   // 退出消息（客户端主动退出）
#define MSG_SYSTEM     4   // 系统消息（服务器广播，如有人加入/离开）

// 消息结构体（固定格式，便于解析）
typedef struct {
    int type;               // 消息类型：1登录 2聊天 3退出 4系统
    char name[32];          // 发送者昵称
    char content[512];      // 消息内容
} Message;

#endif