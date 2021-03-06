/*
 *  This file is part of pom-ng.
 *  Copyright (C) 2010-2014 Guy Martin <gmsoft@tuxicoman.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "common.h"
#include "xmlrpccmd.h"
#include "xmlrpcsrv.h"

#include "xmlrpccmd_monitor.h"
#include "xmlrpccmd_registry.h"

#include "registry.h"


#include <pom-ng/ptype_bool.h>
#include <pom-ng/ptype_string.h>
#include <pom-ng/ptype_timestamp.h>
#include <pom-ng/ptype_uint8.h>
#include <pom-ng/ptype_uint16.h>
#include <pom-ng/ptype_uint32.h>
#include <pom-ng/ptype_uint64.h>

static struct ptype_reg *pt_bool = NULL, *pt_string = NULL, *pt_timestamp = NULL, *pt_uint8 = NULL, *pt_uint16 = NULL, *pt_uint32 = NULL, *pt_uint64 = NULL;

#define XMLRPCCMD_NUM 3
static struct xmlrpcsrv_command xmlrpccmd_commands[XMLRPCCMD_NUM] = {

	{
		.name = "core.getVersion",
		.callback_func = xmlrpccmd_core_get_version,
		.signature = "s:",
		.help = "Get " PACKAGE_NAME " version",
	},

	{
		.name = "core.getLog",
		.callback_func = xmlrpccmd_core_get_log,
		.signature = "A:i",
		.help = "Get the logs",
	},

	{
		.name = "core.pollLog",
		.callback_func = xmlrpccmd_core_poll_log,
		.signature = "A:ii",
		.help = "Poll the logs",
	},

};


int xmlrpccmd_init() {

	pt_bool = ptype_get_type("bool");
	pt_string = ptype_get_type("string");
	pt_timestamp = ptype_get_type("timestamp");
	pt_uint8 = ptype_get_type("uint8");
	pt_uint16 = ptype_get_type("uint16");
	pt_uint32 = ptype_get_type("uint32");
	pt_uint64 = ptype_get_type("uint64");

	if (!pt_bool || !pt_string || !pt_timestamp || !pt_uint8 || !pt_uint16 || !pt_uint32 || !pt_uint64) {
		pomlog(POMLOG_ERR "Some ptype where not loaded correctly");
		return POM_ERR;
	}
	return POM_OK;
}

int xmlrpccmd_cleanup() {

	xmlrpccmd_monitor_cleanup();

	return POM_OK;
}

int xmlrpccmd_register_all() {

	int i;

	for (i = 0; i < XMLRPCCMD_NUM; i++) {
		if (xmlrpcsrv_register_command(&xmlrpccmd_commands[i]) == POM_ERR)
			return POM_ERR;
	}

	int res = POM_OK;
	res += xmlrpccmd_registry_register_all();
	res += xmlrpccmd_monitor_register_all();

	return res;

}


/*
is_utf8 is distributed under the following terms:

Copyright (c) 2013 Palard Julien. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

static int is_utf8(unsigned char *str, size_t len) {

	size_t i = 0;
	size_t continuation_bytes = 0;

	while (i < len) {
		if (str[i] <= 0x7F)
			continuation_bytes = 0;
		else if (str[i] >= 0xC0 /*11000000*/ && str[i] <= 0xDF /*11011111*/)
			continuation_bytes = 1;
		else if (str[i] >= 0xE0 /*11100000*/ && str[i] <= 0xEF /*11101111*/)
			continuation_bytes = 2;
		else if (str[i] >= 0xF0 /*11110000*/ && str[i] <= 0xF4 /* Cause of RFC 3629 */)
			continuation_bytes = 3;
		else
			return i + 1;
		i += 1;

		while (i < len && continuation_bytes > 0 && str[i] >= 0x80 && str[i] <= 0xBF) {

			i += 1;
			continuation_bytes -= 1;
		}

		if (continuation_bytes != 0)
			return i + 1;
	}

	return 0;
}

