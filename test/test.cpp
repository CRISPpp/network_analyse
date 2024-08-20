#include <stdio.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>

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

void test_env() {
    // 默认值
    char *pin_basedir = strdup("/sys/fs/bpf");
    if (pin_basedir == NULL) {
        fprintf(stderr, "Failed to allocate memory for pin_basedir\n");
        return;
    }

    // 从环境变量读取新值
    const char *env_value = getenv("BPF_PIN_BASEDIR");
    if (env_value != NULL) {
        char *temp = (char*)realloc(pin_basedir, strlen(env_value) + 1);
        if (temp == NULL) {
            fprintf(stderr, "Failed to allocate memory for new pin_basedir\n");
            free(pin_basedir);  // 释放原来的内存
            return;
        }
        pin_basedir = temp;
        strcpy(pin_basedir, env_value);
    }

    printf("Current pin_basedir: %s\n", pin_basedir);

    // 释放内存
    free(pin_basedir);
}

#define CONFIG_FILE "config.txt"
#define FIELD "key"

char *read_field_value(const char *filename, const char *field) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return NULL;
    }

    char line[256];
    static char value[256] = {0};

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, field, strlen(field)) == 0 && line[strlen(field)] == '=') {
            char *newline = strchr(line, '\n');
            if (newline) {
                *newline = '\0';
            }
            strcpy(value, line + strlen(field) + 1);
            fclose(file);
            return value;
        }
    }

    fclose(file);
    return NULL;
}

void monitor_config(const char *filename, const char *field, int interval) {
    char *prev_value = NULL;
    while (1) {
        char *current_value = read_field_value(filename, field);
        if (current_value && (!prev_value || strcmp(prev_value, current_value) != 0)) {
            if (prev_value) {
                printf("Field '%s' changed from '%s' to '%s'\n", field, prev_value, current_value);
            } else {
                printf("Field '%s' initialized with value '%s'\n", field, current_value);
            }
            if (current_value) {
                free(prev_value);
                prev_value = strdup(current_value);
            }
        }
        sleep(interval);
    }
}

void test_readConfig() {
    monitor_config(CONFIG_FILE, FIELD, 5);
}

int main() {
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        fprintf(stderr, "can't set signal handler: %s\n", "error");
    }
    while(true) {
        test_get_sys_start_timestamp_us();
    }
}