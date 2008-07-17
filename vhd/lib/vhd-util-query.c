/* Copyright (c) 2007, XenSource Inc.
 * All rights reserved.
 *
 * XenSource proprietary code.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libvhd.h"

int
vhd_util_query(int argc, char **argv)
{
	char *name;
	vhd_context_t vhd;
	int err, c, size, physize, parent, fields;
	off64_t currsize;

	name    = NULL;
	size    = 0;
	physize = 0;
	parent  = 0;
	fields  = 0;

	if (!argc || !argv)
		goto usage;

	optind = 0;
	while ((c = getopt(argc, argv, "n:vspfh")) != -1) {
		switch (c) {
		case 'n':
			name = optarg;
			break;
		case 'v':
			size = 1;
			break;
		case 's':
			physize = 1;
			break;
		case 'p':
			parent = 1;
			break;
		case 'f':
			fields = 1;
			break;

		case 'h':
		default:
			goto usage;
		}
	}

	if (!name || optind != argc)
		goto usage;

	err = vhd_open(&vhd, name, VHD_OPEN_RDONLY);
	if (err) {
		printf("error opening %s: %d\n", name, err);
		return err;
	}

	if (size)
		printf("%llu\n", vhd.footer.curr_size >> 20);

	if (physize) {
		err = vhd_get_phys_size(&vhd, &currsize);
		if (err)
			printf("failed to get physical size: %d\n", err);
		else
			printf("%llu\n", currsize);
	}

	if (parent) {
		if (vhd.footer.type != HD_TYPE_DIFF)
			printf("%s has no parent\n", name);
		else {
			char *pname;

			err = vhd_parent_locator_get(&vhd, &pname);
			if (err)
				printf("query failed\n");
			else {
				printf("%s\n", pname);
				free(pname);
			}
		}
	}

	if (fields) {
		int hidden;

		err = vhd_hidden(&vhd, &hidden);
		if (err) {
			printf("error checking 'hidden' field: %d\n", err);
			goto done;
		}

		printf("hidden: %d\n", hidden);
	}
		
 done:
	vhd_close(&vhd);
	return err;

usage:
	printf("options: <-n name> [-v print virtual size (blocks)] "
			"[-s print physical utilization (bytes)] "
			"[-p print parent] [-f print fields] [-h help]\n");
	return -EINVAL;
}