#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>

/* Simple XDP program that retrieves and prints TCP packet information. */
SEC("xdp")
int xdp_prog(struct xdp_md *ctx) {
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;

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

    __u32 src_ip = bpf_ntohl(ip->saddr);
    __u32 dst_ip = bpf_ntohl(ip->daddr);
    __u16 src_port = bpf_ntohs(tcp->source);
    __u16 dst_port = bpf_ntohs(tcp->dest);
    __u64 timestamp = bpf_ktime_get_ns() / 1000U;

    bpf_printk("Packet Data - src_ip: %u,  src_port: %u, timestamp(us): %llu",
                src_ip, src_port, timestamp);
    bpf_printk("Packet Data -  dst_ip: %u, dst_port: %u, timestamp(us): %llu\n",
                dst_ip, dst_port, timestamp);


    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
