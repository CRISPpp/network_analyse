#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <signal.h>

#include "../utils.cpp"
#include "../common.cpp"

static volatile sig_atomic_t exiting = 0;
static void sig_int(int signo) {
    exiting = 1;
}

int main()
{
    int sockfd, portno = 9999;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    pid_t pid = getpid();
    std::cout << "current PID is: " << pid << std::endl;

    // 创建socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    // 设置SO_TIMESTAMP选项
    int timestamp_on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMP, &timestamp_on, sizeof(timestamp_on)) < 0)
    {
        error("ERROR setting SO_TIMESTAMP");
    }

    // 准备服务器地址结构
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // 绑定socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    // 监听连接
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    std::cout << "Server is listening on port " << portno << "..." << std::endl;

    while (exiting == 0) {
        int newsockfd = 0;
        // 接受客户端连接
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            error("ERROR on accept");
        }

        // 接收数据
        struct msghdr msg;
        struct iovec iov;
        char control[1024];
        struct cmsghdr *cmsg;
        struct timeval *tv;

        bzero(buffer, 256);
        bzero(&msg, sizeof(msg));
        bzero(control, sizeof(control));

        iov.iov_base = buffer;
        iov.iov_len = sizeof(buffer);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = control;
        msg.msg_controllen = sizeof(control);

        n = recvmsg(newsockfd, &msg, 0);
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        tv_message data;

        // 解析控制消息
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg, cmsg))
        {
            if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMP)
            {
                tv = (struct timeval *)CMSG_DATA(cmsg);
                data.server_cmsg_tv = *tv;
                std::cout << "Packet received at: " << tv->tv_sec << "." << tv->tv_usec << " seconds" << std::endl;
                struct timeval cur_time = get_current_timeval();
                std::cout << "Network stack processing time: " << cur_time.tv_sec - tv->tv_sec << "." << cur_time.tv_usec - tv->tv_usec << " seconds" << std::endl;
            }
        }

        // 发送数据
        std::string response = "Hello from server!";
        // 添加附加时间戳
        timeval send_tv = get_current_timeval();
        std::cout << "current timestamp: " << send_tv.tv_sec << "." << send_tv.tv_usec << " seconds\n" << std::endl;

        data.server_sock_tv = send_tv;
        strncpy(data.message, response.c_str(), 255);

        n = write(newsockfd, &data, sizeof(data));
        if (n < 0)
        {
            error("ERROR writing to socket");
        }

    // 关闭连接
    close(newsockfd);
    }
    
    close(sockfd);
    return 0;
}
