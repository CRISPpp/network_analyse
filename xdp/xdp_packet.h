#include <stdio.h>
#include <stdint.h>
#include <linux/types.h>

#ifndef __XDP_PACKET_H
#define __XDP_PACKET_H

struct packet_info {
    __u32 src_ip;
    __u32 dst_ip;
    __u16 src_port;
    __u16 dst_port;
    __u64 timestamp;
};

__u32 hash_packet_info(const struct packet_info *pi) {
    __u32 hash = 0;

    hash ^= pi->src_ip;
    hash ^= pi->dst_ip;
    hash ^= pi->src_port << 16;
    hash ^= pi->dst_port;
    hash ^= (__u32)(pi->timestamp & 0xFFFFFFFF);
    hash ^= (__u32)((pi->timestamp >> 32) & 0xFFFFFFFF);

    return hash;
}

#endif