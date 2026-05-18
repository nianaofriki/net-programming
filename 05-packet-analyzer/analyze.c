//   数据包协议解析模块
// 功能：解析以太网、IP、TCP、UDP 协议头部
// 被 sniffer.c 调用，也可以独立测试
#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

// 协议头结构体定义

// 以太网帧头部
typedef struct {
    unsigned char dest_mac[6];   // 目的MAC地址
    unsigned char src_mac[6];    // 源MAC地址
    unsigned short type;         // 上层协议类型（0x0800=IP, 0x0806=ARP）
} ethernet_header;

// IP头部
typedef struct {
    unsigned char ver_ihl;       // 版本(4bit) + 头部长度(4bit)
    unsigned char tos;           // 服务类型
    unsigned short total_len;    // 总长度
    unsigned short id;           // 标识
    unsigned short frag_off;     // 标志 + 片偏移
    unsigned char ttl;           // 生存时间
    unsigned char protocol;      // 上层协议（6=TCP, 17=UDP, 1=ICMP）
    unsigned short checksum;     // 首部校验和
    unsigned int src_ip;         // 源IP地址
    unsigned int dest_ip;        // 目的IP地址
} ip_header;

// TCP头部
typedef struct {
    unsigned short src_port;     // 源端口
    unsigned short dest_port;    // 目的端口
    unsigned int seq;            // 序号
    unsigned int ack;            // 确认号
    unsigned char offset_res;    // 数据偏移(4bit) + 保留(3bit) + 标志(1bit)
    unsigned char flags;         // 标志位(8bit)
    unsigned short window;       // 窗口大小
    unsigned short checksum;     // 校验和
    unsigned short urgent;       // 紧急指针
} tcp_header;

// UDP头部
typedef struct {
    unsigned short src_port;     // 源端口
    unsigned short dest_port;    // 目的端口
    unsigned short len;          // 数据长度
    unsigned short checksum;     // 校验和
} udp_header;

// 辅助函数
// 打印 MAC 地址
void print_mac(const unsigned char *mac) {
    printf("%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
// 打印 IP 地址（将整数转为点分十进制）
void print_ip(unsigned int ip) {
    printf("%d.%d.%d.%d", 
           ip & 0xFF, 
           (ip >> 8) & 0xFF,
           (ip >> 16) & 0xFF, 
           (ip >> 24) & 0xFF);
}
// 打印 TCP 标志位
void print_tcp_flags(unsigned char flags) {
    if (flags & 0x01) printf("FIN ");//&	按位与运算符，用于检查特定位是否被设置
    if (flags & 0x02) printf("SYN ");
    if (flags & 0x04) printf("RST ");
    if (flags & 0x08) printf("PSH ");
    if (flags & 0x10) printf("ACK ");
    if (flags & 0x20) printf("URG ");
    if (flags & 0x40) printf("ECE ");
    if (flags & 0x80) printf("CWR ");
}

// 主解析函数
void analyze_packet(const unsigned char *packet) {
    ethernet_header *eth;
    ip_header *ip;
    tcp_header *tcp;
    udp_header *udp;
    int ip_header_len;
    // 1. 解析以太网帧
    eth = (ethernet_header *)packet;
    eth = (ethernet_header*)packet;
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ 以太网帧                                │\n");
    printf("├─────────────────────────────────────────┤\n");
    printf("│ 目的MAC: "); print_mac(eth->dest_mac); printf("\n");
    printf("│ 源MAC:   "); print_mac(eth->src_mac); printf("\n");
    printf("│ 类型:    0x%04X", ntohs(eth->type));
    // 判断上层协议
    if (ntohs(eth->type) != 0x0800) {
        printf(" (非IP包)\n");
        printf("└─────────────────────────────────────────┘\n");
        return;
    }
    printf(" (IP包)\n");
    // 2. 解析 IP 头部（跳过以太网帧头 14 字节）
    ip = (ip_header*)(packet + sizeof(ethernet_header));
    ip_header_len = (ip->ver_ihl & 0x0F) * 4;  // 头部长度（4字节为单位）
    printf("├─────────────────────────────────────────┤\n");
    printf("│ IP 协议                                 │\n");
    printf("├─────────────────────────────────────────┤\n");
    printf("│ 版本:    %d\n", (ip->ver_ihl >> 4) & 0x0F);
    printf("│ 头长:    %d 字节\n", ip_header_len);
    printf("│ TTL:     %d\n", ip->ttl);
    printf("│ 协议:    ");
    switch (ip->protocol) {
        case 6:
            printf("TCP\n");
            break;
        case 17:
            printf("UDP\n");
            break;
        case 1:
            printf("ICMP\n");
            break;
        default:
            printf("未知\n");
    }
    printf("│ 源IP:    "); print_ip(ip->src_ip); printf("\n");
    printf("│ 目的IP:  "); print_ip(ip->dest_ip); printf("\n");
    // 3. 根据协议类型解析 TCP 或 UDP 头部
    if (ip->protocol == 6) {
        // TCP
        tcp = (tcp_header*)((unsigned char*)ip + ip_header_len);
        printf("├─────────────────────────────────────────┤\n");
        printf("│ TCP 协议                                │\n");
        printf("├─────────────────────────────────────────┤\n");
        printf("│ 源端口:  %d\n", ntohs(tcp->src_port));
        printf("│ 目的端口:%d\n", ntohs(tcp->dest_port));
        printf("│ 标志位:  "); print_tcp_flags(tcp->flags); printf("\n");
        
    } else if (ip->protocol == 17) {
        // UDP
        udp = (udp_header*)((unsigned char*)ip + ip_header_len);
        printf("├─────────────────────────────────────────┤\n");
        printf("│ UDP 协议                                │\n");
        printf("├─────────────────────────────────────────┤\n");
        printf("│ 源端口:  %d\n", ntohs(udp->src_port));
        printf("│ 目的端口:%d\n", ntohs(udp->dest_port));
    }
    printf("└─────────────────────────────────────────┘\n");
    printf("\n");
}