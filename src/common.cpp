/**
 * @author Alejandro Solozabal
 *
 * @file common.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "common.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define BILLION 1000000000

/*******************************************************************
 * Funtion definition
 *******************************************************************/
int create_dir(char* path)
{
    if(mkdir(path, 0770) == -1)
        return -1;
    else
        return 0;
}

bool check_dir_exist(char *path)
{
    DIR* dir;
    if((dir = opendir(path)))
    {
        closedir(dir);
        return true;
    }
    else
        return false;
}

int delete_all_files_from_dir(const char *path)
{
    char command[50];
    sprintf(command, "rm -rf %s/*",path);
    system(command);
    return 0;
}

struct timespec timeAdd(struct timespec t1, struct timespec t2)
{
    long sec = t2.tv_sec + t1.tv_sec;
    long nsec = t2.tv_nsec + t1.tv_nsec;
    if (nsec >= BILLION) {
        nsec -= BILLION;
        sec++;
    }
    return (struct timespec){ .tv_sec = sec, .tv_nsec = nsec };
}

struct timespec timeSub(struct timespec t1, struct timespec t2)
{
    long sec = t1.tv_sec - t2.tv_sec;
    long nsec = t1.tv_nsec - t2.tv_nsec;
    long res = sec*BILLION + nsec;
    return (struct timespec){ .tv_sec = 0, .tv_nsec = res };
}

bool both_are_spaces(char lhs, char rhs)
{
    return (lhs == rhs) && (lhs == ' ');
}

bool allowed_characters(char c)
{
    if(c >= '0' && c <= '9')
        return false;
    if(c >= 'a' && c <= 'z')
        return false;
    if(c >= 'A' && c <= 'Z')
        return false;
    if(c == '-' || c == '+' || c == ' ')
        return false;

    return true;
}

int init_base64encode(struct sBase64encode_context *c)
{
    /* Initialize base64 filter and memory sink BIO */
    c->b64_bio = BIO_new(BIO_f_base64());
    c->mem_bio = BIO_new(BIO_s_mem());

    /* Link the BIOs by creating a filter-sink BIO chain */
    BIO_push(c->b64_bio, c->mem_bio);

    /* No newlines every 64 characters or less */
    BIO_set_flags(c->b64_bio, BIO_FLAGS_BASE64_NO_NL);

    /* Store address of mem_bio's memory structure */
    BIO_get_mem_ptr(c->mem_bio, &c->mem_bio_mem_ptr);
    return 0;
}

int deinit_base64encode(struct sBase64encode_context *c)
{
    /* Destroys all BIOs in chain, starting with b64 (i.e. the 1st one) */
    BIO_free_all(c->b64_bio);

    return 0;
}

char* base64encode(struct sBase64encode_context *c, const void *data, int length)
{
    /* Reset bio structures */
    BIO_reset(c->b64_bio);
    BIO_reset(c->mem_bio);

    /* Records base64 encoded data */
    BIO_write(c->b64_bio, data, length);

    /* Flush data. Necessary for b64 encoding, because of pad characters */
    BIO_flush(c->b64_bio);

    /* Add null terminator */
    BUF_MEM_grow(c->mem_bio_mem_ptr, (*c->mem_bio_mem_ptr).length + 1);
    (*c->mem_bio_mem_ptr).data[(*c->mem_bio_mem_ptr).length] = '\0';

    return (*c->mem_bio_mem_ptr).data;
}

#if 0
char* base64decode(const void *b64_decode_this, int decode_this_many_bytes)
{
    /* Declares two OpenSSL BIOs: a base64 filter and a memory BIO*/
    BIO *b64_bio, *mem_bio;

    char *base64_decoded = (char *) calloc( (decode_this_many_bytes*3)/4+1, sizeof(char));

    /* Initialize our base64 filter BIO */
    b64_bio = BIO_new(BIO_f_base64());
    /* Initialize our memory source BIO */
    mem_bio = BIO_new(BIO_s_mem());

    /* Base64 data saved in source */
    BIO_write(mem_bio, b64_decode_this, decode_this_many_bytes);
    /* Link the BIOs by creating a filter-source BIO chain */
    BIO_push(b64_bio, mem_bio);
    /* Don't require trailing newlines */
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);

    /* Index where the next base64_decoded byte should be written */
    int decoded_byte_index = 0;

    /* Read byte-by-byte */
    while (0 < BIO_read(b64_bio, base64_decoded+decoded_byte_index, 1))
    {
        /* Increment the index until read of BIO decoded data is complete */
        decoded_byte_index++;
    }
    /* Once we're done reading decoded data, BIO_read returns -1 even though there's no error */

    /* Destroys all BIOs in chain, starting with b64 (i.e. the 1st one) */
    BIO_free_all(b64_bio);

    /* Returns base-64 decoded data with trailing null terminator */
    return base64_decoded;
}
#endif
