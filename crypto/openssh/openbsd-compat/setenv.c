
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

/* OPENBSD ORIGINAL: lib/libc/stdlib/setenv.c */

#include "includes.h"

#if !defined(HAVE_SETENV) || !defined(HAVE_UNSETENV)

#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;
static char **lastenv;				/* last value of environ */

/* OpenSSH Portable: __findenv is from getenv.c rev 1.8, made static */
/*
 * __findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Starts searching within the environmental array at offset.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by putenv(3), setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */
static char *
__findenv(const char *name, int len, int *offset)
{
	extern char **environ;
	int i;
	const char *np;
	char **p, *cp;

	if (name == NULL || environ == NULL)
		return (NULL);
	for (p = environ + *offset; (cp = *p) != NULL; ++p) {
		for (np = name, i = len; i && *cp; i--)
			if (*cp++ != *np++)
				break;
		if (i == 0 && *cp++ == '=') {
			*offset = p - environ;
			return (cp);
		}
	}
	return (NULL);
}

#if 0 /* nothing uses putenv */
/*
 * putenv --
 *	Add a name=value string directly to the environmental, replacing
 *	any current value.
 */
int
putenv(char *str)
{
	char **P, *cp;
	size_t cnt;
	int offset = 0;

	for (cp = str; *cp && *cp != '='; ++cp)
		;
	if (*cp != '=') {
		errno = EINVAL;
		return (-1);			/* missing `=' in string */
	}

	if (__findenv(str, (int)(cp - str), &offset) != NULL) {
		environ[offset++] = str;
		/* could be set multiple times */
		while (__findenv(str, (int)(cp - str), &offset)) {
			for (P = &environ[offset];; ++P)
				if (!(*P = *(P + 1)))
					break;
		}
		return (0);
	}

	/* create new slot for string */
	for (P = environ; *P != NULL; P++)
		;
	cnt = P - environ;
	P = (char **)realloc(lastenv, sizeof(char *) * (cnt + 2));
	if (!P)
		return (-1);
	if (lastenv != environ)
		memcpy(P, environ, cnt * sizeof(char *));
	lastenv = environ = P;
	environ[cnt] = str;
	environ[cnt + 1] = NULL;
	return (0);
}

#endif

#ifndef HAVE_SETENV
/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */
int
setenv(const char *name, const char *value, int rewrite)
{
	char *C, **P;
	const char *np;
	int l_value, offset = 0;

	for (np = name; *np && *np != '='; ++np)
		;
#ifdef notyet
	if (*np) {
		errno = EINVAL;
		return (-1);			/* has `=' in name */
	}
#endif

	l_value = strlen(value);
	if ((C = __findenv(name, (int)(np - name), &offset)) != NULL) {
		int tmpoff = offset + 1;
		if (!rewrite)
			return (0);
#if 0 /* XXX - existing entry may not be writable */
		if (strlen(C) >= l_value) {	/* old larger; copy over */
			while ((*C++ = *value++))
				;
			return (0);
		}
#endif
		/* could be set multiple times */
		while (__findenv(name, (int)(np - name), &tmpoff)) {
			for (P = &environ[tmpoff];; ++P)
				if (!(*P = *(P + 1)))
					break;
		}
	} else {					/* create new slot */
		size_t cnt;

		for (P = environ; *P != NULL; P++)
			;
		cnt = P - environ;
		P = (char **)realloc(lastenv, sizeof(char *) * (cnt + 2));
		if (!P)
			return (-1);
		if (lastenv != environ)
			memcpy(P, environ, cnt * sizeof(char *));
		lastenv = environ = P;
		offset = cnt;
		environ[cnt + 1] = NULL;
	}
	if (!(environ[offset] =			/* name + `=' + value */
	    malloc((size_t)((int)(np - name) + l_value + 2))))
		return (-1);
	for (C = environ[offset]; (*C = *name++) && *C != '='; ++C)
		;
	for (*C++ = '='; (*C++ = *value++); )
		;
	return (0);
}

#endif /* HAVE_SETENV */

#ifndef HAVE_UNSETENV
/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
int
unsetenv(const char *name)
{
	char **P;
	const char *np;
	int offset = 0;

	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}
	for (np = name; *np && *np != '='; ++np)
		;
	if (*np) {
		errno = EINVAL;
		return (-1);			/* has `=' in name */
	}

	/* could be set multiple times */
	while (__findenv(name, (int)(np - name), &offset)) {
		for (P = &environ[offset];; ++P)
			if (!(*P = *(P + 1)))
				break;
	}
	return (0);
}
#endif /* HAVE_UNSETENV */

#endif /* !defined(HAVE_SETENV) || !defined(HAVE_UNSETENV) */