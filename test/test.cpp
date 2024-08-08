#include <stdio.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <arpa/inet.h>

#include "../utils.cpp"
#include "../xdp/xdp_packet.h"

char tcp_state_str[12][30];
char tcp_state[32];
struct event {
    unsigned int tgid;
    char comm[128];
};

const char* get_state(unsigned int state) {
    switch (state) {
        case 1: return "TCP_ESTABLISHED";
        case 2: return "TCP_SYN_SENT";
        case 3: return "TCP_SYN_RECV";
        case 4: return "TCP_FIN_WAIT1";
        case 5: return "TCP_FIN_WAIT2";
        case 6: return "TCP_TIME_WAIT";
        case 7: return "TCP_CLOSE";
        case 8: return "TCP_CLOSE_WAIT";
        case 9: return "TCP_LAST_ACK";
        case 10: return "TCP_LISTEN";
        case 11: return "TCP_CLOSING";
        case 12: return "TCP_NEW_SYN_RECV";
        default: return "UNKNOWN_STATE";
    }
}

static volatile sig_atomic_t exiting = 0;
static void sig_int(int signo) {
    exiting = 1;
}

void test_get_sys_start_timestamp_us() {
    std::cout << get_sys_start_timestamp_us() << std::endl;
}

int test_cstr_eq() {
    char t[128];
    strcpy(t, "12345");
    return strcmp(t, "12345");
}

void test_hash() {
    struct packet_info p = {
        .src_ip = 0xC0A80001, // 192.168.0.1
        .dst_ip = 0xC0A80002, // 192.168.0.2
        .src_port = 12345,
        .dst_port = 80,
        .timestamp = 1625254368000000
    };
    printf("%u\n", hash_packet_info(&p));

    struct packet_info pp = {
        .src_ip = 0xC0A80011, // 192.168.0.1
        .dst_ip = 0xC0A80012, // 192.168.0.2
        .src_port = 12352,
        .dst_port = 2231,
        .timestamp = 16252512312300000
    };
    printf("%u\n", hash_packet_info(&pp));
    struct packet_info ppp = {
        .src_ip = 0xC0A80001, // 192.168.0.1
        .dst_ip = 0xC0A80002, // 192.168.0.2
        .src_port = 12345,
        .dst_port = 80,
        .timestamp = 1625254368000000
    };
    printf("%u\n", hash_packet_info(&ppp));
}

void print_ip(__u32 ip) {
    char ip_str[INET_ADDRSTRLEN];

    // Convert the __u32 IP address to a string in x.x.x.x format
    if (inet_ntop(AF_INET, &ip, ip_str, sizeof(ip_str)) != NULL) {
        printf("IP Address: %s\n", ip_str);
    } else {
        perror("inet_ntop");
    }
}

int main() {
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        fprintf(stderr, "can't set signal handler: %s\n", "error");
    }
    print_ip(2130706433);
}