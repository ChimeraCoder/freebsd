
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

#include "config.h"

#if defined(LIBC_SCCS) && !defined(lint)
static const char sccsid[] = "@(#)getenv.c	8.1 (Berkeley) 6/4/93";
static const char sccsid[] = "@(#)setenv.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <stdlib.h>
#include <string.h>

/*
 * __findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */
static char *
__findenv(name, offset)
	register char *name;
	int *offset;
{
	extern char **environ;
	register int len;
	register char *np;
	register char **p, *c;

	if (name == NULL || environ == NULL)
		return (NULL);
	for (np = name; *np && *np != '='; ++np)
		continue;
	len = np - name;
	for (p = environ; (c = *p) != NULL; ++p)
		if (strncmp(c, name, len) == 0 && c[len] == '=') {
			*offset = p - environ;
			return (c + len + 1);
		}
	return (NULL);
}

#ifndef HAVE_SETENV
/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 *
 * PUBLIC: #ifndef HAVE_SETENV
 * PUBLIC: int setenv __P((const char *, const char *, int));
 * PUBLIC: #endif
 */
setenv(name, value, rewrite)
	register char *name;
	register char *value;
	int rewrite;
{
	extern char **environ;
	static int alloced;			/* if allocated space before */
	register char *c;
	int l_value, offset;

	if (*value == '=')			/* no `=' in value */
		++value;
	l_value = strlen(value);
	if ((c = __findenv(name, &offset))) {	/* find if already exists */
		if (!rewrite)
			return (0);
		if (strlen(c) >= l_value) {	/* old larger; copy over */
			while (*c++ = *value++);
			return (0);
		}
	} else {					/* create new slot */
		register int cnt;
		register char **p;

		for (p = environ, cnt = 0; *p; ++p, ++cnt);
		if (alloced) {			/* just increase size */
			environ = (char **)realloc((char *)environ,
			    (size_t)(sizeof(char *) * (cnt + 2)));
			if (!environ)
				return (-1);
		}
		else {				/* get new space */
			alloced = 1;		/* copy old entries into it */
			p = malloc((size_t)(sizeof(char *) * (cnt + 2)));
			if (!p)
				return (-1);
			memmove(p, environ, cnt * sizeof(char *));
			environ = p;
		}
		environ[cnt + 1] = NULL;
		offset = cnt;
	}
	for (c = (char *)name; *c && *c != '='; ++c);	/* no `=' in name */
	if (!(environ[offset] =			/* name + `=' + value */
	    malloc((size_t)((int)(c - name) + l_value + 2))))
		return (-1);
	for (c = environ[offset]; (*c = *name++) && *c != '='; ++c);
	for (*c++ = '='; *c++ = *value++;);
	return (0);
}
#endif

#ifndef HAVE_UNSETENV
/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 *
 * PUBLIC: #ifndef HAVE_UNSETENV
 * PUBLIC: void unsetenv __P((const char *));
 * PUBLIC: #endif
 */
void
unsetenv(name)
	char *name;
{
	extern char **environ;
	register char **p;
	int offset;

	while (__findenv(name, &offset))	/* if set multiple times */
		for (p = &environ[offset];; ++p)
			if (!(*p = *(p + 1)))
				break;
}
#endif