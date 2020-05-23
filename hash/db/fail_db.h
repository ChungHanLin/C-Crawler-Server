//
//  fail_db.h
//  C-Crawler-Server
//
//  Created by 林重翰 on 2020/4/5.
//  Copyright © 2020 林重翰. All rights reserved.
//

#ifndef fail_db_h
#define fail_db_h

#include "../hash.h"
#include "db_struct.h"

void create_fail_db(FailDB *db);

void recover_fail_db(FailDB *db);

void backup_fail_db(FailDB *db);

#endif /* fail_db_h */
