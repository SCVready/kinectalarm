/**
 * @author Alejandro Solozabal
 *
 * @file common.hpp
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

/*******************************************************************
 * Function declaration
 *******************************************************************/
int CreateDirectory(const char *dir);
int DeleteAllFilesFromDirectory(const char *path);

bool BothAreSpaces(char lhs, char rhs);
bool AllowedCharacters(char c);

#endif /* COMMON_H_ */
