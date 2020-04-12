//
//  cache.h
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#ifndef cache_h
#define cache_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "../hash.h"
#include "../../md5.h"
#include "../../struct.h"

typedef struct cache {
    char *ip;
    unsigned char *url;
    struct cache *next;
}Cache;

typedef struct dns {
    Cache **map;
}Dns;

void init_cache(Dns *cache);

char *find_dns_cache(Dns *cache, unsigned char *url);

char *ask_dns_server(unsigned char *url);

Cache *insert_dns_cache(Cache *head, unsigned char *url, char *ip);

#endif /* cache_h */
