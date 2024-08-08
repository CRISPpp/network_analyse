#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>

#include "xdp_packet.h"

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} packet_ringbuf SEC(".maps");

SEC("xdp")
int xdp_prog(struct xdp_md *ctx) {
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    struct packet_info *e;

    struct ethhdr *eth = data;
    struct iphdr *ip;
    struct tcphdr *tcp;

    if ((void*)eth + sizeof(*eth) > data_end)
        return XDP_ABORTED;

    if (eth->h_proto != bpf_htons(ETH_P_IP))
        return XDP_PASS;

    ip = (struct iphdr *)(eth + 1);
    if ((void*)ip + sizeof(*ip) > data_end)
        return XDP_ABORTED;

    if (ip->protocol != IPPROTO_TCP)
        return XDP_PASS;

    tcp = (struct tcphdr *)(ip + 1);
    if ((void*)tcp + sizeof(*tcp) > data_end)
        return XDP_ABORTED;
        
    struct packet_info p = {
        .src_ip = ip->saddr, 
        .dst_ip = ip->daddr, 
        .src_port = bpf_ntohs(tcp->source),
        .dst_port = bpf_ntohs(tcp->dest),
        .timestamp = bpf_ktime_get_ns() / 1000U
    };

    bpf_printk("Packet Data - src_ip: %u,  src_port: %u, timestamp(us): %llu",
                p.src_ip, p.src_port, p.timestamp);
    bpf_printk("Packet Data -  dst_ip: %u, dst_port: %u, timestamp(us): %llu\n",
                p.dst_ip, p.dst_port, p.timestamp);
    e = bpf_ringbuf_reserve(&packet_ringbuf, sizeof(*e), BPF_ANY);
    if (!e) {
        return XDP_ABORTED;
    }
    e->src_ip = p.src_ip;
    e->dst_ip = p.dst_ip;
    e->src_port = p.src_port;
    e->dst_port = p.dst_port;
    e->timestamp = p.timestamp;

    bpf_ringbuf_submit(e, BPF_ANY);
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
