/**
 * @author Alejandro Solozabal
 *
 * @file config.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "config.hpp"

/*******************************************************************
 * Funtion definition
 *******************************************************************/
int write_status(struct sDet_conf det_conf,struct sLvw_conf lvw_conf)
{
    struct status_table status;
    status.id              = 0;
    status.tilt            = lvw_conf.tilt;
    status.brightness      = lvw_conf.brightness;
    status.contrast        = lvw_conf.contrast;
    status.det_active      = det_conf.is_active;
    status.lvw_active      = lvw_conf.is_active;
    status.det_threshold   = det_conf.threshold;
    status.det_sensitivity = det_conf.tolerance;
    status.det_num_shots   = det_conf.det_num_shots;
    status.det_fps         = 5;
    status.det_curr_det_id = det_conf.curr_det_num;
    status.lvw_fps         = 5;

    int num_status_entries =-1;
    number_entries_status_table_sqlite_db(&num_status_entries);
    if(!num_status_entries)
        insert_entry_status_table_sqlite_db(&status);
    else
        update_entry_status_table_sqlite_db(&status);

    return 0;
}

int read_status(struct sDet_conf *det_conf,struct sLvw_conf *lvw_conf)
{

    /* Creation status table on SQLite */
    int num_status_entries =-1;
    number_entries_status_table_sqlite_db(&num_status_entries);

    /* If there is any entry, create a default status */
    if(!num_status_entries)
        return -1;

    /* Retrieve status */
    struct status_table status;
    status.id = 0;
    get_entry_status_table_sqlite_db(&status);

    det_conf->is_active      = status.det_active;
    det_conf->threshold      = status.det_threshold;
    det_conf->tolerance      = status.det_sensitivity;
    det_conf->det_num_shots  = status.det_num_shots;
    det_conf->frame_interval = status.det_fps;
    det_conf->curr_det_num   = status.det_curr_det_id;
    lvw_conf->is_active      = status.lvw_active;
    lvw_conf->tilt           = status.tilt;
    lvw_conf->brightness     = status.brightness;
    lvw_conf->contrast       = status.contrast;

    return 0;
}
