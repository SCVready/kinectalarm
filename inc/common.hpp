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
#include <openssl/pem.h>
#include <string.h>

/*******************************************************************
 * Structures
 *******************************************************************/
struct sBase64encode_context
{
    BIO *b64_bio, *mem_bio;
    BUF_MEM *mem_bio_mem_ptr;
};

/*******************************************************************
 * Function declaration
 *******************************************************************/
bool check_dir_exist(char *dir);
int create_dir(char *dir);
int delete_all_files_from_dir(const char *path);

bool both_are_spaces(char lhs, char rhs);
bool allowed_characters(char c);

#endif /* COMMON_H_ */
