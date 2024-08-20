# a project to analyse network latency developed CRISPpp
# Result example with service run in one machine with 3000bit size packet
## xdp
```
Packet Data - src_ip: 127.0.0.1,  src_port: 53710, dst_ip: 127.0.0.1,  dst_port: 9999, timestamp(us): 673422472
Packet Data - src_ip: 127.0.0.1,  src_port: 9999, dst_ip: 127.0.0.1,  dst_port: 53710, timestamp(us): 673422561
Packet Data - src_ip: 127.0.0.1,  src_port: 53710, dst_ip: 127.0.0.1,  dst_port: 9999, timestamp(us): 673422624
Packet Data - src_ip: 127.0.0.1,  src_port: 53710, dst_ip: 127.0.0.1,  dst_port: 9999, timestamp(us): 673423268
Packet Data - src_ip: 127.0.0.1,  src_port: 9999, dst_ip: 127.0.0.1,  dst_port: 53710, timestamp(us): 673423302
Packet Data - src_ip: 127.0.0.1,  src_port: 53710, dst_ip: 127.0.0.1,  dst_port: 9999, timestamp(us): 673423395
Packet Data - src_ip: 127.0.0.1,  src_port: 9999, dst_ip: 127.0.0.1,  dst_port: 53710, timestamp(us): 673423563
Packet Data - src_ip: 127.0.0.1,  src_port: 53710, dst_ip: 127.0.0.1,  dst_port: 9999, timestamp(us): 673423646
```
## ebpf for client
```
PID    COMM         IP SADDR            DADDR            DPORT TIMESTAMP(us) FUNC TCP_STATE TCP_DESCRIPTION TCP_CONNECT_TIME(us)
37759  client       4  0.0.0.0          0.0.0.0          0     673422390 tcp_v4_connect TCP_CLOSE client send SYN packet 0
37759  client       4  127.0.0.1        127.0.0.1        9999  673423229 tcp_sendmsg TCP_ESTABLISHED client send message
37759  client       4  0.0.0.0          0.0.0.0          0     673422526 tcp_rcv_state_process TCP_LISTEN transfer into new state 0
37759  client       4  127.0.0.1        127.0.0.1        9999  673422597 tcp_rcv_state_process TCP_SYN_SENT transfer into new state 207
37759  client       4  127.0.0.1        127.0.0.1        53710 673422668 tcp_rcv_state_process TCP_SYN_RECV transfer into new state 0
30177  server       4  127.0.0.1        127.0.0.1        9999  673423605 tcp_rcv_state_process TCP_FIN_WAIT1 transfer into new state 0
30177  server       4  127.0.0.1        127.0.0.1        53710 673423682 tcp_rcv_state_process TCP_LAST_ACK transfer into new state 0
```
## ebpf for service
```
PID    COMM         IP SADDR            DADDR            DPORT TIMESTAMP(us) FUNC TCP_STATE TCP_DESCRIPTION
30177  server       4  127.0.0.1        127.0.0.1        53710 673423330 tcp_recvmsg TCP_ESTABLISHED server receive message
37759  client       6  (null)           (null)           0     673422502 tcp_v4_rcv UNKNOWN_STATE server receive SYN packet
37759  client       4  0.0.0.0          0.0.0.0          0     673422516 tcp_v4_do_rcv TCP_LISTEN rcv ack
37759  client       4  0.0.0.0          0.0.0.0          0     673422522 tcp_rcv_state_process TCP_LISTEN server state change
37759  client       6  (null)           (null)           0     673422579 tcp_v4_rcv UNKNOWN_STATE server receive  packet
37759  client       4  127.0.0.1        127.0.0.1        9999  673422591 tcp_v4_do_rcv TCP_SYN_SENT rcv ack
37759  client       4  127.0.0.1        127.0.0.1        9999  673422594 tcp_rcv_state_process TCP_SYN_SENT server state change
37759  client       6  (null)           (null)           0     673422639 tcp_v4_rcv UNKNOWN_STATE server receive  packet
37759  client       4  127.0.0.1        127.0.0.1        53710 673422663 tcp_rcv_state_process TCP_SYN_RECV server state change
37759  client       6  (null)           (null)           0     673423283 tcp_v4_rcv UNKNOWN_STATE server receive  packet
37759  client       4  127.0.0.1        127.0.0.1        53710 673423289 tcp_v4_do_rcv TCP_ESTABLISHED rcv ack
37759  client       6  (null)           (null)           0     673423308 tcp_v4_rcv UNKNOWN_STATE server receive  packet
37759  client       4  127.0.0.1        127.0.0.1        9999  673423317 tcp_v4_do_rcv TCP_ESTABLISHED rcv ack
37759  client       6  (null)           (null)           0     673423411 tcp_v4_rcv UNKNOWN_STATE server receive  packet
37759  client       4  127.0.0.1        127.0.0.1        53710 673423419 tcp_v4_do_rcv TCP_ESTABLISHED rcv ack
30177  server       4  127.0.0.1        127.0.0.1        53710 673423526 tcp_recvmsg TCP_CLOSE_WAIT server receive message
30177  server       6  (null)           (null)           0     673423586 tcp_v4_rcv UNKNOWN_STATE server receive  packet
30177  server       4  127.0.0.1        127.0.0.1        9999  673423597 tcp_v4_do_rcv TCP_FIN_WAIT1 rcv ack
30177  server       4  127.0.0.1        127.0.0.1        9999  673423602 tcp_rcv_state_process TCP_FIN_WAIT1 server state change
30177  server       6  (null)           (null)           0     673423660 tcp_v4_rcv UNKNOWN_STATE server receive  packet
30177  server       4  127.0.0.1        127.0.0.1        53710 673423674 tcp_v4_do_rcv TCP_LAST_ACK rcv ack
30177  server       4  127.0.0.1        127.0.0.1        53710 673423678 tcp_rcv_state_process TCP_LAST_ACK server state change
```
# USAGE
## ebpf config for ubuntu

```
sudo apt install clang llvm libelf1 libelf-dev zlib1g-dev bpfcc-tools linux-headers-$(uname -r) g++-multilib libpcap-dev build-essential libc6-dev-i386 linux-tools-$(uname -r) linux-tools-common linux-tools-generic tcpdump m4
```
if you cant not get vmlinux.h, you can generate it with command below
```
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
```
## complie
```
./complie.sh
```
## run the test demo
```
./bin/server
./bin/client
```
## run the ebpf part
```
./bin/tcp_analyse_service
./bin/tcp_analyse
```
## run the ebpf part with PID
```
./bin/tcp_analyse_service -p $(PID from test service demo)
./bin/tcp_analyse -p $(PID from test client demo)
```

## load xdp part by ip
```
ip link set dev lo xdpgeneric obj ../bin/xdp_xxx.o sec xdp
```
## remove xdp part by ip
```
ip link set dev lo xdpgeneric off
```

## load xdp by bin
```
cd bin
./xxx -d "interface"
it will auto unload xdp prog auto ctrl-c
```

## show debug log
```
cat /sys/kernel/debug/tracing/trace_pipe 
```
## info
```
time in the program is epoch time rather then unix time
the xdp program just listens the src_port = 9999 or dst_port = 9999
```