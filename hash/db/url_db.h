//
//  url_db.h
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#ifndef url_db_h
#define url_db_h

#include "../hash.h"
#include "ip_map.h"
#include "db_struct.h"

void create_url_db(UrlDB *db);

void recover_url_db(Map *map);

unsigned char *pop_url_db(UrlDB *db);

#endif /* url_db_h */
