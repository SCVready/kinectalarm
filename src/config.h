/*
 * config.h
 *
 *  Created on: 22 ene. 2019
 *      Author: asolo
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <stdio.h>
#include <string.h>
#include <string>

#include "global_parameters.h"
#include "cAlarm.h"
#include "sqlite_db.h"


int write_status(struct sDet_conf det_conf,struct sLvw_conf lvw_conf);
int read_status(struct sDet_conf *det_conf,struct sLvw_conf *lvw_conf);

#endif /* SRC_CONFIG_H_ */
