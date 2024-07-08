// SPDX-License-Identifier: GPL-2.0
#include <vmlinux.h>
#include <string.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>
#include "tcp_analyse.h"

#define AF_INET    2
#define AF_INET6   10

const volatile __u64 targ_min_us = 0;
const volatile pid_t targ_tgid = 0;

struct piddata {
	char comm[TASK_COMM_LEN];
	u64 ts;
	u32 tgid;
};



const char* get_tcp_state(u32 state) {
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

struct {
	__uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
	__uint(key_size, sizeof(u32));
	__uint(value_size, sizeof(u32));
} events SEC(".maps");

static int handle_tcp_rcv_state_process(void *ctx, struct sock *sk)
{
	struct event event = {};
	u64 ts;
	u32 tgid = bpf_get_current_pid_tgid() >> 32;

	ts = (s64)bpf_ktime_get_ns();

	event.delta_us = ts / 1000U;
	if (targ_min_us && event.delta_us < targ_min_us)
		goto cleanup;
	u32 state = BPF_CORE_READ(sk, __sk_common.skc_state);
	bpf_get_current_comm(&event.comm, sizeof(event.comm));
	char* description = "transfer into new state";
	const char* state_str = get_tcp_state(state);
	strcat(description, state_str);
	memcpy(event.tcp_description, description, sizeof(event.tcp_description));
	memcpy(event.func, "tcp_rcv_state_process", sizeof(event.func));
	memcpy(event.tcp_state, get_tcp_state(state), sizeof(event.tcp_state));
	event.ts_us = ts / 1000U;
	event.tgid = tgid;
	event.lport = BPF_CORE_READ(sk, __sk_common.skc_num);
	event.dport = BPF_CORE_READ(sk, __sk_common.skc_dport);
	event.af = BPF_CORE_READ(sk, __sk_common.skc_family);
	
	if (event.af == AF_INET) {
		event.saddr_v4 = BPF_CORE_READ(sk, __sk_common.skc_rcv_saddr);
		event.daddr_v4 = BPF_CORE_READ(sk, __sk_common.skc_daddr);
	} else {
		BPF_CORE_READ_INTO(&event.saddr_v6, sk,
				__sk_common.skc_v6_rcv_saddr.in6_u.u6_addr32);
		BPF_CORE_READ_INTO(&event.daddr_v6, sk,
				__sk_common.skc_v6_daddr.in6_u.u6_addr32);
	}

	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
			&event, sizeof(event));

	cleanup:
	return 0;
}

static int handle_tcp_v4_connect(void *ctx, struct sock *sk)
{
	struct event event = {};
	u64 ts;
	u32 tgid = bpf_get_current_pid_tgid() >> 32;

	ts = (s64)bpf_ktime_get_ns();
	u32 state = BPF_CORE_READ(sk, __sk_common.skc_state);
	event.delta_us = ts / 1000U;
	if (targ_min_us && event.delta_us < targ_min_us)
		goto cleanup;
	bpf_get_current_comm(&event.comm, sizeof(event.comm));
	memcpy(event.func, "tcp_v4_connect", sizeof(event.func));
	const char* description = "client send SYN packet";
	memcpy(event.tcp_description, description, sizeof(event.tcp_description));
	memcpy(event.tcp_state, get_tcp_state(state), sizeof(event.tcp_state));
	event.ts_us = ts / 1000;
	event.tgid = tgid;
	event.lport = BPF_CORE_READ(sk, __sk_common.skc_num);
	event.dport = BPF_CORE_READ(sk, __sk_common.skc_dport);
	event.af = BPF_CORE_READ(sk, __sk_common.skc_family);

	if (event.af == AF_INET) {
		event.saddr_v4 = BPF_CORE_READ(sk, __sk_common.skc_rcv_saddr);
		event.daddr_v4 = BPF_CORE_READ(sk, __sk_common.skc_daddr);
	} else {
		BPF_CORE_READ_INTO(&event.saddr_v6, sk,
				__sk_common.skc_v6_rcv_saddr.in6_u.u6_addr32);
		BPF_CORE_READ_INTO(&event.daddr_v6, sk,
				__sk_common.skc_v6_daddr.in6_u.u6_addr32);
	}
	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU,
			&event, sizeof(event));

cleanup:
	return 0;
}

SEC("fentry/tcp_rcv_state_process")
int BPF_PROG(fentry_tcp_rcv_state_process, struct sock *sk)
{
	return handle_tcp_rcv_state_process(ctx, sk);
}


SEC("fentry/tcp_v4_connect")
int BPF_PROG(fentry_tcp_v4_connect, struct sock *sk)
{
	return handle_tcp_v4_connect(ctx, sk);
}

char LICENSE[] SEC("license") = "GPL";