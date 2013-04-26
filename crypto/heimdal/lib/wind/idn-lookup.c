
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <assert.h>
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <getarg.h>
#include <roken.h>

#include "windlocl.h"

static int version_flag = 0;
static int help_flag	= 0;


static int
is_separator(uint32_t u)
{
    return u == 0x002E || u == 0x3002;
}

static void
lookup(const char *name)
{
    unsigned i;
    char encoded[1024];
    char *ep;
    int ret;
    struct addrinfo hints;
    struct addrinfo *ai;

    size_t u_len = strlen(name);
    uint32_t *u = malloc(u_len * sizeof(uint32_t));
    size_t norm_len = u_len * 2;
    uint32_t *norm = malloc(norm_len * sizeof(uint32_t));

    if (u == NULL && u_len != 0)
	errx(1, "malloc failed");
    if (norm == NULL && norm_len != 0)
	errx(1, "malloc failed");

    ret = wind_utf8ucs4(name, u, &u_len);
    if (ret)
	errx(1, "utf8 conversion failed");
    ret = wind_stringprep(u, u_len, norm, &norm_len, WIND_PROFILE_NAME);
    if (ret)
	errx(1, "stringprep failed");
    free(u);

    ep = encoded;
    for (i = 0; i < norm_len; ++i) {
	unsigned j;
	size_t len;

	for (j = i; j < norm_len && !is_separator(norm[j]); ++j)
	    ;
	len = sizeof(encoded) - (ep - encoded);
	ret = wind_punycode_label_toascii(norm + i, j - i, ep, &len);
	if (ret)
	    errx(1, "punycode failed");

	ep += len;
	*ep++ = '.';
	i = j;
    }
    *ep = '\0';
    free(norm);

    printf("Converted \"%s\" into \"%s\"\n", name, encoded);

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    ret = getaddrinfo(encoded, NULL, &hints, &ai);
    if(ret)
	errx(1, "getaddrinfo failed: %s", gai_strerror(ret));
    printf("canonical-name: %s\n", ai->ai_canonname);
    freeaddrinfo(ai);
}

static struct getargs args[] = {
    {"version",	0,	arg_flag,	&version_flag,
     "print version", NULL },
    {"help",	0,	arg_flag,	&help_flag,
     NULL, NULL }
};

static void
usage (int ret)
{
    arg_printusage(args, sizeof(args)/sizeof(args[0]), NULL,
		   "dns-names ...");
    exit (ret);
}

int
main(int argc, char **argv)
{
    int optidx = 0;
    unsigned i;

    setprogname (argv[0]);

    if(getarg(args, sizeof(args) / sizeof(args[0]), argc, argv, &optidx))
	usage(1);

    if (help_flag)
	usage (0);

    if(version_flag){
	print_version(NULL);
	exit(0);
    }

    argc -= optidx;
    argv += optidx;

    if (argc == 0)
	usage(1);

    for (i = 0; i < argc; ++i)
	lookup(argv[i]);
    return 0;
}