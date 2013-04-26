
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

#include "lib.h"

char *
strconcat(const char *s1, const char *s2)
{
    static char tmp[FILENAME_MAX];

    tmp[0] = '\0';
    strncpy(tmp, s1 ? s1 : s2, FILENAME_MAX);  /* XXX: what if both are NULL? */
    if (s1 && s2)
	strncat(tmp, s2, FILENAME_MAX - strlen(tmp));
    return tmp;
}

/* Get a string parameter as a file spec or as a "contents follow -" spec */
char *
get_dash_string(char **str)
{
    char *s = *str;

    if (*s == '-')
	*str = copy_string_adds_newline(s + 1);
    else
	*str = fileGetContents(s);
    return *str;
}

/* Rather Obvious */
char *
copy_string(const char *str)
{
    return (str ? strdup(str) : NULL);
}

/* Rather Obvious but adds a trailing \n newline */
char *
copy_string_adds_newline(const char *str)
{
    if (str == NULL) {
	return (NULL);
    } else  {
	char *copy;
	size_t line_length;

	line_length = strlen(str) + 2;
	if ((copy = malloc(line_length)) == NULL)
		return (NULL);
	memcpy(copy, str, line_length - 2);
	copy[line_length - 2] = '\n';	/* Adds trailing \n */
	copy[line_length - 1] = '\0';

	return (copy);
   }
}

/* Return TRUE if 'str' ends in suffix 'suff' */
Boolean
suffix(const char *str, const char *suff)
{
    char *idx;
    Boolean ret = FALSE;

    idx = strrchr(str, '.');
    if (idx && !strcmp(idx + 1, suff))
	ret = TRUE;
    return ret;
}

/* Assuming str has a suffix, brutally murder it! */
void
nuke_suffix(char *str)
{
    char *idx;

    idx = strrchr(str, '.');
    if (idx)
	*idx = '\0';  /* Yow!  Don't try this on a const! */
}

/* Lowercase a whole string */
void
str_lowercase(char *str)
{
    while (*str) {
	*str = tolower(*str);
	++str;
    }
}

char *
get_string(char *str, int max, FILE *fp)
{
    int len;

    if (!str)
	return NULL;
    str[0] = '\0';
    while (fgets(str, max, fp)) {
	len = strlen(str);
	while (len && isspace(str[len - 1]))
	    str[--len] = '\0';
	if (len)
	   return str;
    }
    return NULL;
}