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

#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <pom-ng/resource.h>
#include <pom-ng/datastore.h>
#include <libxml/parser.h>

#define RESOURCE_DIR DATAROOT "/resources/"


struct resource {
	char *name;
	xmlDocPtr doc;
	xmlNodePtr ds_root;
	struct resource_template *tmplt;
};

struct resource_dataset {
	char *name;
	struct resource *r;
	xmlNodePtr dset_node, cur;
	struct datavalue_template *data_template;
	struct datavalue *values;
};


#endif
