#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "object.h"

object_t *object_create()
{
    object_t *object = ogs_malloc(sizeof(object_t));

    return object;
}

void object_free(object_t *object)
{
    ogs_free (object);
}

cJSON *object_convertToJSON(object_t *object)
{
    cJSON *item = cJSON_CreateObject();

    return item;
fail:
    cJSON_Delete(item);
    return NULL;
}

object_t *object_parseFromJSON(cJSON *objectJSON)
{
    object_t *object = NULL;

    return object;
end:
    return NULL;
}
