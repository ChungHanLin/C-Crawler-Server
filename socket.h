//
//  socket.h
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#ifndef socket_h
#define socket_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "struct.h"

#define PORT_NUM 8080
#define BACKLOG_NUM 10

#define true 1
#define false 0

typedef unsigned int bool;

typedef struct client {
    int socketFD;
    char *account;
}Client;

int create_socket(void);

void init_socket(struct sockaddr_in *socket, char *socket_addr);

void flush_input(char *buffer);

void bind_addr(int, struct sockaddr_in *);

void listen_socket(int);

void init_selector(int, fd_set *);

void select_client(int, fd_set *);

int accept_client(int, struct sockaddr_in *);

void insert_fd_set(int, fd_set *, int *, int *);

void clear_socket(int, fd_set *, int *, int *);



#endif /* socket_h */
