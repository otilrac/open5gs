
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "identity_range.h"

identity_range_t *identity_range_create(
    char *start,
    char *end,
    char *pattern
    )
{
    identity_range_t *identity_range_local_var = ogs_malloc(sizeof(identity_range_t));
    if (!identity_range_local_var) {
        return NULL;
    }
    identity_range_local_var->start = start;
    identity_range_local_var->end = end;
    identity_range_local_var->pattern = pattern;

    return identity_range_local_var;
}

void identity_range_free(identity_range_t *identity_range)
{
    if(NULL == identity_range) {
        return;
    }
    listEntry_t *listEntry;
    ogs_free(identity_range->start);
    ogs_free(identity_range->end);
    ogs_free(identity_range->pattern);
    ogs_free(identity_range);
}

cJSON *identity_range_convertToJSON(identity_range_t *identity_range)
{
    cJSON *item = cJSON_CreateObject();
    if (identity_range->start) {
        if(cJSON_AddStringToObject(item, "start", identity_range->start) == NULL) {
            goto fail;
        }
    }

    if (identity_range->end) {
        if(cJSON_AddStringToObject(item, "end", identity_range->end) == NULL) {
            goto fail;
        }
    }

    if (identity_range->pattern) {
        if(cJSON_AddStringToObject(item, "pattern", identity_range->pattern) == NULL) {
            goto fail;
        }
    }

    return item;
fail:
    if (item) {
        cJSON_Delete(item);
    }
    return NULL;
}

identity_range_t *identity_range_parseFromJSON(cJSON *identity_rangeJSON)
{
    identity_range_t *identity_range_local_var = NULL;
    cJSON *start = cJSON_GetObjectItemCaseSensitive(identity_rangeJSON, "start");

    if (start) {
        if(!cJSON_IsString(start))
        {
            goto end;
        }
    }

    cJSON *end = cJSON_GetObjectItemCaseSensitive(identity_rangeJSON, "end");

    if (end) {
        if(!cJSON_IsString(end))
        {
            goto end;
        }
    }

    cJSON *pattern = cJSON_GetObjectItemCaseSensitive(identity_rangeJSON, "pattern");

    if (pattern) {
        if(!cJSON_IsString(pattern))
        {
            goto end;
        }
    }

    identity_range_local_var = identity_range_create (
        start ? ogs_strdup(start->valuestring) : NULL,
        end ? ogs_strdup(end->valuestring) : NULL,
        pattern ? ogs_strdup(pattern->valuestring) : NULL
        );

    return identity_range_local_var;
end:
    return NULL;
}

