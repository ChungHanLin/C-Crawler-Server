//
//  ip_map.h
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/9.
//  Copyright © 2020 林重翰. All rights reserved.
//

#ifndef ip_map_h
#define ip_map_h

#include <stdio.h>

#include "url_db.h"
#include "fail_db.h"
#include "seen_db.h"
#include "../hash.h"
#include "db_struct.h"
#include "../../struct.h"

void init_ip_map(Map *map);

void init_record(Record *prev);

IP *find_route(Map *map, int hash_index, char *addr);

int next_hash_index(Map *map, int hash_index, IP *cursor);

void link_dispatcher(char *write_path, Map *map, Record *prev_cursor);

void ip_analysis(Map map, char *path);

#endif /* ip_map_h */
