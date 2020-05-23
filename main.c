//
//  main.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include <stdio.h>
#include <signal.h>

#include "struct.h"
#include "socket.h"
#include "log.h"
#include "hash/dns/cache.h"
#include "hash/db/ip_map.h"
#include "hash/db/db_struct.h"

void sigint_handler(int sig);

void *thread_handler(void *socketFD);

int create_client_index(int socketFD, char *account);

void remove_client_index(int socketFD);

char *check_connected_account();

fd_set master, read_fds;
int minFD, maxFD, clientNum;
unsigned char url[1024];
Client client[BACKLOG_NUM];
Map map;
Dns cache;
SeenDB seen_db;
FailDB fail_db;
Record prev;    // Record previous IP map index.
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, const char * argv[]) {
    struct sockaddr_in server_info;
    struct sockaddr_in client_info;
    int serverFD, clientFD;
    char recieve[512];
    char response[128];
    
    pthread_t client_thread;
    
    signal(SIGINT, sigint_handler);

    // Initialize In-memory DB
    init_cache(&cache);
    init_ip_map(&map);
    create_seen_db(&seen_db);
    create_fail_db(&fail_db);
    
    // Recover db data, if needed
    recover_cache(&cache);
    recover_route(&map);
    recover_url_db(&map);
    recover_seen_db(&seen_db);
    recover_fail_db(&fail_db);

    
    // Given entrance url
    // 表示 server 從未被啟動過
    if (seen_db.size == 0) {
        printf("Please enter an url: ");
        scanf(" %s", url);
    }

    // create socket server information
    serverFD = create_socket();

    init_socket(&server_info, NULL);

    bind_addr(serverFD, &server_info);

    listen_socket(serverFD);

    init_selector(serverFD, &master);

    int i;
    minFD = maxFD = serverFD;
    clientNum = 0;
    
    // Make server can execute all the time
    while(true) {
        read_fds = master;
        select_client(maxFD, &read_fds);
        for (i = minFD; i <= maxFD; i++) {
            if (FD_ISSET(i, &read_fds)) {
                clientFD = accept_client(serverFD, &client_info);
                insert_fd_set(clientFD, &master, &minFD, &maxFD);
                
                // Using pthread to handle mutiple client.
                pthread_create(&client_thread, NULL, thread_handler, (void *) clientFD);
            }
        }
    }

    close(serverFD);
  
    return 0;
}

void sigint_handler(int sig) {
    char c;

    signal(sig, SIG_IGN);
    printf("Do you want to close the server ? [y/n] ");
    scanf(" %c", &c);
    if (c == 'y' || c == 'Y') {
        /* Backup current data in the DB */
        fprintf(stderr, "Backup the data in the DB...\n");
        fprintf(stderr, "Seen DB... ");
        backup_seen_db(&seen_db);
        fprintf(stderr, "OK\n");

        fprintf(stderr, "Fail DB... ");
        backup_fail_db(&fail_db);
        fprintf(stderr, "OK\n");

        fprintf(stderr, "Url DB... ");
        backup_route(&map);
        fprintf(stderr, "OK\n");

        fprintf(stderr, "DNS Cache... ");
        backup_cache(&cache);
        fprintf(stderr, "OK\n");

        fprintf(stderr, "Done!\n");
        exit(EXIT_SUCCESS);
    }
    else {
        signal(SIGINT, sigint_handler);
    }
}

