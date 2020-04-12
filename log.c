//
//  log.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/6.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "log.h"

static inline int extract_link_count(char *buffer, char *pattern) {
    int i;
    int count = 0;
    
    for (i = strlen(pattern); buffer[i] != '\n' && buffer[i] != '\0'; i++) {
        count = count * 10 + (buffer[i] - '0');
    }
    
    return count;
}

inline void load_log_file(char *account, SeenDB *seen_db, FailDB *fail_db, Map *map, Dns *cache) {
    char buffer[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    char success[] = "Recent crawled cnt: ";
    char fail[] = "Recent failed cnt: ";
    
    int success_link, fail_link;
    
    success_link = fail_link = 0;
    
    // path 要改
    sprintf(path, "log/account/%s.log", account);
    
    FILE *fp = fopen(path, "r");
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        if (strstr(buffer, success)) {
            success_link = extract_link_count(buffer, success);
        }
        else if (strstr(buffer, fail)) {
            fail_link = extract_link_count(buffer, fail);
        }
    }
    fclose(fp);
    
    // Insert it into the seen_db
    if (success_link > 0) {
        load_success_link(account, seen_db, map, cache);
    }
    
    if (fail_link > 0) {
        load_fail_link(account, fail_db, map, cache);
    }
    
    load_collect_link(account, seen_db, fail_db, map, cache);
}

void load_success_link(char *account, SeenDB *seen_db, Map *map, Dns *cache) {
    char *md5;
    char path[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int hash_index = 0;
    IP *cursor;
    
    sprintf(path, "log/success/%s.log", account);
    FILE *fp = fopen(path, "r");
    
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        if (buffer[strlen(buffer)] == '\n') {
            buffer[strlen(buffer)] = '\0';
        }
        
        encode_md5(buffer, &md5);
        hash_index = hash_func(md5);
        seen_db->table[hash_index] = insert(seen_db->table[hash_index], buffer);
        seen_db->size++;
    }
    
    fclose(fp);
}

void load_fail_link(char *account, FailDB *fail_db, Map *map, Dns *cache) {
    char *ip, *md5;
    char path[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int hash_index = 0;
    IP *cursor;
    
    sprintf(path, "log/fail/%s.log", account);
    FILE *fp = fopen(path, "r");
    
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        if (buffer[strlen(buffer)] == '\n') {
            buffer[strlen(buffer)] = '\0';
        }
        
        encode_md5(buffer, &md5);
        hash_index = hash_func(md5);
        fail_db->table[hash_index] = insert(fail_db->table[hash_index], buffer);
        fail_db->size++;
    }
    
    fclose(fp);
}

void load_collect_link(char *account, SeenDB *seen_db, FailDB *fail_db, Map *map, Dns *cache) {
    char *ip, *md5;
    char path[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int ip_hash_index = 0, md5_hash_index = 0;
    IP *cursor;
    
    sprintf(path, "log/collect/%s.log", account);
    FILE *fp = fopen(path, "r");
    
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        if (buffer[strlen(buffer)] == '\n') {
            buffer[strlen(buffer)] = '\0';
        }
        
        encode_md5(buffer, &md5);
        md5_hash_index = hash_func(md5);
        if (find_db(seen_db->table[md5_hash_index], buffer) || find_db(fail_db->table[md5_hash_index], buffer)) {
            continue;
        }
        
        ip = find_dns_cache(cache, buffer);
        ip_hash_index = hash_func(ip);
        cursor = find_route(map->route[ip_hash_index], ip);
        free(ip);
        
        cursor->url_db.table[md5_hash_index] = insert(cursor->url_db.table[md5_hash_index], buffer);
        cursor->url_db.size++;
        map->size++;
    }
    
    fclose(fp);
}
