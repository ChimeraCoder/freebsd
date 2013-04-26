
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
/* Simple implementation of strstr for systems without it.   This function is in the public domain.  */

/*

@deftypefn Supplemental char* strstr (const char *@var{string}, const char *@var{sub})

This function searches for the substring @var{sub} in the string
@var{string}, not including the terminating null characters.  A pointer
to the first occurrence of @var{sub} is returned, or @code{NULL} if the
substring is absent.  If @var{sub} points to a string with zero
length, the function returns @var{string}.

@end deftypefn


*/


/* FIXME:  The above description is ANSI compiliant.  This routine has not
   been validated to comply with it.  -fnf */

#include <stddef.h>

extern char *strchr (const char *, int);
extern int strncmp (const void *, const void *, size_t);
extern size_t strlen (const char *);

char *
strstr (const char *s1, const char *s2)
{
  const char *p = s1;
  const size_t len = strlen (s2);

  for (; (p = strchr (p, *s2)) != 0; p++)
    {
      if (strncmp (p, s2, len) == 0)
	return (char *)p;
    }
  return (0);
}