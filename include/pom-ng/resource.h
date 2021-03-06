/*
 *  This file is part of pom-ng.
 *  Copyright (C) 2012 Guy Martin <gmsoft@tuxicoman.be>
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

#ifndef __POM_NG_RESOURCE_H__
#define __POM_NG_RESOURCE_H__

#include <pom-ng/datastore.h>

struct resource_template {
	char *dataset_name;
	struct datavalue_template *data_template;
};

struct resource* resource_open(char *resource_name, struct resource_template *template);
int resource_close(struct resource *r);

struct resource_dataset* resource_dataset_open(struct resource *r, char *dset_name);
int resource_dataset_close(struct resource_dataset *ds);

int resource_dataset_read(struct resource_dataset *ds, struct datavalue **dvp);


#endif
