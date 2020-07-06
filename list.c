#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "proxy.h"

//取出key对应的value
const char *list_get_key(struct METADATA_HEAD *list, const char *key)
{
    http_metadata_item *item;
    //对队列进行遍历操作
    TAILQ_FOREACH(item, list, entries) {
        if(strcmp(item->key, key) == 0)
        {
            return item->value;
        }
    }

    return NULL;
}
