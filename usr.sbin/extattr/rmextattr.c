
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

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/extattr.h>

#include <libgen.h>
#include <libutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vis.h>
#include <err.h>
#include <errno.h>

static enum { EADUNNO, EAGET, EASET, EARM, EALS } what = EADUNNO;

static void __dead2
usage(void) 
{

	switch (what) {
	case EAGET:
		fprintf(stderr, "usage: getextattr [-fhqsx] attrnamespace");
		fprintf(stderr, " attrname filename ...\n");
		exit(-1);
	case EASET:
		fprintf(stderr, "usage: setextattr [-fhnq] attrnamespace");
		fprintf(stderr, " attrname attrvalue filename ...\n");
		exit(-1);
	case EARM:
		fprintf(stderr, "usage: rmextattr [-fhq] attrnamespace");
		fprintf(stderr, " attrname filename ...\n");
		exit(-1);
	case EALS:
		fprintf(stderr, "usage: lsextattr [-fhq] attrnamespace");
		fprintf(stderr, " filename ...\n");
		exit(-1);
	case EADUNNO:
	default:
		fprintf(stderr, "usage: (getextattr|lsextattr|rmextattr");
		fprintf(stderr, "|setextattr)\n");
		exit (-1);
	}
}

static void
mkbuf(char **buf, int *oldlen, int newlen)
{

	if (*oldlen >= newlen)
		return;
	if (*buf != NULL)
		free(*buf);
	*buf = malloc(newlen);
	if (*buf == NULL)
		err(1, "malloc");
	*oldlen = newlen;
	return;
}

int
main(int argc, char *argv[])
{
	char	*buf, *visbuf, *p;

	const char *options, *attrname;
	size_t	len;
	ssize_t	ret;
	int	 buflen, visbuflen, ch, error, i, arg_counter, attrnamespace,
		 minargc;

	int	flag_force = 0;
	int	flag_nofollow = 0;
	int	flag_null = 0;
	int	flag_quiet = 0;
	int	flag_string = 0;
	int	flag_hex = 0;

	visbuflen = buflen = 0;
	visbuf = buf = NULL;

	p = basename(argv[0]);
	if (p == NULL)
		p = argv[0];
	if (!strcmp(p, "getextattr")) {
		what = EAGET;
		options = "fhqsx";
		minargc = 3;
	} else if (!strcmp(p, "setextattr")) {
		what = EASET;
		options = "fhnq";
		minargc = 4;
	} else if (!strcmp(p, "rmextattr")) {
		what = EARM;
		options = "fhq";
		minargc = 3;
	} else if (!strcmp(p, "lsextattr")) {
		what = EALS;
		options = "fhq";
		minargc = 2;
	} else {
		usage();
	}

	while ((ch = getopt(argc, argv, options)) != -1) {
		switch (ch) {
		case 'f':
			flag_force = 1;
			break;
		case 'h':
			flag_nofollow = 1;
			break;
		case 'n':
			flag_null = 1;
			break;
		case 'q':
			flag_quiet = 1;
			break;
		case 's':
			flag_string = 1;
			break;
		case 'x':
			flag_hex = 1;
			break;
		case '?':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < minargc)
		usage();

	error = extattr_string_to_namespace(argv[0], &attrnamespace);
	if (error)
		err(-1, "%s", argv[0]);
	argc--; argv++;

	if (what != EALS) {
		attrname = argv[0];
		argc--; argv++;
	} else
		attrname = NULL;

	if (what == EASET) {
		mkbuf(&buf, &buflen, strlen(argv[0]) + 1);
		strcpy(buf, argv[0]);
		argc--; argv++;
	}

	for (arg_counter = 0; arg_counter < argc; arg_counter++) {
		switch (what) {
		case EARM:
			if (flag_nofollow)
				error = extattr_delete_link(argv[arg_counter],
				    attrnamespace, attrname);
			else
				error = extattr_delete_file(argv[arg_counter],
				    attrnamespace, attrname);
			if (error >= 0)
				continue;
			break;
		case EASET:
			len = strlen(buf) + flag_null;
			if (flag_nofollow)
				ret = extattr_set_link(argv[arg_counter],
				    attrnamespace, attrname, buf, len);
			else
				ret = extattr_set_file(argv[arg_counter],
				    attrnamespace, attrname, buf, len);
			if (ret >= 0) {
				if ((size_t)ret != len && !flag_quiet) {
					warnx("Set %zd bytes of %zu for %s",
					    ret, len, attrname);
				}
				continue;
			}
			break;
		case EALS:
			if (flag_nofollow)
				ret = extattr_list_link(argv[arg_counter],
				    attrnamespace, NULL, 0);
			else
				ret = extattr_list_file(argv[arg_counter],
				    attrnamespace, NULL, 0);
			if (ret < 0)
				break;
			mkbuf(&buf, &buflen, ret);
			if (flag_nofollow)
				ret = extattr_list_link(argv[arg_counter],
				    attrnamespace, buf, buflen);
			else
				ret = extattr_list_file(argv[arg_counter],
				    attrnamespace, buf, buflen);
			if (ret < 0)
				break;
			if (!flag_quiet)
				printf("%s\t", argv[arg_counter]);
			for (i = 0; i < ret; i += ch + 1) {
			    /* The attribute name length is unsigned. */
			    ch = (unsigned char)buf[i];
			    printf("%s%*.*s", i ? "\t" : "",
				ch, ch, buf + i + 1);
			}
			if (!flag_quiet || ret > 0)
				printf("\n");
			continue;
		case EAGET:
			if (flag_nofollow)
				ret = extattr_get_link(argv[arg_counter],
				    attrnamespace, attrname, NULL, 0);
			else
				ret = extattr_get_file(argv[arg_counter],
				    attrnamespace, attrname, NULL, 0);
			if (ret < 0)
				break;
			mkbuf(&buf, &buflen, ret);
			if (flag_nofollow)
				ret = extattr_get_link(argv[arg_counter],
				    attrnamespace, attrname, buf, buflen);
			else
				ret = extattr_get_file(argv[arg_counter],
				    attrnamespace, attrname, buf, buflen);
			if (ret < 0)
				break;
			if (!flag_quiet)
				printf("%s\t", argv[arg_counter]);
			if (flag_string) {
				mkbuf(&visbuf, &visbuflen, ret * 4 + 1);
				strvisx(visbuf, buf, ret,
				    VIS_SAFE | VIS_WHITE);
				printf("\"%s\"\n", visbuf);
				continue;
			} else if (flag_hex) {
				for (i = 0; i < ret; i++)
					printf("%s%02x", i ? " " : "",
					    buf[i]);
				printf("\n");
				continue;
			} else {
				fwrite(buf, ret, 1, stdout);
				printf("\n");
				continue;
			}
		default:
			break;
		}
		if (!flag_quiet) 
			warn("%s: failed", argv[arg_counter]);
		if (flag_force)
			continue;
		return(1);
	}
	return (0);
}