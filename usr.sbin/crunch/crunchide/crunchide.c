
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

#include <sys/cdefs.h>
#ifndef lint
__RCSID("$NetBSD: crunchide.c,v 1.8 1997/11/01 06:51:45 lukem Exp $");
#endif
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <a.out.h>

#include "extern.h"

char *pname = "crunchide";

void usage(void);

void add_to_keep_list(char *symbol);
void add_file_to_keep_list(char *filename);

int hide_syms(const char *filename);

int verbose;

int main(int, char *[]);

int
main(int argc, char **argv)
{
    int ch, errors;

    if(argc > 0) pname = argv[0];

    while ((ch = getopt(argc, argv, "k:f:v")) != -1)
	switch(ch) {
	case 'k':
	    add_to_keep_list(optarg);
	    break;
	case 'f':
	    add_file_to_keep_list(optarg);
	    break;
	case 'v':
	    verbose = 1;
	    break;
	default:
	    usage();
	}

    argc -= optind;
    argv += optind;

    if(argc == 0) usage();

    errors = 0;
    while(argc) {
	if (hide_syms(*argv))
		errors = 1;
	argc--, argv++;
    }

    return errors;
}

void
usage(void)
{
    fprintf(stderr,
	    "usage: %s [-k <symbol-name>] [-f <keep-list-file>] <files> ...\n",
	    pname);
    exit(1);
}

/* ---------------------------- */

struct keep {
    struct keep *next;
    char *sym;
} *keep_list;

void
add_to_keep_list(char *symbol)
{
    struct keep *newp, *prevp, *curp;
    int cmp;

    cmp = 0;

    for(curp = keep_list, prevp = NULL; curp; prevp = curp, curp = curp->next)
	if((cmp = strcmp(symbol, curp->sym)) <= 0) break;

    if(curp && cmp == 0)
	return;	/* already in table */

    newp = (struct keep *) malloc(sizeof(struct keep));
    if(newp) newp->sym = strdup(symbol);
    if(newp == NULL || newp->sym == NULL) {
	fprintf(stderr, "%s: out of memory for keep list\n", pname);
	exit(1);
    }

    newp->next = curp;
    if(prevp) prevp->next = newp;
    else keep_list = newp;
}

int
in_keep_list(const char *symbol)
{
    struct keep *curp;
    int cmp;

    cmp = 0;

    for(curp = keep_list; curp; curp = curp->next)
	if((cmp = strcmp(symbol, curp->sym)) <= 0) break;

    return curp && cmp == 0;
}

void
add_file_to_keep_list(char *filename)
{
    FILE *keepf;
    char symbol[1024];
    int len;

    if((keepf = fopen(filename, "r")) == NULL) {
	perror(filename);
	usage();
    }

    while(fgets(symbol, sizeof(symbol), keepf)) {
	len = strlen(symbol);
	if(len && symbol[len-1] == '\n')
	    symbol[len-1] = '\0';

	add_to_keep_list(symbol);
    }
    fclose(keepf);
}

/* ---------------------------- */

struct {
	const char *name;
	int	(*check)(int, const char *);	/* 1 if match, zero if not */
	int	(*hide)(int, const char *);	/* non-zero if error */
} exec_formats[] = {
#ifdef NLIST_AOUT
	{	"a.out",	check_aout,	hide_aout,	},
#endif
#ifdef NLIST_ECOFF
	{	"ECOFF",	check_elf64,	hide_elf64,	},
#endif
#ifdef NLIST_ELF32
	{	"ELF32",	check_elf32,	hide_elf32,	},
#endif
#ifdef NLIST_ELF64
	{	"ELF64",	check_elf64,	hide_elf64,	},
#endif
};

int
hide_syms(const char *filename)
{
	int fd, i, n, rv;

	fd = open(filename, O_RDWR, 0);
	if (fd == -1) {
		perror(filename);
		return 1;
	}

	rv = 0;

        n = sizeof exec_formats / sizeof exec_formats[0];
        for (i = 0; i < n; i++) {
		if (lseek(fd, 0, SEEK_SET) != 0) {
			perror(filename);
			goto err;
		}
                if ((*exec_formats[i].check)(fd, filename) != 0)
                        break;
	}
	if (i == n) {
		fprintf(stderr, "%s: unknown executable format\n", filename);
		goto err;
	}

	if (verbose)
		fprintf(stderr, "%s is an %s binary\n", filename,
		    exec_formats[i].name);

	if (lseek(fd, 0, SEEK_SET) != 0) {
		perror(filename);
		goto err;
	}
	rv = (*exec_formats[i].hide)(fd, filename);

out:
	close (fd);
	return (rv);

err:
	rv = 1;
	goto out;
}