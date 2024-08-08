# a project to analyse network latency developed CRISPpp
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
./xxx
it will auto unload xdp prog auto ctrl-c
```

## show debug log
```
cat /sys/kernel/debug/tracing/trace_pipe 
```