
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)setlocale.c	8.1 (Berkeley) 7/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <paths.h>	/* for _PATH_LOCALE */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "collate.h"
#include "lmonetary.h"	/* for __monetary_load_locale() */
#include "lnumeric.h"	/* for __numeric_load_locale() */
#include "lmessages.h"	/* for __messages_load_locale() */
#include "setlocale.h"
#include "ldpart.h"
#include "../stdtime/timelocal.h" /* for __time_load_locale() */

/*
 * Category names for getenv()
 */
static const char categories[_LC_LAST][12] = {
    "LC_ALL",
    "LC_COLLATE",
    "LC_CTYPE",
    "LC_MONETARY",
    "LC_NUMERIC",
    "LC_TIME",
    "LC_MESSAGES",
};

/*
 * Current locales for each category
 */
static char current_categories[_LC_LAST][ENCODING_LEN + 1] = {
    "C",
    "C",
    "C",
    "C",
    "C",
    "C",
    "C",
};

/*
 * Path to locale storage directory
 */
char	*_PathLocale;

/*
 * The locales we are going to try and load
 */
static char new_categories[_LC_LAST][ENCODING_LEN + 1];
static char saved_categories[_LC_LAST][ENCODING_LEN + 1];

static char current_locale_string[_LC_LAST * (ENCODING_LEN + 1/*"/"*/ + 1)];

static char	*currentlocale(void);
static char	*loadlocale(int);
const char *__get_locale_env(int);

char *
setlocale(category, locale)
	int category;
	const char *locale;
{
	int i, j, len, saverr;
        const char *env, *r;

	if (category < LC_ALL || category >= _LC_LAST) {
		errno = EINVAL;
		return (NULL);
	}

	if (locale == NULL)
		return (category != LC_ALL ?
		    current_categories[category] : currentlocale());

	/*
	 * Default to the current locale for everything.
	 */
	for (i = 1; i < _LC_LAST; ++i)
		(void)strcpy(new_categories[i], current_categories[i]);

	/*
	 * Now go fill up new_categories from the locale argument
	 */
	if (!*locale) {
		if (category == LC_ALL) {
			for (i = 1; i < _LC_LAST; ++i) {
				env = __get_locale_env(i);
				if (strlen(env) > ENCODING_LEN) {
					errno = EINVAL;
					return (NULL);
				}
				(void)strcpy(new_categories[i], env);
			}
		} else {
			env = __get_locale_env(category);
			if (strlen(env) > ENCODING_LEN) {
				errno = EINVAL;
				return (NULL);
			}
			(void)strcpy(new_categories[category], env);
		}
	} else if (category != LC_ALL) {
		if (strlen(locale) > ENCODING_LEN) {
			errno = EINVAL;
			return (NULL);
		}
		(void)strcpy(new_categories[category], locale);
	} else {
		if ((r = strchr(locale, '/')) == NULL) {
			if (strlen(locale) > ENCODING_LEN) {
				errno = EINVAL;
				return (NULL);
			}
			for (i = 1; i < _LC_LAST; ++i)
				(void)strcpy(new_categories[i], locale);
		} else {
			for (i = 1; r[1] == '/'; ++r)
				;
			if (!r[1]) {
				errno = EINVAL;
				return (NULL);	/* Hmm, just slashes... */
			}
			do {
				if (i == _LC_LAST)
					break;  /* Too many slashes... */
				if ((len = r - locale) > ENCODING_LEN) {
					errno = EINVAL;
					return (NULL);
				}
				(void)strlcpy(new_categories[i], locale,
					      len + 1);
				i++;
				while (*r == '/')
					r++;
				locale = r;
				while (*r && *r != '/')
					r++;
			} while (*locale);
			while (i < _LC_LAST) {
				(void)strcpy(new_categories[i],
					     new_categories[i-1]);
				i++;
			}
		}
	}

	if (category != LC_ALL)
		return (loadlocale(category));

	for (i = 1; i < _LC_LAST; ++i) {
		(void)strcpy(saved_categories[i], current_categories[i]);
		if (loadlocale(i) == NULL) {
			saverr = errno;
			for (j = 1; j < i; j++) {
				(void)strcpy(new_categories[j],
					     saved_categories[j]);
				if (loadlocale(j) == NULL) {
					(void)strcpy(new_categories[j], "C");
					(void)loadlocale(j);
				}
			}
			errno = saverr;
			return (NULL);
		}
	}
	return (currentlocale());
}

static char *
currentlocale()
{
	int i;

	(void)strcpy(current_locale_string, current_categories[1]);

	for (i = 2; i < _LC_LAST; ++i)
		if (strcmp(current_categories[1], current_categories[i])) {
			for (i = 2; i < _LC_LAST; ++i) {
				(void)strcat(current_locale_string, "/");
				(void)strcat(current_locale_string,
					     current_categories[i]);
			}
			break;
		}
	return (current_locale_string);
}

static char *
loadlocale(category)
	int category;
{
	char *new = new_categories[category];
	char *old = current_categories[category];
	int (*func)(const char *);
	int saved_errno;

	if ((new[0] == '.' &&
	     (new[1] == '\0' || (new[1] == '.' && new[2] == '\0'))) ||
	    strchr(new, '/') != NULL) {
		errno = EINVAL;
		return (NULL);
	}

	saved_errno = errno;
	errno = __detect_path_locale();
	if (errno != 0)
		return (NULL);
	errno = saved_errno;

	switch (category) {
	case LC_CTYPE:
		func = __wrap_setrunelocale;
		break;
	case LC_COLLATE:
		func = __collate_load_tables;
		break;
	case LC_TIME:
		func = __time_load_locale;
		break;
	case LC_NUMERIC:
		func = __numeric_load_locale;
		break;
	case LC_MONETARY:
		func = __monetary_load_locale;
		break;
	case LC_MESSAGES:
		func = __messages_load_locale;
		break;
	default:
		errno = EINVAL;
		return (NULL);
	}

	if (strcmp(new, old) == 0)
		return (old);

	if (func(new) != _LDP_ERROR) {
		(void)strcpy(old, new);
		(void)strcpy(__xlocale_global_locale.components[category-1]->locale, new);
		return (old);
	}

	return (NULL);
}

const char *
__get_locale_env(category)
        int category;
{
        const char *env;

        /* 1. check LC_ALL. */
        env = getenv(categories[0]);

        /* 2. check LC_* */
	if (env == NULL || !*env)
                env = getenv(categories[category]);

        /* 3. check LANG */
	if (env == NULL || !*env)
                env = getenv("LANG");

        /* 4. if none is set, fall to "C" */
	if (env == NULL || !*env)
                env = "C";

	return (env);
}

/*
 * Detect locale storage location and store its value to _PathLocale variable
 */
int
__detect_path_locale(void)
{
	if (_PathLocale == NULL) {
		char *p = getenv("PATH_LOCALE");

		if (p != NULL && !issetugid()) {
			if (strlen(p) + 1/*"/"*/ + ENCODING_LEN +
			    1/*"/"*/ + CATEGORY_LEN >= PATH_MAX)
				return (ENAMETOOLONG);
			_PathLocale = strdup(p);
			if (_PathLocale == NULL)
				return (errno == 0 ? ENOMEM : errno);
		} else
			_PathLocale = _PATH_LOCALE;
	}
	return (0);
}