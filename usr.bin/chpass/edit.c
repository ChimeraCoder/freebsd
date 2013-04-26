
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)edit.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pw_scan.h>
#include <libutil.h>

#include "chpass.h"

static int display(const char *tfn, struct passwd *pw);
static struct passwd *verify(const char *tfn, struct passwd *pw);

struct passwd *
edit(const char *tfn, struct passwd *pw)
{
	struct passwd *npw;
	char *line;
	size_t len;

	if (display(tfn, pw) == -1)
		return (NULL);
	for (;;) {
		switch (pw_edit(1)) {
		case -1:
			return (NULL);
		case 0:
			return (pw_dup(pw));
		default:
			break;
		}
		if ((npw = verify(tfn, pw)) != NULL)
			return (npw);
		free(npw);
		printf("re-edit the password file? ");
		fflush(stdout);
		if ((line = fgetln(stdin, &len)) == NULL) {
			warn("fgetln()");
			return (NULL);
		}
		if (len > 0 && (*line == 'N' || *line == 'n'))
			return (NULL);
	}
}

/*
 * display --
 *	print out the file for the user to edit; strange side-effect:
 *	set conditional flag if the user gets to edit the shell.
 */
static int
display(const char *tfn, struct passwd *pw)
{
	FILE *fp;
	char *bp, *gecos, *p;

	if ((fp = fopen(tfn, "w")) == NULL) {
		warn("%s", tfn);
		return (-1);
	}

	(void)fprintf(fp,
	    "#Changing user information for %s.\n", pw->pw_name);
	if (master_mode) {
		(void)fprintf(fp, "Login: %s\n", pw->pw_name);
		(void)fprintf(fp, "Password: %s\n", pw->pw_passwd);
		(void)fprintf(fp, "Uid [#]: %lu\n", (unsigned long)pw->pw_uid);
		(void)fprintf(fp, "Gid [# or name]: %lu\n",
		    (unsigned long)pw->pw_gid);
		(void)fprintf(fp, "Change [month day year]: %s\n",
		    ttoa(pw->pw_change));
		(void)fprintf(fp, "Expire [month day year]: %s\n",
		    ttoa(pw->pw_expire));
		(void)fprintf(fp, "Class: %s\n", pw->pw_class);
		(void)fprintf(fp, "Home directory: %s\n", pw->pw_dir);
		(void)fprintf(fp, "Shell: %s\n",
		    *pw->pw_shell ? pw->pw_shell : _PATH_BSHELL);
	}
	/* Only admin can change "restricted" shells. */
#if 0
	else if (ok_shell(pw->pw_shell))
		/*
		 * Make shell a restricted field.  Ugly with a
		 * necklace, but there's not much else to do.
		 */
#else
	else if ((!list[E_SHELL].restricted && ok_shell(pw->pw_shell)) ||
	    master_mode)
		/*
		 * If change not restrict (table.c) and standard shell
		 *	OR if root, then allow editing of shell.
		 */
#endif
		(void)fprintf(fp, "Shell: %s\n",
		    *pw->pw_shell ? pw->pw_shell : _PATH_BSHELL);
	else
		list[E_SHELL].restricted = 1;

	if ((bp = gecos = strdup(pw->pw_gecos)) == NULL) {
		warn(NULL);
		fclose(fp);
		return (-1);
	}

	p = strsep(&bp, ",");
	p = strdup(p ? p : "");
	list[E_NAME].save = p;
	if (!list[E_NAME].restricted || master_mode)
	  (void)fprintf(fp, "Full Name: %s\n", p);

	p = strsep(&bp, ",");
	p = strdup(p ? p : "");
	list[E_LOCATE].save = p;
	if (!list[E_LOCATE].restricted || master_mode)
	  (void)fprintf(fp, "Office Location: %s\n", p);

	p = strsep(&bp, ",");
	p = strdup(p ? p : "");
	list[E_BPHONE].save = p;
	if (!list[E_BPHONE].restricted || master_mode)
	  (void)fprintf(fp, "Office Phone: %s\n", p);

	p = strsep(&bp, ",");
	p = strdup(p ? p : "");
	list[E_HPHONE].save = p;
	if (!list[E_HPHONE].restricted || master_mode)
	  (void)fprintf(fp, "Home Phone: %s\n", p);

	bp = strdup(bp ? bp : "");
	list[E_OTHER].save = bp;
	if (!list[E_OTHER].restricted || master_mode)
	  (void)fprintf(fp, "Other information: %s\n", bp);

	free(gecos);

	(void)fchown(fileno(fp), getuid(), getgid());
	(void)fclose(fp);
	return (0);
}

static struct passwd *
verify(const char *tfn, struct passwd *pw)
{
	struct passwd *npw;
	ENTRY *ep;
	char *buf, *p, *val;
	struct stat sb;
	FILE *fp;
	int line;
	size_t len;

	if ((pw = pw_dup(pw)) == NULL)
		return (NULL);
	if ((fp = fopen(tfn, "r")) == NULL ||
	    fstat(fileno(fp), &sb) == -1) {
		warn("%s", tfn);
		free(pw);
		return (NULL);
	}
	if (sb.st_size == 0) {
		warnx("corrupted temporary file");
		fclose(fp);
		free(pw);
		return (NULL);
	}
	val = NULL;
	for (line = 1; (buf = fgetln(fp, &len)) != NULL; ++line) {
		if (*buf == '\0' || *buf == '#')
			continue;
		while (len > 0 && isspace(buf[len - 1]))
			--len;
		for (ep = list;; ++ep) {
			if (!ep->prompt) {
				warnx("%s: unrecognized field on line %d",
				    tfn, line);
				goto bad;
			}
			if (ep->len > len)
				continue;
			if (strncasecmp(buf, ep->prompt, ep->len) != 0)
				continue;
			if (ep->restricted && !master_mode) {
				warnx("%s: you may not change the %s field",
				    tfn, ep->prompt);
				goto bad;
			}
			for (p = buf; p < buf + len && *p != ':'; ++p)
				/* nothing */ ;
			if (*p != ':') {
				warnx("%s: line %d corrupted", tfn, line);
				goto bad;
			}
			while (++p < buf + len && isspace(*p))
				/* nothing */ ;
			free(val);
			asprintf(&val, "%.*s", (int)(buf + len - p), p);
			if (val == NULL)
				goto bad;
			if (ep->except && strpbrk(val, ep->except)) {
				warnx("%s: invalid character in \"%s\" field '%s'",
				    tfn, ep->prompt, val);
				goto bad;
			}
			if ((ep->func)(val, pw, ep))
				goto bad;
			break;
		}
	}
	free(val);
	fclose(fp);

	/* Build the gecos field. */
	len = asprintf(&p, "%s,%s,%s,%s,%s", list[E_NAME].save,
	    list[E_LOCATE].save, list[E_BPHONE].save,
	    list[E_HPHONE].save, list[E_OTHER].save);
	if (p == NULL) {
		warn("asprintf()");
		free(pw);
		return (NULL);
	}
	while (len > 0 && p[len - 1] == ',')
		p[--len] = '\0';
	pw->pw_gecos = p;
	buf = pw_make(pw);
	free(pw);
	free(p);
	if (buf == NULL) {
		warn("pw_make()");
		return (NULL);
	}
	npw = pw_scan(buf, PWSCAN_WARN|PWSCAN_MASTER);
	free(buf);
	return (npw);
bad:
	free(pw);
	free(val);
	fclose(fp);
	return (NULL);
}