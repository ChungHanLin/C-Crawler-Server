//
//  cache.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "cache.h"

void init_cache(Dns *cache) {
    int i;
    
    cache->map = (Cache **) malloc (sizeof(Cache *) * MAX_HASH);
    for (i = 0; i < MAX_HASH; i++) {
        cache->map[i] = NULL;
    }
}

char *find_dns_cache(Dns *cache, unsigned char *url) {
    char *md5, *ip;
    int hash_index, compare;
    
    // url 必須先萃取出 domain names
    
    encode_md5(url, &md5);
    hash_index = hash_func(md5);
    
    Cache *cursor = cache->map[hash_index];
    
    while (cursor) {
        compare = strcmp(url, cursor->url);
        if (compare > 0) {
            cursor = cursor->next;
        }
        else if (compare == 0) {
            return cursor->ip;
        }
        else {
            break;
        }
    }
    ip = ask_dns_server(url);
    cache->map[hash_index] = insert_dns_cache(cache->map[hash_index], url, ip);
    
    return ip;
}

char *ask_dns_server(unsigned char *url) {
    char *ip = (char *) malloc(16);
    CURLcode res;
    CURL *curl = curl_easy_init();
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip);
    }
    
    curl_easy_cleanup(curl);
    
    return ip;
}

Cache *insert_dns_cache(Cache *head, unsigned char *url, char *ip) {
    int compare;
    Cache *cursor, *node;
    Cache *prev = NULL;
    cursor = head;
    
    node = (Cache *) malloc (sizeof(Cache));
    node->url = strdup(url);
    node->ip = strdup(ip);
    
    if (!cursor) {
        node->next = NULL;
        head = node;
    }
    else {
        while (cursor != NULL) {
            if ((compare = strcmp(cursor->url, url)) < 0) {
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
