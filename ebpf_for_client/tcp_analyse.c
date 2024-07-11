#include <argp.h>
#include <arpa/inet.h>
#include <bpf/bpf.h>
#include <bpf/btf.h>
#include <bpf/libbpf.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "../shm.c"
#include "tcp_analyse.h"
#include "tcp_analyse.skel.h"

#define PERF_BUFFER_PAGES 16
#define PERF_POLL_TIMEOUT_MS 100

static volatile sig_atomic_t exiting = 0;

static struct env {
    __u64 min_us;
    pid_t pid;
    bool timestamp;
    bool lport;
    bool verbose;
} env;

const char* argp_program_version = "tcp_analyse 0.1";
const char* argp_program_bug_address =
    "https://github.com/iovisor/bcc/tree/master/libbpf-tools";
const char argp_program_doc[] =
    "\nTrace TCP connects and show timestqmp.\n"
    "\n"
    "USAGE: tcp_analyse [--help] [-t] [-p PID] [-L]\n"
    "\n"
    "EXAMPLES:\n"
    "    tcp_analyse              # summarize on-CPU time as a histogram\n"
    "    tcp_analyse 1            # trace timestamp lower than 1 ms\n"
    "    tcp_analyse 0.1          # trace timestamp lower than 100 us\n"
    "    tcp_analyse -p 185       # trace PID 185 only\n"
    "    tcp_analyse -L           # include LPORT while printing outputs\n";

static const struct argp_option opts[] = {
    {"pid", 'p', "PID", 0, "Trace this PID only"},
    {"lport", 'L', NULL, 0, "Include LPORT on output"},
    {"verbose", 'v', NULL, 0, "Verbose debug output"},
    {NULL, 'h', NULL, OPTION_HIDDEN, "Show the full help"},
    {},
};

