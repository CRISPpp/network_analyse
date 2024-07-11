/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#ifndef __TCP_ANALYSE_H
#define __TCP_ANALYSE_H
// #include <inttypes.h>
typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;
typedef unsigned long long __u64;

#define TASK_COMM_LEN 16
#define TASK_FUNC_LEN 32
#define TCP_STATE_LEN 32
#define TCP_DESCRIPTION_LEN 128

struct event {
    union {
        __u32 saddr_v4;
        __u8 saddr_v6[16];
    };
    union {
        __u32 daddr_v4;
        __u8 daddr_v6[16];
    };
    char comm[TASK_COMM_LEN];
    char func[TASK_FUNC_LEN];
    char tcp_state[TCP_STATE_LEN];
    char tcp_description[TCP_DESCRIPTION_LEN];
    __u64 delta_us;
    __u64 ts_us;
    __u32 tgid;
    int af;
    __u16 lport;
    __u16 dport;
    __u64 tcp_connect_time;
};

#endif /* __TCP_ANALYSE_H_ */