//
//  seen_db.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "seen_db.h"

void create_seen_db(SeenDB *db) {
    db->size = 0;
    db->table = (DB **) malloc (sizeof(DB *) * MAX_HASH);
    
    int i;
    for (i = 0; i < MAX_HASH; i++) {
        db->table[i] = NULL;
    }
}

void recover_seen_db(SeenDB *db) {
    int i, hash_index;
    char path[] = "backup/seen_db.log";
    char buffer[BUFFER_SIZE];
    FILE *fp = fopen(path, "r");
    DB *head;

    if (fp) {
        fscanf(fp, "%d", &db->size);

        while (fscanf(fp, "%d %s", &hash_index, buffer) != EOF) {
            db->table[hash_index] = insert(db->table[hash_index], buffer);
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
            if (!db->table[i]) {
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