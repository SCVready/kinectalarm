#include "../../../inc/sqlite_db.hpp"

int init_sqlite_db()
{
    return 0;
}

int deinit_sqlite_db()
{
    return 0;
}

int create_det_table_sqlite_db()
{
    return 0;
}

int insert_entry_det_table_sqlite_db(unsigned int id, unsigned int datetime, unsigned int duration, char *filename_img, char *filename_vid)
{
    return 0;
}

int get_entry_det_table_sqlite_db(uint32_t id, uint32_t *timestamp, uint32_t *duration, char *filename)
{
    return 0;
}

int number_entries_det_table_sqlite_db(int *number_entries)
{
    return 0;
}

int delete_entry_det_table_sqlite_db(int id)
{
    return 0;
}

int delete_all_entries_det_table_sqlite_db()
{
    return 0;
}

int create_status_table_sqlite_db()
{
    return 0;
}

int number_entries_status_table_sqlite_db(int *number_entries)
{
    return 0;
}

int insert_entry_status_table_sqlite_db(struct status_table *status)
{
    return 0;
}

int get_entry_status_table_sqlite_db(struct status_table *status)
{
    return 0;
}

int update_entry_status_table_sqlite_db(struct status_table *status)
{
    return 0;
}
