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

#ifndef SMF_SBI_PATH_H
#define SMF_SBI_PATH_H

#include "nnrf-build.h"

#ifdef __cplusplus
extern "C" {
#endif

int smf_sbi_open(void);
void smf_sbi_close(void);

void smf_sbi_send_nf_register(ogs_sbi_client_t *client,
        ogs_sbi_nf_type_e nf_type, ogs_sbi_nf_status_e nf_status);
void smf_sbi_send_nf_update(ogs_sbi_client_t *client);
void smf_sbi_send_nf_de_register(ogs_sbi_client_t *client);

void smf_sbi_send_nf_status_subscribe(ogs_sbi_client_t *client);

#ifdef __cplusplus
}
#endif

#endif /* SMF_SBI_PATH_H */
