/*
 * sqlite_db.cpp
 *
 *  Created on: Apr 26, 2019
 *      Author: scvready
 */

#include "sqlite_db.h"

// Global variables
sqlite3 *db;

int init_sqlite_db()
{
	int rc = sqlite3_open("/etc/kinectalarm/detections.db", &db);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	return 0;
}

int deinit_sqlite_db()
{
    sqlite3_close(db);
	return 0;
}

int create_det_table_sqlite_db()
{
	char *err_msg = 0;

	/* Create SQL statement */
	char *sql = "CREATE TABLE IF NOT EXISTS DETECTIONS("  \
	  "ID			INTEGER		PRIMARY KEY," \
	  "DATE			DATETIME	NOT NULL," \
	  "DURATION		INTEGER		NOT NULL," \
	  "FILENAME		CHAR(50) );";

	/* Execute SQL statement */
	int rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);

	if(rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		return -1;
	}

	return 0;
}

int insert_entry_det_table_sqlite_db(unsigned int id, unsigned int datetime, unsigned int duration, char *filename)
{
	char *err_msg = 0;
	/* Create SQL statement */

	char sql[255];
	sprintf(sql,"INSERT INTO DETECTIONS (ID,DATE,DURATION,FILENAME) "  \
		"VALUES (%u, %u, %u, '%s'); ",id,datetime,duration,filename);


	/* Execute SQL statement */
	int rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		return -1;
	} else {
		fprintf(stdout, "Record created successfully\n");
	}
	return 0;
}

static int number_entries_det_table_sqlite_db_cb(void *param, int argc, char **argv, char **azColName) {
	int *number_entries = (int*) param;

	if(argc>0)
		*number_entries = atoi(argv[0]);

	return 0;
}

int number_entries_det_table_sqlite_db(int *number_entries)
{
	char *err_msg = 0;
	/* Create SQL statement */
	char sql[255];
	sprintf(sql,"SELECT count(*) FROM detections;");

	/* Execute SQL statement */
	int rc = sqlite3_exec(db, sql, number_entries_det_table_sqlite_db_cb, number_entries, &err_msg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		return -1;
	} else {
		fprintf(stdout, "List entry successfully\n");
	}
	return 0;
}

static int list_entry_det_table_sqlite_db_cb(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int list_entry_det_table_sqlite_db(int id)
{
	char *err_msg = 0;
	/* Create SQL statement */
	char sql[255];
	sprintf(sql,"SELECT * FROM DETECTIONS WHERE id=%d;",id);

	/* Execute SQL statement */
	int rc = sqlite3_exec(db, sql, list_entry_det_table_sqlite_db_cb, 0, &err_msg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		return -1;
	} else {
		fprintf(stdout, "List entry successfully\n");
	}
	return 0;
}

int delete_entry_det_table_sqlite_db(int id)
{
	char *err_msg = 0;
	/* Create SQL statement */
	char sql[255];
	sprintf(sql,"DELETE FROM detections WHERE id=%d;",id);

	/* Execute SQL statement */
	int rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		return -1;
	} else {
		fprintf(stdout, "Delete entry successfully\n");
	}
	return 0;
}

int delete_all_entries_det_table_sqlite_db()
{
	char *err_msg = 0;
	/* Create SQL statement */
	char sql[255];
	sprintf(sql,"DELETE FROM detections;");

	/* Execute SQL statement */
	int rc = sqlite3_exec(db, sql, NULL, 0, &err_msg);

	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		return -1;
	} else {
		fprintf(stdout, "Delete all entries successfully\n");
	}
	return 0;
}

/*
#include <time.h>
int test_sqlite()
{
	time_t t;
	time(&t);

	init_sqlite_db();
	create_det_table_sqlite_db();
	insert_entry_det_table_sqlite_db(t,5,"/var/detection/det_1556356447_0.jpeg");
	list_entrie_det_table_sqlite_db(11);
	delete_entrie_det_table_sqlite_db(2);
    return 0;
}
*/
