#include "../../../inc/common.hpp"

bool check_dir_exist(char *dir)
{
    return true;
}

int create_dir(char *dir)
{
    return 0;
}

int delete_all_files_from_dir(const char *path)
{
    return 0;
}

bool both_are_spaces(char lhs, char rhs)
{
    return true;
}

bool allowed_characters(char c)
{
    return true;
}

int init_base64encode(struct sBase64encode_context *c)
{
    return 0;
}

int deinit_base64encode(struct sBase64encode_context *c)
{
    return 0;
}

const char* base64encode(struct sBase64encode_context *c, const void *data, int length)
{
    return "test";
}

const char* base64decode (const void *b64_decode_this, int decode_this_many_bytes)
{
    return "test";
}
