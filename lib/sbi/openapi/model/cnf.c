
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cnf.h"

OpenAPI_cnf_t *OpenAPI_cnf_create(
    OpenAPI_list_t *cnf_units
    )
{
    OpenAPI_cnf_t *cnf_local_var = OpenAPI_malloc(sizeof(OpenAPI_cnf_t));
    if (!cnf_local_var) {
        return NULL;
    }
    cnf_local_var->cnf_units = cnf_units;

    return cnf_local_var;
}

void OpenAPI_cnf_free(OpenAPI_cnf_t *cnf)
{
    if (NULL == cnf) {
        return;
    }
    OpenAPI_lnode_t *node;
    OpenAPI_list_for_each(cnf->cnf_units, node) {
        OpenAPI_cnf_unit_free(node->data);
    }
    OpenAPI_list_free(cnf->cnf_units);
    ogs_free(cnf);
}

cJSON *OpenAPI_cnf_convertToJSON(OpenAPI_cnf_t *cnf)
{
    cJSON *item = NULL;

    if (cnf == NULL) {
        ogs_error("OpenAPI_cnf_convertToJSON() failed [Cnf]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!cnf->cnf_units) {
        ogs_error("OpenAPI_cnf_convertToJSON() failed [cnf_units]");
        goto end;
    }
    cJSON *cnf_unitsList = cJSON_AddArrayToObject(item, "cnfUnits");
    if (cnf_unitsList == NULL) {
        ogs_error("OpenAPI_cnf_convertToJSON() failed [cnf_units]");
        goto end;
    }

    OpenAPI_lnode_t *cnf_units_node;
    if (cnf->cnf_units) {
        OpenAPI_list_for_each(cnf->cnf_units, cnf_units_node) {
            cJSON *itemLocal = OpenAPI_cnf_unit_convertToJSON(cnf_units_node->data);
            if (itemLocal == NULL) {
                ogs_error("OpenAPI_cnf_convertToJSON() failed [cnf_units]");
                goto end;
            }
            cJSON_AddItemToArray(cnf_unitsList, itemLocal);
        }
    }

end:
    return item;
}

OpenAPI_cnf_t *OpenAPI_cnf_parseFromJSON(cJSON *cnfJSON)
{
    OpenAPI_cnf_t *cnf_local_var = NULL;
    cJSON *cnf_units = cJSON_GetObjectItemCaseSensitive(cnfJSON, "cnfUnits");
    if (!cnf_units) {
        ogs_error("OpenAPI_cnf_parseFromJSON() failed [cnf_units]");
        goto end;
    }

    OpenAPI_list_t *cnf_unitsList;

    cJSON *cnf_units_local_nonprimitive;
    if (!cJSON_IsArray(cnf_units)) {
        ogs_error("OpenAPI_cnf_parseFromJSON() failed [cnf_units]");
        goto end;
    }

    cnf_unitsList = OpenAPI_list_create();

    cJSON_ArrayForEach(cnf_units_local_nonprimitive, cnf_units ) {
        if (!cJSON_IsObject(cnf_units_local_nonprimitive)) {
            ogs_error("OpenAPI_cnf_parseFromJSON() failed [cnf_units]");
            goto end;
        }
        OpenAPI_cnf_unit_t *cnf_unitsItem = OpenAPI_cnf_unit_parseFromJSON(cnf_units_local_nonprimitive);

        OpenAPI_list_add(cnf_unitsList, cnf_unitsItem);
    }

    cnf_local_var = OpenAPI_cnf_create (
        cnf_unitsList
        );

    return cnf_local_var;
end:
    return NULL;
}

