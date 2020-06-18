//
//  url_db.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "url_db.h"

static inline int parseInt(char *string) {
    int i, sum = 0;
    for (i = 0; string[i] != '\0'; i++) {
        sum = sum * 10 + (string[i] - '0');
    }

    return sum;
}


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
    int spaceCnt = 0, moveCnt = 0;
    char *ptr;
    char path[] = "backup/url_db.log";
    char buffer[BUFFER_SIZE];
    char prev_addr[BUFFER_SIZE];
    char cur_addr[BUFFER_SIZE];
    char url[BUFFER_SIZE];
    IP *cursor;

    FILE *fp = fopen(path, "r");
    bzero(prev_addr, sizeof(prev_addr));
                                                                                                                                                                                                                                                                                                              
    if (fp) {
        // 若網址包含內容，極有可能會出現 space，因此 fscanf 不適用
        while (fgets(buffer, BUFFER_SIZE, fp)) {
            ptr = buffer;
            moveCnt = 0;
            spaceCnt = 0;
            for (i = 0; buffer[i] != '\n' && buffer[i] != '\0'; i++, moveCnt++) {
                if (buffer[i] == ' ') {
                    spaceCnt++;
                    // map_hash_index
                    if (spaceCnt == 1) {
                        buffer[i] = '\0';
                        map_hash_index = parseInt(ptr);
                        ptr = ptr + moveCnt + 1;
                        moveCnt = 0;
                    }
                    // cur_addr
                    else if (spaceCnt == 2) {
                        buffer[i] = '\0';
                        strcpy(cur_addr, ptr);
                        ptr = ptr + moveCnt;
                        moveCnt = 0;
                    }
                    // url_hash_index
                    else if (spaceCnt == 3) {
                        buffer[i] = '\0';
                        url_hash_index = parseInt(ptr);
                        ptr = ptr + moveCnt;
                        moveCnt = 0;
                    }
                }         
            }
            buffer[i] = '\0';
            strcpy(url, ptr);

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
