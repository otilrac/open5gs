/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "sbi-path.h"
#include "nnrf-handler.h"

void nrf_state_initial(ogs_fsm_t *s, nrf_event_t *e)
{
    nrf_sm_debug(e);

    ogs_assert(s);

    OGS_FSM_TRAN(s, &nrf_state_operational);
}

void nrf_state_final(ogs_fsm_t *s, nrf_event_t *e)
{
    nrf_sm_debug(e);

    ogs_assert(s);
}

void nrf_state_operational(ogs_fsm_t *s, nrf_event_t *e)
{
    int rv;
    ogs_sbi_session_t *session = NULL;
    ogs_sbi_request_t *request = NULL;
    ogs_sbi_message_t message;
    nrf_nf_instance_t *nf_instance = NULL;

    ogs_assert(e);

    nrf_sm_debug(e);

    ogs_assert(s);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        rv = nrf_sbi_open();
        if (rv != OGS_OK) {
            ogs_fatal("Can't establish SBI path");
        }
        break;

    case OGS_FSM_EXIT_SIG:
        nrf_sbi_close();
        break;

    case NRF_EVT_SBI_SERVER:
        request = e->sbi.request;
        ogs_assert(request);
        session = e->sbi.session;
        ogs_assert(session);

        rv = ogs_sbi_parse_request(&message, request);
        if (rv != OGS_OK) {
            /* 'message' buffer is released in ogs_sbi_parse_request() */
            ogs_error("cannot parse HTTP message");
            ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                    NULL, "cannot parse HTTP message", NULL);
            break;
        }

        if (strcmp(message.h.api.version, OGS_SBI_API_VERSION) != 0) {
            ogs_error("Not supported version [%s]", message.h.api.version);
            ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                    &message, "Not supported version", NULL);
            ogs_sbi_message_free(&message);
            break;
        }

        SWITCH(message.h.api.name)
        CASE(OGS_SBI_API_NAME_NRF_NFM)

            SWITCH(message.h.resource.name)
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)

                nf_instance = nrf_nf_instance_find(message.h.resource.id);
                if (!nf_instance) {
                    SWITCH(message.h.method)
                    CASE(OGS_SBI_HTTP_METHOD_PUT)
                        nf_instance = 
                            nrf_nf_instance_add(message.h.resource.id);
                        ogs_assert(nf_instance);
                        break;
                    DEFAULT
                        ogs_error("Not found [%s]", message.h.resource.id);
                        ogs_sbi_server_send_error(session,
                                OGS_SBI_HTTP_STATUS_NOT_FOUND,
                                &message, "Not found", message.h.resource.id);
                    END
                }

                if (nf_instance) {
                    e->nf_instance = nf_instance;
                    e->sbi.message = &message;
                    ogs_fsm_dispatch(&nf_instance->sm, e);
                    if (OGS_FSM_CHECK(&nf_instance->sm,
                                nrf_nf_state_exception)) {
                        ogs_error("State machine exception");
                        ogs_sbi_message_free(&message);
                        nrf_nf_instance_remove(nf_instance);
                    }
                }
                break;

            CASE(OGS_SBI_RESOURCE_NAME_SUBSCRIPTIONS)
                ogs_sbi_subscription_data_t *subscription_data =
                    message.subscription_data;
                char *req_nf_instance_id = NULL;
                if (subscription_data) {
                    req_nf_instance_id = subscription_data->req_nf_instance_id;
                    if (req_nf_instance_id) 
                        nf_instance = nrf_nf_instance_find(req_nf_instance_id);
                }

                if (nf_instance) {
                    e->nf_instance = nf_instance;
                    e->sbi.message = &message;
                    ogs_fsm_dispatch(&nf_instance->sm, e);
                    if (OGS_FSM_CHECK(&nf_instance->sm,
                                nrf_nf_state_exception)) {
                        ogs_error("State machine exception");
                        ogs_sbi_message_free(&message);
                        nrf_nf_instance_remove(nf_instance);
                    }
                } else {
                    const char *detail = req_nf_instance_id ?
                        req_nf_instance_id : "Unknown";
                    ogs_error("Not found [%s]", detail);
                    ogs_sbi_server_send_error(session,
                            OGS_SBI_HTTP_STATUS_NOT_FOUND,
                            &message, "Not found", detail);
                }

                break;

            DEFAULT
                ogs_error("Invalid resource name [%s]",
                        message.h.resource.name);
                ogs_sbi_server_send_error(session,
                        OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED, &message,
                        "Unknown resource name", message.h.resource.name);
            END
            break;

        DEFAULT
            ogs_error("Invalid API name [%s]", message.h.api.name);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED, &message,
                    "Invalid API name", message.h.resource.name);
        END

        /* In lib/sbi/server.c, notify_completed() releases 'request' buffer. */
        ogs_sbi_message_free(&message);
        break;
    case NRF_EVT_SBI_TIMER:
        nf_instance = e->nf_instance;
        ogs_assert(nf_instance);

        ogs_assert(OGS_FSM_STATE(&nf_instance->sm));
        ogs_fsm_dispatch(&nf_instance->sm, e);
        if (OGS_FSM_CHECK(&nf_instance->sm, nrf_nf_state_de_registered)) {
            nrf_nf_instance_remove(nf_instance);
        } else if (OGS_FSM_CHECK(&nf_instance->sm, nrf_nf_state_exception)) {
            ogs_error("State machine exception");
            nrf_nf_instance_remove(nf_instance);
        }
        break;
    default:
        ogs_error("No handler for event %s", nrf_event_get_name(e));
        break;
    }
}
