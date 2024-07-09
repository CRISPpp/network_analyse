# a project to analyse network latency developed CRISPpp
# USAGE
## ebpf config for ubuntu
```
sudo apt install make clang llvm libelf1 libelf-dev zlib1g-dev bpfcc-tools linux-headers-$(uname -r)
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