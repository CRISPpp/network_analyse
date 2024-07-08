#include <stdio.h>
#include <string.h>
#include <signal.h>

char tcp_state_str[12][30];
char tcp_state[32];

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

int main() {
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        fprintf(stderr, "can't set signal handler: %s\n", "error");
    }
    while (exiting == 0) {
        ;
    }
    
    memcpy(tcp_state, get_state(2), 32);
    printf("%s", tcp_state);
}