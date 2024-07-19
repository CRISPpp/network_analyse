#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#include "../utils.cpp"
#include "../common.cpp"

static volatile sig_atomic_t exiting = 0;
static void sig_int(int signo) {
    exiting = 1;
}

int main()
{
    pid_t pid = getpid();

    std::cout << "current PID is: " << pid << std::endl;
    // while (exiting == 0)
    // {        
        int sockfd, portno = 9999;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        char buffer[4096];

        // 创建socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            error("ERROR opening socket");
        }

        // 设置SO_TIMESTAMP选项
        int timestamp_on = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMP, &timestamp_on, sizeof(timestamp_on)) < 0) {
            error("ERROR setting SO_TIMESTAMP");
        }

        // 获取服务器地址
        server = gethostbyname("localhost");
        if (server == nullptr)
        {
            fprintf(stderr, "ERROR, no such host\n");
            exit(0);
        }

        // 准备服务器地址结构
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(portno);

        // 连接服务器
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            error("ERROR connecting");
        }

        // 发送数据
        std::string message = generateRandomString();
        bzero(buffer, sizeof(buffer));
        strncpy(buffer, message.c_str(), sizeof(buffer));
        int n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
        {
            error("ERROR writing to socket");
        }
        // // 接收数据
        // struct msghdr msg;
        // struct iovec iov;
        // char control[1024];
        // struct cmsghdr *cmsg;
        // struct timeval *tv;
        // tv_message recv_data;

        // bzero(buffer, 256);
        // bzero(&msg, sizeof(msg));
        // bzero(control, sizeof(control));

        // iov.iov_base = &recv_data;
        // iov.iov_len = sizeof(recv_data);
        // msg.msg_iov = &iov;
        // msg.msg_iovlen = 1;
        // msg.msg_control = control;
        // msg.msg_controllen = sizeof(control);

        // n = recvmsg(sockfd, &msg, 0);
        // if (n < 0) {
        //     error("ERROR reading from socket");
        // }

        // // 解析控制消息
        // for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        //     if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMP) {
        //         tv = (struct timeval *) CMSG_DATA(cmsg);
        //         std::cout << "Packet received at : " << tv->tv_sec << "." << tv->tv_usec << " seconds" << std::endl;
        //         std::cout << "Network transfer time: " << tv->tv_sec - recv_data.server_sock_tv.tv_sec << "." << tv->tv_usec - recv_data.server_sock_tv.tv_usec << " seconds" << std::endl;
        //     }
        // }

        // // 输出接收到的时间戳和消息
        // std::cout << "Received from server: " << recv_data.message << std::endl;
        // std::cout << "Sock timestamp from server: " << recv_data.server_sock_tv.tv_sec << "." << recv_data.server_sock_tv.tv_usec << " seconds" << std::endl;
        // std::cout << "Cmsg timestamp from server: " << recv_data.server_cmsg_tv.tv_sec << "." << recv_data.server_cmsg_tv.tv_usec << " seconds\n" << std::endl;


        // 关闭连接
        close(sockfd);
        sleep(5);
   //}
    std::cout << "exit\n";

    return 0;
}
