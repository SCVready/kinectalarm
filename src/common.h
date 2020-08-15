/**
 * @author Alejandro Solozabal
 *
 * @file common.h
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

bool check_dir_exist(char *dir);
int create_dir(char *dir);
int delete_all_files_from_dir(char *path);

struct timespec timeAdd(struct timespec t1, struct timespec t2);
struct timespec timeSub(struct timespec t1, struct timespec t2);

bool both_are_spaces(char lhs, char rhs);
bool allowed_characters(char c);

#include <openssl/pem.h>
#include <string.h>


struct sBase64encode_context{
    BIO *b64_bio, *mem_bio;
    BUF_MEM *mem_bio_mem_ptr;
};
int init_base64encode(struct sBase64encode_context *c);
int deinit_base64encode(struct sBase64encode_context *c);
char* base64encode(struct sBase64encode_context *c, const void *data, int length);
char* base64decode (const void *b64_decode_this, int decode_this_many_bytes);

#endif /* COMMON_H_ */
