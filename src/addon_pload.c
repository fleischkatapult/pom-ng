/*
 *  This file is part of pom-ng.
 *  Copyright (C) 2012-2014 Guy Martin <gmsoft@tuxicoman.be>
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


#include "addon_pload.h"
#include "addon_event.h"
#include "addon_data.h"
#include "pload.h"

// Called from lua to get the metatable
static int addon_pload_metatable(lua_State *L) {
	
	const char *key = luaL_checkstring(L, 2);

	struct addon_pload *ap = luaL_checkudata(L, 1, ADDON_PLOAD_METATABLE);
	struct pload *p = ap->pload;

	if (!strcmp(key, "event")) {
		// Return the corresponding event
		addon_event_push(L, p->rel_event);
	} else if (!strcmp(key, "data")) {
		addon_data_push(L, p->data, p->type->analyzer->data_reg);
	} else if (!strcmp(key, "type") && p->type) {
		// Return the type table if any
		lua_newtable(L);
		
		// Add type name
		lua_pushliteral(L, "name");
		lua_pushstring(L, p->type->name);
		lua_settable(L, -3);

		// Add type description
		lua_pushliteral(L, "description");
		lua_pushstring(L, p->type->description);
		lua_settable(L, -3);

		// Add type extension
		lua_pushliteral(L, "extension");
		lua_pushstring(L, p->type->extension);
		lua_settable(L, -3);

		// Add the class
		lua_pushliteral(L, "class");
		switch (p->type->cls) {
			case pload_class_unknown:
				lua_pushliteral(L, "unknown");
				break;
			case pload_class_application:
				lua_pushliteral(L, "application");
				break;
			case pload_class_audio:
				lua_pushliteral(L, "audio");
				break;
			case pload_class_image:
				lua_pushliteral(L, "image");
				break;
			case pload_class_video:
				lua_pushliteral(L, "video");
				break;
			case pload_class_document:
				lua_pushliteral(L, "document");
				break;
		}
		lua_settable(L, -3);
	} else if (!strcmp(key, "parent") && p->parent) {
		addon_pload_push(L, p->parent, NULL);
	} else {
		return 0;
	}
	return 1;
}

static int addon_pload_data_metatable(lua_State *L) {

	struct addon_pload_data *p = luaL_checkudata(L, 1, ADDON_PLOAD_DATA_METATABLE);
	const char *key = luaL_checkstring(L, 2);
	if (!strcmp(key, "data")) {
		lua_pushlstring(L, p->data, p->len);
	} else if (!strcmp(key, "len")) {
		lua_pushinteger(L, p->len);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int addon_pload_lua_register(lua_State *L) {

	luaL_newmetatable(L, ADDON_PLOAD_METATABLE);
	lua_pushliteral(L, "__index");
	lua_pushcfunction(L, addon_pload_metatable);
	lua_settable(L, -3);

	luaL_newmetatable(L, ADDON_PLOAD_DATA_METATABLE);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, addon_pload_data_metatable);
	lua_settable(L, -3);

	return POM_OK;
}

void addon_pload_data_push(lua_State *L) {
	lua_newuserdata(L, sizeof(struct addon_pload_data));
	luaL_getmetatable(L, ADDON_PLOAD_DATA_METATABLE);
	lua_setmetatable(L, -2);
}

void addon_pload_data_update(lua_State *L, int n, void *data, size_t len) {
	struct addon_pload_data *p = luaL_checkudata(L, n, ADDON_PLOAD_DATA_METATABLE);
	p->data = data;
	p->len = len;
}

void addon_pload_push(lua_State *L, struct pload *pload, void *priv) {

	struct addon_pload *p = lua_newuserdata(L, sizeof(struct addon_pload));
	
	p->pload = pload;
	p->priv = priv;;

	luaL_getmetatable(L, ADDON_PLOAD_METATABLE);
	lua_setmetatable(L, -2);
}

struct pload *addon_pload_get(lua_State *L, int n) {

	struct addon_pload *p = luaL_checkudata(L, n, ADDON_PLOAD_METATABLE);
	return p->pload;
}

void *addon_pload_get_priv(lua_State *L, int n) {

	struct addon_pload *p = luaL_checkudata(L, n, ADDON_PLOAD_METATABLE);
	return p->priv;
}
