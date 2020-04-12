//
//  url_db.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "url_db.h"

void create_url_db(UrlDB *db) {
    db->size = 0;
    db->table = (DB **) malloc (sizeof(DB *) * MAX_HASH);
    
    int i;
    for (i = 0; i < MAX_HASH; i++) {
        db->table[i] = NULL;
    }
}

unsigned char *pop_url_db(UrlDB *db) {
    unsigned char *url;
    DB *cursor;
    int i;
    
    for (i = 0; i < MAX_HASH; i++) {
        if (db->table[i]) {
            cursor = db->table[i];
            db->table[i] = db->table[i]->next;
            break;
        }
    }
    url = strdup(cursor->url);
    db->size--;
    free(cursor->url);
    free(cursor);
    
    return url;
}
