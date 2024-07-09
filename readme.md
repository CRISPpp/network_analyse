# a project to analyse network latency developed CRISPpp
# USAGE
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
./bin/tcp_analyse_service -p $(PID from test client demo)
```