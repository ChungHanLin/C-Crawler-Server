//
//  hash.c
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#include "hash.h"

unsigned int hash_func(char *data) {
    unsigned int hash_value = 0;
    int i;
    
    for (i = 0 ; i < strlen(data); i++) {
        // Using hash33 -> suggested by Sun Wu
        hash_value = (hash_value << 5) + hash_value + data[i];
    }
    
    return hash_value % MAX_HASH;
}

DB *insert(DB *head, char *url) {
    int compare;
    DB *cursor, *node;
    DB *prev = NULL;
    cursor = head;
    
    node = (DB *) malloc (sizeof(DB));
    node->url = (unsigned char *) strdup (url);
    
    // new list
    if (!cursor) {
        node->next = NULL;
        head = node;
    }
    else {
        
        while (cursor) {
            if ((compare = strcmp(url, cursor->url)) < 0) {
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

bool find_db(DB *head, char *url) {
    DB *cursor = head;
    int compare;
    
    while (cursor) {
        compare = strcmp(url, cursor->url);
        if (compare < 0) {
            return false;
        }
        else if (compare == 0) {
            return true;
        }
        else {
            cursor = cursor->next;
        }
    }
    
    return false;
}