xmlrpc_value *xmlrpccmd_ptype_to_val(xmlrpc_env* const envP, struct ptype* p) {

	if (p->type == pt_bool) {
		return xmlrpc_bool_new(envP, *PTYPE_BOOL_GETVAL(p));
	} else if (p->type == pt_string) {
		char *value = PTYPE_STRING_GETVAL(p);
		size_t len = strlen(value);
		xmlrpc_value *res = NULL;
		if (!is_utf8((unsigned char *)value, len))
			res = xmlrpc_string_new(envP, value);
		else
			res = xmlrpc_base64_new(envP, len, (unsigned char*)value);
		return res;
	} else if (p->type == pt_timestamp) {
		// Don't use the time version of xmlrpc as it's not precise enough
		ptime t = *PTYPE_TIMESTAMP_GETVAL(p);
		return xmlrpc_build_value(envP, "{s:i,s:i}", "sec", pom_ptime_sec(t), "usec", pom_ptime_usec(t));
	} else if (p->type == pt_uint8) {
		return xmlrpc_int_new(envP, *PTYPE_UINT8_GETVAL(p));
	} else if (p->type == pt_uint16) {
		return xmlrpc_int_new(envP, *PTYPE_UINT16_GETVAL(p));
	} else if (p->type == pt_uint32) {
		return xmlrpc_int_new(envP, *PTYPE_UINT32_GETVAL(p));
	} else if (p->type == pt_uint64) {
		return xmlrpc_i8_new(envP, *PTYPE_UINT64_GETVAL(p));
	}

	// The type is not handled, return a string
	
	char *value = ptype_print_val_alloc(p, NULL);
	if (!value) {
		xmlrpc_faultf(envP, "Error while getting ptype value");
		return NULL;
	}
	
	size_t len = strlen(value);
	xmlrpc_value *res = NULL;

	if (!is_utf8((unsigned char *)value, len))
		res = xmlrpc_string_new(envP, value);
	else
		res = xmlrpc_base64_new(envP, len, (unsigned char *)value);
	free(value);
	return res;
}

xmlrpc_value *xmlrpccmd_core_get_version(xmlrpc_env * const envP, xmlrpc_value * const paramArrayP, void * const userData) {
	
	return xmlrpc_string_new(envP, VERSION);
}

xmlrpc_value *xmlrpccmd_core_get_log(xmlrpc_env * const envP, xmlrpc_value * const paramArrayP, void * const userData) {

	uint32_t last_id;

	xmlrpc_decompose_value(envP, paramArrayP, "(i)", &last_id);
	if (envP->fault_occurred)
		return NULL;

	xmlrpc_value *res = xmlrpc_array_new(envP);
	if (envP->fault_occurred)
		return NULL;

	pomlog_rlock();

	struct pomlog_entry *log = pomlog_get_tail();

	if (log->id <= last_id) {
		pomlog_unlock();
		return res;
	}

	while (log && log->id > last_id + 1)
		log = log->main_prev;

	while (log) {
		xmlrpc_value *entry = xmlrpc_build_value(envP, "{s:i,s:i,s:s,s:s,s:t}",
								"id", log->id,
								"level", log->level,
								"file", log->file,
								"data", log->data,
								"timestamp", (time_t)log->ts.tv_sec);
		xmlrpc_array_append_item(envP, res, entry);
		xmlrpc_DECREF(entry);
		log = log->main_next;

	}
	pomlog_unlock();

	return res;
}

xmlrpc_value *xmlrpccmd_core_poll_log(xmlrpc_env * const envP, xmlrpc_value * const paramArrayP, void * const userData) {
	
	uint32_t last_id;
	int level, max_results;

	xmlrpc_decompose_value(envP, paramArrayP, "(iii)", &last_id, &level, &max_results);

	if (envP->fault_occurred)
		return NULL;

	xmlrpc_value *res = xmlrpc_array_new(envP);
	if (envP->fault_occurred)
		return NULL;

	struct timeval now;
	gettimeofday(&now, NULL);
	struct timespec then = { 0 };
	then.tv_sec = now.tv_sec + XMLRPCSRV_POLL_TIMEOUT;

	int count = 0, results = 0;

	while (1) {


		pomlog_rlock();
		struct pomlog_entry *log = pomlog_get_tail();

		// Go to the last entry we were at
		while (log && log->main_prev && log->id > last_id + 1) {
			log = log->main_prev;

			if (log->level <= level)
				count++;
			if (max_results && count >= max_results - 1)
				break;
		}
		
		// If we already have the last, don't do anything
		if (log && log->id <= last_id)
			log = NULL;

		// Add whatever is after our last entry
		for (; log; log = log->main_next) {
			if (log->level > level)
				continue;
			xmlrpc_value *entry = xmlrpc_build_value(envP, "{s:i,s:i,s:s,s:s,s:t}",
									"id", log->id,
									"level", log->level,
									"file", log->file,
									"data", log->data,
									"timestamp", (time_t)log->ts.tv_sec);
			xmlrpc_array_append_item(envP, res, entry);
			xmlrpc_DECREF(entry);
			results++;

		}
		pomlog_unlock();

		if (results)
			break;

		gettimeofday(&now, NULL);
		if (now.tv_sec > then.tv_sec)
			break;
		
		if (pomlog_poll(&then) == POM_ERR) {
			// We are shutting down
			break;
		}
	};

	return res;

}
