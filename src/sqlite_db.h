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
#include <stdint.h>
#include <string.h>

int init_sqlite_db();
int deinit_sqlite_db();

////// DETECTION TABLE ////////

struct det_table{
	uint32_t id;
	uint32_t datetime;
	uint32_t duration;
	char *filename;
};

int create_det_table_sqlite_db();
int insert_entry_det_table_sqlite_db(unsigned int id, unsigned int datetime, unsigned int duration, char *filename);
int get_entry_det_table_sqlite_db(uint32_t id, uint32_t *timestamp, uint32_t *duration, char *filename);
int number_entries_det_table_sqlite_db(int *number_entries);
int delete_entry_det_table_sqlite_db(int id);
int delete_all_entries_det_table_sqlite_db();

////// STATUS TABLE ////////

struct status_table{
	uint32_t id;
	int32_t tilt;
	int32_t brightness;
	int32_t contrast;
	uint32_t det_active;
	uint32_t lvw_active;
	uint32_t det_threshold;
	uint32_t det_sensitivity;
	uint32_t det_num_shots;
	uint32_t det_fps;
	uint32_t det_curr_det_id;
	uint32_t lvw_fps;
};

int create_status_table_sqlite_db();
int number_entries_status_table_sqlite_db(int *number_entries);
int insert_entry_status_table_sqlite_db(struct status_table *status);
int get_entry_status_table_sqlite_db(struct status_table *status);
int update_entry_status_table_sqlite_db(struct status_table *status);

#endif /* SQLITE_DB_H_ */