static error_t parse_arg(int key, char* arg, struct argp_state* state) {
    static int pos_args;

    switch (key) {
        case 'h':
            argp_state_help(state, stderr, ARGP_HELP_STD_HELP);
            break;
        case 'v':
            env.verbose = true;
            break;
        case 'p':
            errno = 0;
            env.pid = strtol(arg, NULL, 10);
            if (errno) {
                fprintf(stderr, "invalid PID: %s\n", arg);
                argp_usage(state);
            }
            break;
        case 'L':
            env.lport = true;
            break;
        case ARGP_KEY_ARG:
            if (pos_args++) {
                fprintf(stderr, "Unrecognized positional argument: %s\n", arg);
                argp_usage(state);
            }
            errno = 0;
            env.min_us = strtod(arg, NULL) * 1000;
            if (errno || env.min_us <= 0) {
                fprintf(stderr, "Invalid delay (in us): %s\n", arg);
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static int libbpf_print_fn(enum libbpf_print_level level,
                           const char* format,
                           va_list args) {
    if (level == LIBBPF_DEBUG && !env.verbose)
        return 0;
    return vfprintf(stderr, format, args);
}

static void sig_int(int signo) {
    exiting = 1;
}

void handle_event(void* ctx, int cpu, void* data, __u32 data_sz) {
    const struct event* e = data;
    if (strcmp(e->func, "tcp_v4_connect") == 0) {

    } else if (strcmp(e->func, "tcp_rcv_state_process") == 0){ 

    }
    char src[INET6_ADDRSTRLEN];
    char dst[INET6_ADDRSTRLEN];
    union {
        struct in_addr x4;
        struct in6_addr x6;
    } s, d;
    pid_t e_pid = e->tgid;
    if (env.pid != 0 && e_pid != env.pid) {
        return;
    }
    // if (e->af == AF_INET) {
        s.x4.s_addr = e->saddr_v4;
        d.x4.s_addr = e->daddr_v4;
    // } 
    // else if (e->af == AF_INET6) {
    //     memcpy(&s.x6.s6_addr, e->saddr_v6, sizeof(s.x6.s6_addr));
    //     memcpy(&d.x6.s6_addr, e->daddr_v6, sizeof(d.x6.s6_addr));
    // } else {
    //     return;
    // }

    if (env.lport) {
        printf("%-6d %-12.12s %-2d %-16s %-6d %-16s %-5d %.2f %s %s %s %lld\n", e->tgid,
               e->comm, e->af == AF_INET ? 4 : 6,
               inet_ntop(e->af, &s, src, sizeof(src)), e->lport,
               inet_ntop(e->af, &d, dst, sizeof(dst)), ntohs(e->dport),
               e->delta_us / 1000.0, e->func, e->tcp_state, e->tcp_description, e->tcp_connect_time);
    } else {
        printf("%-6d %-12.12s %-2d %-16s %-16s %-5d %.2f %s %s %s %lld\n", e->tgid, e->comm,
               e->af == AF_INET ? 4 : 6, inet_ntop(e->af, &s, src, sizeof(src)),
               inet_ntop(e->af, &d, dst, sizeof(dst)), ntohs(e->dport),
               e->delta_us / 1000.0, e->func, e->tcp_state, e->tcp_description, e->tcp_connect_time);
    }
}

void handle_lost_events(void* ctx, int cpu, __u64 lost_cnt) {
    fprintf(stderr, "lost %llu events on CPU #%d\n", lost_cnt, cpu);
}
static bool fentry_try_attach(int id) {
    int prog_fd, attach_fd;
    char error[4096];
    struct bpf_insn insns[] = {
        {.code = BPF_ALU64 | BPF_MOV | BPF_K, .dst_reg = BPF_REG_0, .imm = 0},
        {.code = BPF_JMP | BPF_EXIT},
    };
    LIBBPF_OPTS(bpf_prog_load_opts, opts,
                .expected_attach_type = BPF_TRACE_FENTRY, .attach_btf_id = id,
                .log_buf = error, .log_size = sizeof(error), );

    prog_fd = bpf_prog_load(BPF_PROG_TYPE_TRACING, "test", "GPL", insns,
                            sizeof(insns) / sizeof(struct bpf_insn), &opts);
    if (prog_fd < 0)
        return false;

    attach_fd = bpf_raw_tracepoint_open(NULL, prog_fd);
    if (attach_fd >= 0)
        close(attach_fd);

    close(prog_fd);
    return attach_fd >= 0;
}
static bool fentry_can_attach(const char* name, const char* mod) {
    struct btf *btf, *vmlinux_btf, *module_btf = NULL;
    int err, id;

    vmlinux_btf = btf__load_vmlinux_btf();
    err = libbpf_get_error(vmlinux_btf);
    if (err)
        return false;

    btf = vmlinux_btf;

    if (mod) {
        module_btf = btf__load_module_btf(mod, vmlinux_btf);
        err = libbpf_get_error(module_btf);
        if (!err)
            btf = module_btf;
    }

    id = btf__find_by_name_kind(btf, name, BTF_KIND_FUNC);

    btf__free(module_btf);
    btf__free(vmlinux_btf);
    return id > 0 && fentry_try_attach(id);
}

int main(int argc, char** argv) {
    static const struct argp argp = {
        .options = opts,
        .parser = parse_arg,
        .doc = argp_program_doc,
    };
    struct perf_buffer* pb = NULL;
    struct tcp_analyse_bpf* obj;
    int err;

    err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
    if (err)
        return err;

    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
    libbpf_set_print(libbpf_print_fn);

    obj = tcp_analyse_bpf__open();
    if (!obj) {
        fprintf(stderr, "failed to open BPF object\n");
        return 1;
    }

    /* initialize global data (filtering options) */
    obj->rodata->targ_min_us = env.min_us;
    obj->rodata->targ_tgid = env.pid;
    // 这里的函数名是内核函数名
    if (fentry_can_attach("tcp_v4_connect", NULL)) {
        // printf("attach tcp_v4_connect\n");
        bpf_program__set_attach_target(obj -> progs.fentry_tcp_v4_connect, 0, "tcp_v4_connect");
    } else {
        bpf_program__set_autoload(obj->progs.fentry_tcp_v4_connect, false);
    }
    
    // 这里的函数名是内核函数名
    if (fentry_can_attach("tcp_rcv_state_process", NULL)) {
        // printf("attach tcp_rcv_state_process\n");
        bpf_program__set_attach_target(obj->progs.fentry_tcp_rcv_state_process,
                                       0, "tcp_rcv_state_process");
    } else {
        bpf_program__set_autoload(obj->progs.fentry_tcp_rcv_state_process,
                                  false);
    }

    err = tcp_analyse_bpf__load(obj);
    if (err) {
        fprintf(stderr, "failed to load BPF object: %d\n", err);
        goto cleanup;
    }

    err = tcp_analyse_bpf__attach(obj);
    if (err) {
        goto cleanup;
    }

    pb = perf_buffer__new(bpf_map__fd(obj->maps.events), PERF_BUFFER_PAGES,
                          handle_event, handle_lost_events, NULL, NULL);
    if (!pb) {
        fprintf(stderr, "failed to open perf buffer: %d\n", errno);
        goto cleanup;
    }

    /* print header */
    if (env.timestamp)
        printf("%-9s ", ("TIME(s)"));
    if (env.lport) {
        printf("%-6s %-12s %-2s %-16s %-6s %-16s %-5s %s %s %s %s %s\n", "PID", "COMM",
               "IP", "SADDR", "LPORT", "DADDR", "DPORT", "TIMESTAMP(ms)", "FUNC", "TCP_STATE", "TCP_DESCRIPTION", "TCP_CONNECT_TIME(us)");
    } else {
        printf("%-6s %-12s %-2s %-16s %-16s %-5s %s %s %s %s %s\n", "PID", "COMM", "IP",
               "SADDR", "DADDR", "DPORT", "TIMESTAMP(ms)", "FUNC", "TCP_STATE", "TCP_DESCRIPTION", "TCP_CONNECT_TIME(us)");
    }

    if (signal(SIGINT, sig_int) == SIG_ERR) {
        fprintf(stderr, "can't set signal handler: %s\n", strerror(errno));
        err = 1;
        goto cleanup;
    }

    /* main: poll */
    while (!exiting) {
        err = perf_buffer__poll(pb, PERF_POLL_TIMEOUT_MS);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "error polling perf buffer: %s\n", strerror(-err));
            goto cleanup;
        }
        /* reset err to return 0 if exiting */
        err = 0;
    }

cleanup:
    perf_buffer__free(pb);
    tcp_analyse_bpf__destroy(obj);

    return err != 0;
}