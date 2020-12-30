#include "../../../inc/redis_db.hpp"

int init_redis_db()
{
    return 0;
}

int deinit_redis_db()
{
    return 0;
}

int redis_get_int(const char *key, int *value)
{
    return 0;
}

int redis_get_char(const char *key, char **value)
{
    return 0;
}

int redis_set_int(const char *key, int value)
{
    return 0;
}

int redis_set_char(const char *key, const char *value)
{
    return 0;
}

int redis_setex_int(const char *key, int time, int value)
{
    return 0;
}

int redis_publish(const char *channel, const char *message)
{
    return 0;
}

int init_async_redis_db()
{
    return 0;
}

int async_redis_subscribe(const char * channel, void callback(redisAsyncContext *c, void *reply, void *privdata),void * data)
{
    return 0;
}

int async_redis_event_dispatch()
{
    return 0;
}

int async_redis_event_loopbreak()
{
    return 0;
}