void *thread_handler(void *fd) {
    int socketFD = (int) fd;
    int client_index;
    int recv_num;
    char recv_buffer[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    bool r = false;

    // pthread_detach can release system resource automatically.
    pthread_detach(pthread_self());

    while (true) {
        bzero(recv_buffer, sizeof(recv_buffer));
        bzero(send_buffer, sizeof(send_buffer));
        
        recv_num = recv(socketFD, &recv_buffer, BUFFER_SIZE, 0);
        if (recv_num < 0) {
            perror("recv");
            pthread_mutex_lock(&lock);
            remove_client_index(socketFD);
            clear_socket(socketFD, &master, &minFD, &maxFD);
            pthread_mutex_unlock(&lock);
            break;
        }
        else if (recv_num == 0) {
            fprintf(stderr, "Client %d disconnect\n", socketFD);
            pthread_mutex_lock(&lock);
            remove_client_index(socketFD);
            clear_socket(socketFD, &master, &minFD, &maxFD);
            pthread_mutex_unlock(&lock);
            break;
        }
        
        // If it was first execution, should create client index in the client list.
        if (!r) {
            client_index = create_client_index(socketFD, recv_buffer);
            r = true;
        }
        if (strcmp("@Done", recv_buffer) == 0) {
            
            // Lock the process whem reading log file.
            pthread_mutex_lock(&lock);
            load_log_file(client[client_index].account, &seen_db, &fail_db, &map, &cache);
            remove_client_index(socketFD);
            clear_socket(socketFD, &master, &minFD, &maxFD);
            pthread_mutex_unlock(&lock);
            
            fprintf(stderr, "Client leave\n");
            
            break;
        }
        // 可能爲 php socket 連線，使用 account name 進行判斷 
        if (strcmp(client[client_index].account, "dashboard") == 0) {
            // 原則上 php socket 強制結束不會有問題
            
            if (strcmp(recv_buffer, "@IP analysis") == 0) {
                // send 分析結果檔案位置給 dashboard client
                // 各個 IP success 與 fail url 數量 (暫時只能做到這個 QQ)
                sprintf(send_buffer, "../C-Crawler-Server/log/analysis.log");
                ip_analysis(map, send_buffer);
                send(socketFD, &send_buffer, BUFFER_SIZE, 0);
                pthread_mutex_lock(&lock);
                remove_client_index(socketFD);
                clear_socket(socketFD, &master, &minFD, &maxFD);
                pthread_mutex_unlock(&lock);
            }
            else {
                // send 當前 connected socket client 資訊
                // dashboard 可根據 client account 至 log 尋找相應的 log file
                char *account = check_connected_account();
                send(socketFD, account, BUFFER_SIZE, 0);
                pthread_mutex_lock(&lock);
                remove_client_index(socketFD);
                clear_socket(socketFD, &master, &minFD, &maxFD);
                pthread_mutex_unlock(&lock);
                free(account);
            }
            break;
        }
        else {
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
                sprintf(path, "../C-Crawler-Server/log/link_queue/%s.log", client[client_index].account);
                
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
    
    pthread_exit(NULL);
}

int create_client_index(int socketFD, char *account) {
    
    // Lock when writing
    pthread_mutex_lock(&lock);
    
    // Add socketFD with account name
    client[clientNum].socketFD = socketFD;
    client[clientNum].account = strdup(account);
    fprintf(stderr, "%s\n", client[clientNum].account);
    clientNum++;
    
    pthread_mutex_unlock(&lock);
    
    return (clientNum - 1);
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

    for(i = 0; i < clientNum; i++) {
        if (client[i].socketFD == socketFD) {
            fprintf(stderr, "%s leave\n", client[i].account);
            for (j = i; j < clientNum - 1; j++) {
                client[j].socketFD = client[j + 1].socketFD;
                free(client[j].account);
                client[j].account = strdup(client[j + 1].account);
            }
            free(client[j].account);
            client[j].account = NULL;
            client[j].socketFD = -1;
            clientNum--;
            break;
        }
    }
    
    fprintf(stderr, "Connected client: %d\n", clientNum);
    for (i = 0; i < 10; i++) {
        if (client[i].account) {
            fprintf(stderr, "%d: %s\n", client[i].socketFD, client[i].account);
        }
    }

}

char *check_connected_account() {
    int i;
    char *account = (char *) malloc(BUFFER_SIZE);

    memset(account, '\0', BUFFER_SIZE);
    for (i = 0 ; i < clientNum; i++) {
        if (strcmp(client[i].account, "dashboard") == 0){
            continue;
        }
        if (account[0] == '\0') {
            sprintf(account, "%s ", client[i].account);
        }
        else {
            sprintf(account, "%s%s ", account, client[i].account);
        }
    }
    
    return account;
}
