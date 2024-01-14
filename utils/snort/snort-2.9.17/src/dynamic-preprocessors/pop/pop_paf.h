/****************************************************************************
 * Copyright (C) 2014-2020 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#ifndef __POP_PAF_H__
#define __POP_PAF_H__
#include "sfPolicy.h"
#include "sfPolicyUserData.h"

//void pop_paf_free(void);

#ifdef TARGET_BASED
void register_pop_paf_service(struct _SnortConfig *sc, int16_t app, tSfPolicyId policy);
#endif
void register_pop_paf_port(struct _SnortConfig *sc, unsigned int i, tSfPolicyId policy);
bool is_data_end (void* ssn);
#endif
