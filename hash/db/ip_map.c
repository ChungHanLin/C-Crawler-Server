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

    for (i = 0; i < MAX_HASH; i++) {
        if (map.route[i]) {
            cursor = map.route[i];
            while (cursor) {
                fprintf(fp, "%s %d %d\n", cursor->addr, cursor->seen_link_cnt, cursor->fail_link_cnt);
                cursor = cursor->next;
            }
        }
    }

    fclose(fp);
}