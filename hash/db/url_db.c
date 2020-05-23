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



void recover_url_db(Map *map) {
    int i, map_hash_index, url_hash_index;
    char path[] = "backup/url_db.log";
    char prev_addr[BUFFER_SIZE];
    char cur_addr[BUFFER_SIZE];
    char url[BUFFER_SIZE];
    IP *cursor;

    FILE *fp = fopen(path, "r");
    bzero(prev_addr, sizeof(prev_addr));

    if (fp) {
        while (fscanf(fp, "%d %s %d %s", &map_hash_index, cur_addr, &url_hash_index, url) != EOF) {
            if (strcmp(prev_addr, cur_addr) != 0) {
                cursor = find_route(map, map_hash_index, cur_addr);
            }
            cursor->url_db.table[url_hash_index] = insert(cursor->url_db.table[url_hash_index], url);
            cursor->url_db.size++;
        }

        fclose(fp);
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
