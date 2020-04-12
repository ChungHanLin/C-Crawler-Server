//
//  db_struct.h
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#ifndef db_struct_h
#define db_struct_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../struct.h"

// Using linked-list hash instead of linear-probing
typedef struct db {
    unsigned char *url;
    struct db *next;
}DB;

// Record the link that has been visited.
// Using "Hash" data strcture for storing data.
typedef struct seenDB {
    DB **table;
    unsigned int size;
}SeenDB;

// Link queue -> record depth as the end condition of the crawler
// Using "Hash" data structure for storing data.
typedef struct urlDB {
    DB **table;
    unsigned int size;
}UrlDB;

// Stroing access failed url.
typedef struct failDB {
    DB **table;
    unsigned int size;
}FailDB;

// IP hashing
typedef struct ip {
    char *addr;
    UrlDB url_db;
    int seen_link_cnt;
    int fail_link_cnt;
    struct ip *next;
}IP;

typedef struct map {
    IP **route;
    int size;
}Map;

typedef struct record {
    IP *cursor;
    int hash_index;
}Record;

#endif /* struct_h */
