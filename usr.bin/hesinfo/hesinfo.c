
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*	$NetBSD: hesinfo.c,v 1.1 1999/01/25 22:45:55 lukem Exp $	*/
/* Copyright 1988, 1996 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This file is a simple driver for the Hesiod library. */


#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <errno.h>
#include <hesiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int 
main(int argc, char **argv)
{
	char  **list, **p, *bindname, *name, *type;
	int     lflag = 0, errflg = 0, bflag = 0, c;
	void   *context;

	while ((c = getopt(argc, argv, "lb")) != -1) {
		switch (c) {
		case 'l':
			lflag = 1;
			break;
		case 'b':
			bflag = 1;
			break;
		default:
			errflg++;
			break;
		}
	}
	if (argc - optind != 2 || errflg) {
		fprintf(stderr, "usage: hesinfo [-bl] name type\n");
		fprintf(stderr, "\t-l selects long format\n");
		fprintf(stderr, "\t-b also does hes_to_bind conversion\n");
		exit(2);
	}
	name = argv[optind];
	type = argv[optind + 1];

	if (hesiod_init(&context) < 0) {
		if (errno == ENOEXEC)
			warnx(
			    "hesiod_init: Invalid Hesiod configuration file.");
		else
			warn("hesiod_init");
	}
	/* Display bind name if requested. */
	if (bflag) {
		if (lflag)
			printf("hes_to_bind(%s, %s) expands to\n", name, type);
		bindname = hesiod_to_bind(context, name, type);
		if (!bindname) {
			if (lflag)
				printf("nothing\n");
			if (errno == ENOENT)
				warnx("hesiod_to_bind: Unknown rhs-extension.");
			else
				warn("hesiod_to_bind");
			exit(1);
		}
		printf("%s\n", bindname);
		free(bindname);
		if (lflag)
			printf("which ");
	}
	if (lflag)
		printf("resolves to\n");

	/* Do the hesiod resolve and check for errors. */
	list = hesiod_resolve(context, name, type);
	if (!list) {
		if (lflag)
			printf("nothing\n");
		if (errno == ENOENT)
			warnx("hesiod_resolve: Hesiod name not found.");
		else
			warn("hesiod_resolve");
		exit(1);
	}
	/* Display the results. */
	for (p = list; *p; p++)
		printf("%s\n", *p);

	hesiod_free_list(context, list);
	hesiod_end(context);
	exit(0);
}