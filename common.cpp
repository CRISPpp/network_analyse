#include<iostream>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

struct tv_message 
{
    struct timeval server_sock_tv;
    struct timeval server_cmsg_tv;
    char message[256];
};