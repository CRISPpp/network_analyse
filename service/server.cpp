#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <vector>
#include <signal.h>

#include "../common.cpp"
#include "../utils.cpp"

#define PORT 9999
#define MAX_EVENTS 10
#define BUFFER_SIZE 4096
#define NUM_THREADS 4

static volatile sig_atomic_t exiting = 0;
static void sig_int(int signo) {
    exiting = 1;
}

void set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

struct ThreadData {
    int epoll_fd;
    int thread_id;
};

void* worker_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int epoll_fd = data->epoll_fd;
    int thread_id = data->thread_id;
    struct epoll_event events[MAX_EVENTS];

    std::cout << "Worker thread " << thread_id << " started" << std::endl;

    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            int client_fd = events[i].data.fd;
            if (events[i].events & EPOLLIN) {
                struct msghdr msg;
                struct iovec iov;
                char buffer[BUFFER_SIZE];
                char control[CMSG_SPACE(sizeof(struct timeval))];
                struct cmsghdr *cmsg;
                struct timeval *tv;

                memset(&msg, 0, sizeof(msg));
                memset(buffer, 0, BUFFER_SIZE);
                memset(control, 0, sizeof(control));

                iov.iov_base = buffer;
                iov.iov_len = BUFFER_SIZE;

                msg.msg_name = nullptr;
                msg.msg_namelen = 0;
                msg.msg_iov = &iov;
                msg.msg_iovlen = 1;
                msg.msg_control = control;
                msg.msg_controllen = sizeof(control);

                // 使用 recvmsg 读取数据和控制信息
                int bytes_read = recvmsg(client_fd, &msg, 0);
                if (bytes_read <= 0) {
                    // 客户端关闭连接或读取出错
                    close(client_fd);
                    std::cout << "Connection closed by client on thread " << thread_id << std::endl;
                } else {
                    cmsg = CMSG_FIRSTHDR(&msg);
                    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg, cmsg))
                    {
                        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMP)
                        {
                            tv = (struct timeval *)CMSG_DATA(cmsg);
                            std::cout << "Packet received at: " << tv->tv_sec << "." << (float)tv->tv_usec / 1000000 << " seconds" << std::endl;
                            struct timeval cur_time = get_current_timeval();
                            std::cout << "Network stack processing time: " << cur_time.tv_sec - tv->tv_sec << "." << (float)(cur_time.tv_usec - tv->tv_usec) / 1000000 << " seconds" << std::endl;
                        }
                    }
                    // 读取数据并打印
                    buffer[bytes_read] = '\0';
                    std::cout << "Received on thread " << thread_id << ": " << buffer << std::endl;
                }
            }
        }
    }

    return nullptr;
}

int main() {
    if (signal(SIGINT, sig_int) == SIG_ERR) {
       error("can't set signal handler: %s\n");
    }
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    struct epoll_event event;
    int addrlen = sizeof(address);

    // 创建服务器套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("socket failed");
    }
    // 设置SO_TIMESTAMP选项
    int timestamp_on = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_TIMESTAMP, &timestamp_on, sizeof(timestamp_on)) < 0)
    {
        error("ERROR setting SO_TIMESTAMP");
    }
    // 绑定套接字
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        error("bind failed");
    }

    // 监听
    if (listen(server_fd, 3) < 0) {
        close(server_fd);
        error("listen");
    }

    // 设置服务器套接字为非阻塞
    set_non_blocking(server_fd);

    // 创建线程池
    std::vector<pthread_t> threads(NUM_THREADS);
    std::vector<ThreadData> thread_data(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i) {
        // 创建 epoll 实例
        thread_data[i].epoll_fd = epoll_create1(0);
        if (thread_data[i].epoll_fd == -1) {
            close(server_fd);
            error("epoll_create1");
        }

        thread_data[i].thread_id = i;
        pthread_create(&threads[i], nullptr, worker_thread, &thread_data[i]);
    }

    std::cout << "Server started on port " << PORT << std::endl;

    while (exiting == 0) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket != -1) {
            set_non_blocking(new_socket);
            std::cout << "Accepted new connection" << std::endl;

            // 将新的客户端连接分配给工作线程
            int thread_index = new_socket % NUM_THREADS;
            event.data.fd = new_socket;
            event.events = EPOLLIN;
            epoll_ctl(thread_data[thread_index].epoll_fd, EPOLL_CTL_ADD, new_socket, &event);
        }
    }

    close(server_fd);
    for (int i = 0; i < NUM_THREADS; ++i) {
        close(thread_data[i].epoll_fd);
    }
    std::cout << "close server\n";
    return 0;
}