//
//  socket.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "socket.h"

int create_socket(void) {
    int socket_fd;

    // create TCP connection
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

void init_socket(struct sockaddr_in *socket, char *socket_addr) {
    socket->sin_family = AF_INET;
    socket->sin_port = htons(PORT_NUM);

    if (socket_addr) {
        socket->sin_addr.s_addr = inet_addr(socket_addr);
        inet_aton(socket_addr, (struct in_addr *) &(socket->sin_addr.s_addr));
    }
    else {
        socket->sin_addr.s_addr = INADDR_ANY;
    }
}

void flush_input(char *buffer) {
    char c;
    char *head = buffer;
    
    while ((c = getchar()) != '\n' && c != EOF) {
        *head = c;
        head++;
    }
    *head = '\0';
}

void bind_addr(int socketFD, struct sockaddr_in *info) {
    int c = 1;

    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &c, sizeof(int));

    if (bind(socketFD, (struct sockaddr *) info, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        close(socketFD);
        exit(EXIT_FAILURE);
    }
}

void listen_socket(int socketFD) {
    if (listen(socketFD, BACKLOG_NUM) == -1) {
        perror("listen");
        close(socketFD);
        exit(EXIT_FAILURE);
    }
}

void init_selector(int socketFD, fd_set *master) {
    FD_ZERO(master);
    FD_SET(socketFD, master);
}

void select_client(int socketFD, fd_set *read_fds) {

    if (select(socketFD + 1, read_fds, NULL, NULL, NULL) == -1) {
        perror("select");
        close(socketFD);
        exit(EXIT_FAILURE);
    }
}

int accept_client(int serverFD, struct sockaddr_in *client_info) {
    socklen_t length = sizeof(struct sockaddr_in);

    int clientFD = accept(serverFD, (struct sockaddr *) client_info, &length);

    if (clientFD == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return clientFD;
}

void insert_fd_set(int clientFD, fd_set *master, int *minFD, int *maxFD) {
    FD_SET(clientFD, master);

    if (clientFD > *maxFD) {
        *maxFD = clientFD;
    }
    else if (clientFD < *minFD) {
        *minFD = clientFD;
    }
}

void clear_socket(int socketFD, fd_set *master, int *minFD, int *maxFD) {
    int fd = socketFD;
    close(socketFD);
    FD_CLR(socketFD, master);

    if (socketFD == *maxFD) {
        while((fd > *minFD) && (!FD_ISSET(fd, master))) {
            fd--;
        }
        *maxFD = fd;
    }
    else if (socketFD == *minFD) {
        while((fd < *maxFD) && (!FD_ISSET(fd, master))) {
            fd++;
        }
        *minFD = fd;
    }
}

