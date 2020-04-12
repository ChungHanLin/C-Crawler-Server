//
//  log.h
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/6.
//  Copyright © 2020 林重翰. All rights reserved.
//

#ifndef log_h
#define log_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash/hash.h"
#include "hash/dns/cache.h"
#include "hash/db/ip_map.h"
#include "hash/db/url_db.h"
#include "hash/db/fail_db.h"
#include "hash/db/seen_db.h"
#include "hash/db/db_struct.h"

#define BUFFER_SIZE 2048

void load_log_file(char *account, SeenDB *seen_db, FailDB *fail_db, Map *map, Dns *cache);

void load_success_link(char *account, SeenDB *seen_db, Map *map, Dns *cache);

void load_fail_link(char *account, FailDB *fail_db, Map *map, Dns *cache);

void load_collect_link(char *account, SeenDB *seen_db, FailDB *fail_db, Map *map, Dns *cache);

#endif /* log_h */
