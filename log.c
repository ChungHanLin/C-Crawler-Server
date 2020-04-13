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
    char success[] = "Recent crawled count: ";
    char fail[] = "Recent fail count: ";
    
    int success_link, fail_link;
    
    success_link = fail_link = 0;
    
    sprintf(path, "log/account/%s.log", account);

    FILE *fp = fopen(path, "r");
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        if (strstr(buffer, success)) {
            success_link = extract_link_count(buffer, success);
            fprintf(stderr, "\nsuccess link: %d\n", success_link);
        }
        else if (strstr(buffer, fail)) {
            fail_link = extract_link_count(buffer, fail);
            fprintf(stderr, "fail link: %d\n", fail_link);
        }
    }
    fclose(fp);
    
    // Insert it into the seen_db
    if (success_link > 0) {
        fprintf(stderr, "Load success file\n");
        load_success_link(account, seen_db, map, cache);
    }
    
    if (fail_link > 0) {
        fprintf(stderr, "Load fail file\n");
        load_fail_link(account, fail_db, map, cache);
    }
    
    load_collect_link(account, seen_db, fail_db, &(*map), &(*cache));
}

void load_success_link(char *account, SeenDB *seen_db, Map *map, Dns *cache) {
    char *ip, *md5;
    char path[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int ip_hash_index, md5_hash_index = 0;
    IP *cursor;
    
    sprintf(path, "log/success/%s.log", account);
    FILE *fp = fopen(path, "r");
    
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        
        // Insert into seen_db
        encode_md5(buffer, &md5);
        md5_hash_index = hash_func(md5);
        seen_db->table[md5_hash_index] = insert(seen_db->table[md5_hash_index], buffer);
        seen_db->size++;

        // 紀錄該 IP 的 success url 情形
        ip = find_dns_cache(cache, buffer);
        if (!ip) {
            continue;
        }
        ip_hash_index = hash_func(ip);
        cursor = find_route(map, ip_hash_index, ip);
        cursor->seen_link_cnt++;
    }
    
    fclose(fp);
}

void load_fail_link(char *account, FailDB *fail_db, Map *map, Dns *cache) {
    char *ip, *md5;
    char path[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int ip_hash_index, hash_index = 0;
    IP *cursor;
    
    sprintf(path, "log/fail/%s.log", account);
    FILE *fp = fopen(path, "r");
    
    while (fgets(buffer, BUFFER_SIZE, fp)) {
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        
        encode_md5(buffer, &md5);
        hash_index = hash_func(md5);
        fail_db->table[hash_index] = insert(fail_db->table[hash_index], buffer);
        fail_db->size++;

        // 紀錄該 IP 的 success url 情形
        ip = find_dns_cache(cache, buffer);
        if (!ip) {
            continue;
        }
        ip_hash_index = hash_func(ip);
        cursor = find_route(map, ip_hash_index, ip);
        cursor->fail_link_cnt++;
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
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        
        encode_md5(buffer, &md5);
        md5_hash_index = hash_func(md5);
        if (find_db(seen_db->table[md5_hash_index], buffer) || find_db(fail_db->table[md5_hash_index], buffer)) {
            continue;
        }
        
        ip = find_dns_cache(cache, buffer);
        if (!ip) {
            continue;
        }
        ip_hash_index = hash_func(ip);
        cursor = find_route(map, ip_hash_index, ip);
        
        cursor->url_db.table[md5_hash_index] = insert(cursor->url_db.table[md5_hash_index], buffer);
        cursor->url_db.size++;
        map->size++;
    }
    
    fclose(fp);
}
