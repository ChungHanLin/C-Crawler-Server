//
//  hash.h
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#ifndef hash_h
#define hash_h

#include <stdio.h>

#include "db/db_struct.h"

#define MAX_HASH 1024

unsigned int hash_func(char *data);

DB *insert(DB *head, char *url);

bool find_db(DB *head, char *url);

#endif /* hash_h */
