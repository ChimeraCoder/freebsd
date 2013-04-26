
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
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <errno.h>
#include <libgeom.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct retval {
	struct retval *retval;
	const char *param;
	char *value;
};

struct retval *retval;
int verbose;

static void
usage()
{
	fprintf(stdout, "usage: %s [-v] param[:len][=value] ...\n",
	    getprogname());
	exit(1);
}

static int
parse(char *arg, char **param, char **value, int *len)
{
	char *e, *colon, *equal;

	if (*arg == '\0')
		return (EINVAL);

	colon = strchr(arg, ':');
	equal = strchr(arg, '=');
	if (colon == NULL && equal == NULL)
		return (EINVAL);
	if (colon == arg || equal == arg)
		return (EINVAL);
	if (colon != NULL && equal != NULL && equal < colon)
		return (EINVAL);

	if (colon != NULL)
		*colon++ = '\0';
	if (equal != NULL)
		*equal++ = '\0';

	*param = arg;
	if (colon != NULL) {
		/* Length specification. This parameter is RW. */
		if (*colon == '\0')
			return (EINVAL);
		*len = strtol(colon, &e, 0);
		if (*e != '\0')
			return (EINVAL);
		if (*len <= 0 || *len > PATH_MAX)
			return (EINVAL);
		*value = malloc(*len);
		if (*value == NULL)
			return (ENOMEM);
		memset(*value, 0, *len);
		if (equal != NULL) {
			if (strlen(equal) >= PATH_MAX)
				return (ENOMEM);
			strcpy(*value, equal);
		}
	} else {
		/* This parameter is RO. */
		*len = -1;
		if (*equal == '\0')
			return (EINVAL);
		*value = equal;
	}

	return (0);
}

int main(int argc, char *argv[])
{
	struct retval *rv;
	struct gctl_req *req;
	char *param, *value;
	const char *s;
	int c, len;

	req = gctl_get_handle();
	gctl_ro_param(req, "class", -1, "GPT");

	while ((c = getopt(argc, argv, "v")) != -1) {
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
			break;
		}
	}

	while (optind < argc) {
		if (!parse(argv[optind++], &param, &value, &len)) {
			if (len > 0) {
				rv = malloc(sizeof(struct retval));
				rv->param = param;
				rv->value = value;
				rv->retval = retval;
				retval = rv;
				gctl_rw_param(req, param, len, value);
			} else
				gctl_ro_param(req, param, -1, value);
		}
	}

	if (verbose)
		gctl_dump(req, stdout);

	s = gctl_issue(req);
	if (s == NULL) {
		printf("PASS");
		while (retval != NULL) {
			rv = retval->retval;
			printf(" %s=%s", retval->param, retval->value);
			free(retval->value);
			free(retval);
			retval = rv;
		}
		printf("\n");
	} else
		printf("FAIL %s\n", s);

	gctl_free(req);
	return (0);
}