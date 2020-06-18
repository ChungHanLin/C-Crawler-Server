//
//  ip_map.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/9.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "ip_map.h"

void init_ip_map(Map *map) {
    int i;
    map->route = (IP **) malloc (sizeof(IP *) * MAX_HASH);
    map->size = 0;
    
    for (i = 0; i < MAX_HASH; i++) {
        map->route[i] = NULL;
    }
}

void init_record(Record *prev) {
    prev->cursor = NULL;
    prev->hash_index = -1;
}

void backup_route(Map *map) {
    int i, j;
    char ip_path[] = "backup/ip_map.log";
    char url_path[] = "backup/url_db.log";

    FILE *fp_map = fopen(ip_path, "w");
    FILE *fp_url = fopen(url_path, "w");
    IP *cursor;
    DB *ptr;

    if (fp_map) {
        fprintf(fp_map, "%d\n", map->size);
        for (i = 0; i < MAX_HASH; i++) {
            if (map->route[i]) {
                cursor = map->route[i];
                while (cursor) {
                    fprintf(fp_map, "%d %s %d %d\n", i, cursor->addr, cursor->seen_link_cnt, cursor->fail_link_cnt);
                    for (j = 0; j < MAX_HASH; j++) {
                        if (cursor->url_db.table[j]) {
                            ptr = cursor->url_db.table[j];
                            while (ptr) {
                                fprintf(fp_url, "%d %s %d %s\n", i, cursor->addr, j, ptr->url);
                                ptr = ptr->next;
                            }
                        }
                    }
                    cursor = cursor->next;
                }
            }
        }
    }
}

void recover_route(Map *map) {
    int i, hash_index, seen_cnt, fail_cnt;
    char path[] = "backup/ip_map.log";
    char addr[BUFFER_SIZE];
    FILE *fp = fopen(path, "r");

    if (fp) {
        fscanf(fp, "%d", &map->size);

        while (fscanf(fp, "%d %s %d %d", &hash_index, addr, &seen_cnt, &fail_cnt) != EOF) {
            map->route[hash_index] = insert_route(map->route[hash_index], addr, seen_cnt, fail_cnt);
        }

        fclose(fp);
    }
}

IP *insert_route(IP *head, char *addr, int seen_cnt, int fail_cnt) {
    int compare;
    IP *cursor, *node;
    IP *prev = NULL;
    cursor = head;

    node = (IP *) malloc (sizeof(IP));
    node->addr = strdup (addr);
    node->seen_link_cnt = seen_cnt;
    node->fail_link_cnt = fail_cnt;
    create_url_db(&node->url_db);

    if (!cursor) {
        node->next = NULL;
        head = node;
    }
    else {
        
        while (cursor) {
            if ((compare = strcmp(addr, cursor->addr)) < 0) {
                break;
            }
            prev = cursor;
            cursor = cursor->next;
        }
        // Insert at head
        if (cursor == head) {
            node->next = head;
            head = node;
        }
        else {
            node->next = cursor;
            prev->next = node;
        }
    }
    
    return head;
}

IP *find_route(Map *map, int hash_index, char *addr) {
    IP *head, *cursor, *node, *prev = NULL;
    int compare;
    
    head = cursor = map->route[hash_index];
    
    if (head) {
        while (cursor) {
            compare = strcmp(addr, cursor->addr);
            if (compare > 0) {
                prev = cursor;
                cursor = cursor->next;
            }
            else if (compare == 0) {
                return cursor;
            }
            else {
                break;
            }
        }
    }

    // 建立新的 route node
    node = (IP *) malloc (sizeof(IP));
    node->addr = strdup(addr);
    node->url_db.size = 0;
    node->seen_link_cnt = 0;
    node->fail_link_cnt = 0;
    create_url_db(&node->url_db);
    if (!head) {
        node->next = NULL;
        map->route[hash_index] = node;
    }
    else if (cursor == head) {
        node->next = head;
        map->route[hash_index] = node;
    }
    else if (prev) {
        node->next = cursor;
        prev->next = node;
    }
    
    return node;
}

int next_hash_index(Map *map, int hash_index, IP *cursor) {
    int begin;
    if (hash_index == -1) {
        begin = 0;
    }
    else {
        begin = hash_index + 1;
    }
    
    for (;; begin++) {
        if (begin == MAX_HASH) {
            begin = 0;
        }
        
        if (map->route[begin]) {
            return begin;
        }
    }
    
    return -1;
}

void link_dispatcher(char *write_path, Map *map, Record *prev) {
    unsigned char *url;
    int i, collect_cnt = 0;
    int hash_index = prev->hash_index;
    IP *ptr, *cursor = prev->cursor;
    FILE *fp = fopen(write_path, "w");
    
    while (collect_cnt < BATCH_NUM) {
        if (!cursor || !cursor->next) {
            hash_index = next_hash_index(map, hash_index, cursor);
            // 指向新 hash_index 的 head
            cursor = map->route[hash_index];
        }
        else {
            // 指向前一次結果的 next
            cursor = cursor->next;
        }
        if (cursor->url_db.size == 0) {
            continue;
        }
        url = pop_url_db(&cursor->url_db);
        fprintf(fp, "%s\n", url);
        free(url);
        collect_cnt++;
        map->size--;
    }
    fclose(fp);
    // 紀錄最後一個 cursor 指向位置
    prev->hash_index = hash_index;
    prev->cursor = cursor;
}

void ip_analysis(Map map, char *path) {
    FILE *fp = fopen(path, "w");
    IP *cursor;
    int i;
    fprintf(stderr, "IP anlysis... ");
    for (i = 0; i < MAX_HASH; i++) {
        if (map.route[i]) {
            cursor = map.route[i];
            while (cursor) {
                // if (cursor->seen_link_cnt == 0 && cursor->fail_link_cnt == 0) {
                //     continue;
                // }
                fprintf(fp, "%s %d %d\n", cursor->addr, cursor->seen_link_cnt, cursor->fail_link_cnt);
                cursor = cursor->next;
            }
        }
    }
    fprintf(stderr, "Done!\n");

    fclose(fp);
}