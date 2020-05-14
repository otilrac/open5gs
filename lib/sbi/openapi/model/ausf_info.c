
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ausf_info.h"

ogs_sbi_ausf_info_t *ogs_sbi_ausf_info_create(
    char *group_id,
    ogs_sbi_list_t *supi_ranges,
    ogs_sbi_list_t *routing_indicators
    )
{
    ogs_sbi_ausf_info_t *ausf_info_local_var = ogs_sbi_malloc(sizeof(ogs_sbi_ausf_info_t));
    if (!ausf_info_local_var) {
        return NULL;
    }
    ausf_info_local_var->group_id = group_id;
    ausf_info_local_var->supi_ranges = supi_ranges;
    ausf_info_local_var->routing_indicators = routing_indicators;

    return ausf_info_local_var;
}

void ogs_sbi_ausf_info_free(ogs_sbi_ausf_info_t *ausf_info)
{
    if (NULL == ausf_info) {
        return;
    }
    ogs_sbi_lnode_t *node;
    ogs_free(ausf_info->group_id);
    ogs_sbi_list_for_each(ausf_info->supi_ranges, node) {
        ogs_sbi_supi_range_free(node->data);
    }
    ogs_sbi_list_free(ausf_info->supi_ranges);
    ogs_sbi_list_for_each(ausf_info->routing_indicators, node) {
        ogs_free(node->data);
    }
    ogs_sbi_list_free(ausf_info->routing_indicators);
    ogs_free(ausf_info);
}

cJSON *ogs_sbi_ausf_info_convertToJSON(ogs_sbi_ausf_info_t *ausf_info)
{
    cJSON *item = cJSON_CreateObject();
    if (ausf_info->group_id) {
        if (cJSON_AddStringToObject(item, "groupId", ausf_info->group_id) == NULL) {
            ogs_error("ogs_sbi_ausf_info_convertToJSON() failed [group_id]");
            goto end;
        }
    }

    if (ausf_info->supi_ranges) {
        cJSON *supi_ranges = cJSON_AddArrayToObject(item, "supiRanges");
        if (supi_ranges == NULL) {
            ogs_error("ogs_sbi_ausf_info_convertToJSON() failed [supi_ranges]");
            goto end;
        }

        ogs_sbi_lnode_t *supi_ranges_node;
        if (ausf_info->supi_ranges) {
            ogs_sbi_list_for_each(ausf_info->supi_ranges, supi_ranges_node) {
                cJSON *itemLocal = ogs_sbi_supi_range_convertToJSON(supi_ranges_node->data);
                if (itemLocal == NULL) {
                    ogs_error("ogs_sbi_ausf_info_convertToJSON() failed [supi_ranges]");
                    goto end;
                }
                cJSON_AddItemToArray(supi_ranges, itemLocal);
            }
        }
    }

    if (ausf_info->routing_indicators) {
        cJSON *routing_indicators = cJSON_AddArrayToObject(item, "routingIndicators");
        if (routing_indicators == NULL) {
            ogs_error("ogs_sbi_ausf_info_convertToJSON() failed [routing_indicators]");
            goto end;
        }

        ogs_sbi_lnode_t *routing_indicators_node;
        ogs_sbi_list_for_each(ausf_info->routing_indicators, routing_indicators_node)  {
            if (cJSON_AddStringToObject(routing_indicators, "", (char*)routing_indicators_node->data) == NULL) {
                ogs_error("ogs_sbi_ausf_info_convertToJSON() failed [routing_indicators]");
                goto end;
            }
        }
    }

end:
    return item;
}

ogs_sbi_ausf_info_t *ogs_sbi_ausf_info_parseFromJSON(cJSON *ausf_infoJSON)
{
    ogs_sbi_ausf_info_t *ausf_info_local_var = NULL;
    cJSON *group_id = cJSON_GetObjectItemCaseSensitive(ausf_infoJSON, "groupId");

    if (group_id) {
        if (!cJSON_IsString(group_id)) {
            ogs_error("ogs_sbi_ausf_info_parseFromJSON() failed [group_id]");
            goto end;
        }
    }

    cJSON *supi_ranges = cJSON_GetObjectItemCaseSensitive(ausf_infoJSON, "supiRanges");

    ogs_sbi_list_t *supi_rangesList;
    if (supi_ranges) {
        cJSON *supi_ranges_local_nonprimitive;
        if (!cJSON_IsArray(supi_ranges)) {
            ogs_error("ogs_sbi_ausf_info_parseFromJSON() failed [supi_ranges]");
            goto end;
        }

        supi_rangesList = ogs_sbi_list_create();

        cJSON_ArrayForEach(supi_ranges_local_nonprimitive, supi_ranges ) {
            if (!cJSON_IsObject(supi_ranges_local_nonprimitive)) {
                ogs_error("ogs_sbi_ausf_info_parseFromJSON() failed [supi_ranges]");
                goto end;
            }
            ogs_sbi_supi_range_t *supi_rangesItem = ogs_sbi_supi_range_parseFromJSON(supi_ranges_local_nonprimitive);

            ogs_sbi_list_add(supi_rangesList, supi_rangesItem);
        }
    }

    cJSON *routing_indicators = cJSON_GetObjectItemCaseSensitive(ausf_infoJSON, "routingIndicators");

    ogs_sbi_list_t *routing_indicatorsList;
    if (routing_indicators) {
        cJSON *routing_indicators_local;
        if (!cJSON_IsArray(routing_indicators)) {
            ogs_error("ogs_sbi_ausf_info_parseFromJSON() failed [routing_indicators]");
            goto end;
        }
        routing_indicatorsList = ogs_sbi_list_create();

        cJSON_ArrayForEach(routing_indicators_local, routing_indicators) {
            if (!cJSON_IsString(routing_indicators_local)) {
                ogs_error("ogs_sbi_ausf_info_parseFromJSON() failed [routing_indicators]");
                goto end;
            }
            ogs_sbi_list_add(routing_indicatorsList, ogs_strdup(routing_indicators_local->valuestring));
        }
    }

    ausf_info_local_var = ogs_sbi_ausf_info_create (
        group_id ? ogs_strdup(group_id->valuestring) : NULL,
        supi_ranges ? supi_rangesList : NULL,
        routing_indicators ? routing_indicatorsList : NULL
        );

    return ausf_info_local_var;
end:
    return NULL;
}

