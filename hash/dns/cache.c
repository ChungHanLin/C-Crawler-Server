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

unsigned char *extract_domain(unsigned char *url) {
    int i, j, k, cnt, start = 0;
    cnt = 0;
    for(i = 0; i < strlen(url); i++) {
        if (url[i] == '/') {
            cnt++;
            if (cnt == 2) {
                start = i;
            }
        }
        
        if (cnt == 3) {
            break;
        }
        
    }
    
    unsigned char *domain = (unsigned char *) malloc (i - start);
    for (j = start + 1, k = 0; j < i; j++, k++) {
        domain[k] = url[j];
    }
    domain[k] = '\0';
    
    return domain;
}

char *find_dns_cache(Dns *cache, unsigned char *url) {
    char *md5, *ip, *domain;
    int hash_index, compare;
    
    // url 必須先萃取出 domain names  
    domain = extract_domain(url);
    encode_md5(domain, &md5);
    hash_index = hash_func(md5);

    Cache *cursor = cache->map[hash_index];
    
    while (cursor) {
        compare = strcmp(domain, cursor->url);
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
    ip = ask_dns_server(domain);
    if (!ip) {
        return NULL;
    }
    cache->map[hash_index] = insert_dns_cache(cache->map[hash_index], domain, ip);
    
    return ip;
}

char *ask_dns_server(unsigned char *url) {
    char record[16], *ip;
    struct addrinfo hints, *res;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(url, NULL, &hints, &res);
    if (status != 0) {
        return NULL;
    }
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    void *addr = &(ipv4->sin_addr);
    inet_ntop(res->ai_family, addr, record, sizeof(record));
    freeaddrinfo(res);

    ip = strdup(record);

    return ip;
}

Cache *insert_dns_cache(Cache *head, unsigned char *url, char *ip) {
    int compare;
    Cache *cursor, *node;
    Cache *prev = NULL;
    cursor = head;
    
    node = (Cache *) malloc (sizeof(Cache));
    node->url = url;
    node->ip = ip;
    
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
