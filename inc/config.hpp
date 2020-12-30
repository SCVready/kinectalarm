/**
 * @author Alejandro Solozabal
 *
 * @file config.hpp
 *
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_


/*******************************************************************
 * Includes
 *******************************************************************/
#include <stdio.h>
#include <string.h>
#include <string>

#include "global_parameters.hpp"
#include "alarm.hpp"
#include "sqlite_db.hpp"

/*******************************************************************
 * Function declaration
 *******************************************************************/
int write_status(struct sDet_conf det_conf,struct sLvw_conf lvw_conf);
int read_status(struct sDet_conf *det_conf,struct sLvw_conf *lvw_conf);

#endif /* SRC_CONFIG_H_ */
