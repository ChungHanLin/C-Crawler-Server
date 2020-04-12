//
//  main.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include <stdio.h>

#include "struct.h"
#include "socket.h"
#include "log.h"
#include "hash/dns/cache.h"
#include "hash/db/ip_map.h"
#include "hash/db/db_struct.h"

void *thread_handler(void *socketFD);

bool create_client_index(int socketFD);

int find_client_index(int socketFD);

void remove_client_index(int socketFD);

void check_connected_account();

fd_set master, read_fds;
int minFD, maxFD, clientNum;
unsigned char url[256];
Client client[BACKLOG_NUM];
Map map;
Dns cache;
SeenDB seen_db;
FailDB fail_db;
Record prev;    // 記錄前一次 ip 所指的到的位置

int main(int argc, const char * argv[]) {
    struct sockaddr_in server_info;
    struct sockaddr_in client_info;
    int serverFD, clientFD;
    char recieve[512];
    char response[128];
    
    pthread_t client_thread;
    
    // Given entrance url
    printf("Please enter an url: ");
    scanf("%s", url);
    
    init_cache(&cache);
    init_ip_map(&map);
    create_seen_db(&seen_db);
    create_fail_db(&fail_db);
    
    // create socket server information
    serverFD = create_socket();

    init_socket(&server_info, NULL);

    bind_addr(serverFD, &server_info);

    listen_socket(serverFD);

    init_selector(serverFD, &master);

    int i;
    minFD = maxFD = serverFD;
    clientNum = 0;
    while(true) {
        read_fds = master;
        select_client(maxFD, &read_fds);
        for (i = minFD; i <= maxFD; i++) {
            if (FD_ISSET(i, &read_fds)) {
                clientFD = accept_client(serverFD, &client_info);
                insert_fd_set(clientFD, &master, &minFD, &maxFD);
                pthread_create(&client_thread, NULL, thread_handler, (void *) clientFD);
            }
        }
    }

    close(serverFD);
  
    return 0;
}

void *thread_handler(void *fd) {
    int socketFD = (int) fd;
    int client_index;
    int recv_num;
    char recv_buffer[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_detach(pthread_self());
    
    if (create_client_index(socketFD)) {
        client_index = find_client_index(socketFD);
        while (true) {
            bzero(recv_buffer, sizeof(recv_buffer));
            bzero(send_buffer, sizeof(send_buffer));
            
            recv_num = recv(socketFD, &recv_buffer, BUFFER_SIZE, 0);
            if (recv_num < 0) {
                perror("recv");
                break;
            }
            else if (recv_num == 0) {
                fprintf(stderr, "Client %d disconnect", socketFD);
                break;
            }
            
            
            if (strcmp("@Done", recv_buffer) == 0) {
                // log file 讀取
                // 所有輸入動作皆會在此完成
                pthread_mutex_lock(&lock);
                load_log_file(client[client_index].account, &seen_db, &fail_db, &map, &cache);
                clear_socket(socketFD, &master, &minFD, &maxFD);
                pthread_mutex_unlock(&lock);
                
                fprintf(stderr, "Client leave\n");
                
                break;
            }
            
            // 判斷是否為第一次執行
            if (map.size < BATCH_NUM) {
                // 為第一次執行 (send 通知 client 爬 link)
                init_record(&prev);
                sprintf(send_buffer, "@Initialize: %s", url);
                send(socketFD, &send_buffer, BUFFER_SIZE, 0);
            }
            else {
                // 通知 client 檔案位置
                bzero(path, sizeof(path));
                sprintf(path, "log/link_queue/%s.log", client[client_index].account);
                
                // dispatch link (避免同時分配時分配到同樣的 link，因此 lock 確保資料唯一性)
                pthread_mutex_lock(&lock);
                link_dispatcher(path, &map, &prev);
                pthread_mutex_unlock(&lock);
                
                sprintf(send_buffer, "@Path: %s", path);
                if (send(socketFD, &send_buffer, BUFFER_SIZE, 0) < 0) {
                    perror("send");
                    break;
                }
            }
        }
    }

    remove_client_index(socketFD);
    pthread_exit(NULL);
}

bool create_client_index(int socketFD) {
    char recv_buffer[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    
    // First recv msg is client`s account name
    bzero(recv_buffer, sizeof(recv_buffer));
    if (recv(socketFD, &recv_buffer, BUFFER_SIZE, 0) < 0) {
        perror("account recv");
        return false;
    }
    
    sprintf(send_buffer, "Welcome %s", recv_buffer);
    if (send(socketFD, &send_buffer, BUFFER_SIZE, 0) < 0) {
        perror("send");
        return false;
    }
    
    // Lock when writing
    pthread_mutex_lock(&lock);
    
    // Add socketFD with account name
    client[clientNum].socketFD = socketFD;
    client[clientNum].account = strdup(recv_buffer);
    clientNum++;
    
    pthread_mutex_unlock(&lock);
    
    return true;
}

int find_client_index(int socketFD) {
    int i;
    for (i = 0; i < clientNum; i++) {
        if (client[i].socketFD == socketFD) {
            return i;
        }
    }
    
    return -1;
}

void remove_client_index(int socketFD) {
    int i, j;
    pthread_mutex_t lock;
    
    pthread_mutex_lock(&lock);
    for(i = 0; i < clientNum; i++) {
        if (client[i].socketFD == socketFD) {
            for (j = i; j < clientNum - 1; j++) {
                client[j].socketFD = client[j + 1].socketFD;
                free(client[j].account);
                client[j].account = strdup(client[j + 1].account);
            }
            free(client[j].account);
            client[j].socketFD = -1;
            clientNum--;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
}

void check_connected_account() {
    int i;
    
    for (i = 0 ; i < clientNum; i++) {
        fprintf(stderr, "FD: %d  %s\n", client[i].socketFD, client[i].account);
    }
}
