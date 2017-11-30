#ifndef BAPI_H
#define BAPI_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "custlist.h"

#define BUF_SIZE 1024
#define EPOLL_RUN_TIMEOUT -1
#define EPOLL_SIZE 7000

int SetNB(int sockfd);
void HMessage(void* ptrData, uint32_t events);
void modifyEpollContext(int epollfd, int operation, int fd, uint32_t events, void* data);
#endif
