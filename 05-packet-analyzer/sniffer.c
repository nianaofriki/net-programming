// 网络嗅探器（抓包程序）
// 使用 WinPcap/Npcap 库捕获网络数据包
#include <pcap.h>// 包含 WinPcap/Npcap 头文件
#include <stdio.h>
#include <stdlib.h>

// 声明外部函数（定义在 analyze.c 中）
void analyze_packet(const unsigned char *packet);

// 回调函数：每收到一个数据包时调用
void packet_handler(u_char *user, const struct pcap_pkthdr *header, const u_char *packet) {
    static int packet_count = 0;
    
    packet_count++;
    printf("\n========== 数据包 #%d ==========\n", packet_count);
    printf("捕获时间: %ld.%06ld 秒\n", header->ts.tv_sec, header->ts.tv_usec);
    printf("包长度: %d 字节\n", header->len);
    
    // 调用解析函数
    analyze_packet(packet);
}
int main() {
    pcap_if_t *alldevs, *d;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    int i = 0;
    int choice;
    // 1. 获取本机网卡列表
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        printf("获取网卡列表失败: %s\n", errbuf);
        return -1;
    }
    // 2. 打印网卡列表
    printf("本机网卡列表:\n");
    for (d = alldevs; d; d = d->next) {
        printf("%d. %s\n", ++i, d->description ? d->description : d->name);
    }
    if (i == 0) {
        printf("没有找到网卡，请安装 Npcap 后重试\n");
        pcap_freealldevs(alldevs);
        return -1;
    }
    // 3. 用户选择网卡
    printf("请选择要监听的网卡编号: ");
    printf("\n请选择网卡编号 (1-%d): ", i);
    scanf("%d", &choice);
    
    if (choice < 1 || choice > i) {
        printf("无效选择\n");
        pcap_freealldevs(alldevs);
        return -1;
    }
