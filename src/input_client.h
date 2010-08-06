/*
 *  This file is part of pom-ng.
 *  Copyright (C) 2010 Guy Martin <gmsoft@tuxicoman.be>
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



#ifndef __INPUT_CLIENT_H__
#define __INPUT_CLIENT_H__

#define INPUT_CLIENT_REGISTRY "input"

int input_client_init();

int input_client_cmd_mod_load(char *mod_name);
int input_client_cmd_add(char *name);
int input_client_cmd_remove(unsigned int input_id);
int input_client_cmd_start(unsigned int input_id);
int input_client_cmd_stop(unsigned int input_id);

#endif
