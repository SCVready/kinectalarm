/*
 * sqlite_db.h
 *
 *  Created on: Apr 26, 2019
 *      Author: scvready
 */

#ifndef SQLITE_DB_H_
#define SQLITE_DB_H_

#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>

int init_sqlite_db();
int deinit_sqlite_db();

int create_det_table_sqlite_db();
int insert_entry_det_table_sqlite_db(unsigned int id, unsigned int datetime, unsigned int duration, char *filename);
int list_entry_det_table_sqlite_db(int id);
int number_entries_det_table_sqlite_db(int *number_entries);
int delete_entry_det_table_sqlite_db(int id);
int delete_all_entries_det_table_sqlite_db();

int test_sqlite();

#endif /* SQLITE_DB_H_ */
