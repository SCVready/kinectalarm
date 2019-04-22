/**
  * @file common.h
  * @author Alejandro Solozabal
  */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

bool check_dir_exist(char *dir);
int create_dir(char *dir);

struct timespec timeAdd(struct timespec t1, struct timespec t2);
struct timespec timeSub(struct timespec t1, struct timespec t2);

bool both_are_spaces(char lhs, char rhs);
bool allowed_characters(char c);

#include <openssl/pem.h>
#include <string.h>

char *base64encode (const void *b64_encode_this, int encode_this_many_bytes);
char *base64decode (const void *b64_decode_this, int decode_this_many_bytes);

#endif /* COMMON_H_ */
