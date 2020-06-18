//
//  seen_db.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "seen_db.h"

static inline int parseInt(char *string) {
    int i, sum = 0;
    for (i = 0; string[i] != '\0'; i++) {
        sum = sum * 10 + (string[i] - '0');
    }

    return sum;
}

void create_seen_db(SeenDB *db) {
    db->size = 0;
    db->table = (DB **) malloc (sizeof(DB *) * MAX_HASH);
    
    int i;
    for (i = 0; i < MAX_HASH; i++) {
        db->table[i] = NULL;
    }
}

void recover_seen_db(SeenDB *db) {
    int i, hash_index, spaceCnt = 0, moveCnt = 0;
    char path[] = "backup/seen_db.log";
    char *ptr;
    char buffer[BUFFER_SIZE], url[BUFFER_SIZE];
    FILE *fp = fopen(path, "r");
    DB *head;

    if (fp) {
        fscanf(fp, "%d", &db->size);

        while (fgets(buffer, BUFFER_SIZE, fp)) {
            ptr = buffer;
            moveCnt = 0;
            spaceCnt = 0;
            for (i = 0; buffer[i] != '\n' && buffer[i] != '\0'; i++, moveCnt++) {
                if (buffer[i] == ' ') {
                    spaceCnt++;
                    // hash_index
                    if (spaceCnt == 1) {
                        buffer[i] = '\0';
                        hash_index = parseInt(ptr);
                        ptr = ptr + moveCnt + 1;
                        moveCnt;
                    }
                }
            }
            buffer[i] = '\0';
            strcpy(url, ptr);
            db->table[hash_index] = insert(db->table[hash_index], url);
        }

        fclose(fp);
    }   
}

void backup_seen_db(SeenDB *db) {
    int i;
    char path[] = "backup/seen_db.log";
    FILE *fp = fopen(path, "w");
    DB *cursor;

    if (fp) {
        fprintf(fp, "%d\n", db->size);
        for (i = 0; i < MAX_HASH; i++) {
            if (db->table[i]) {
                cursor = db->table[i];
                while (cursor) {
                    fprintf(fp, "%d %s\n", i, cursor->url);
                    cursor = cursor->next;
                }
            }
        } 
        fclose(fp);
    }
    
}

/* Back log file format */
/* Seen DB size: xxx */
/* hash_index -> url */