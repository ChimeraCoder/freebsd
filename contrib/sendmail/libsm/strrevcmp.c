
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

#include <sm/gen.h>
SM_RCSID("@(#)$Id: strrevcmp.c,v 1.5 2001/09/11 04:04:49 gshapiro Exp $")

#include <sm/config.h>
#include <sm/string.h>
#include <string.h>

/* strcasecmp.c */
extern const unsigned char charmap[];

/*
**  SM_STRREVCASECMP -- compare two strings starting at the end (ignore case)
**
**	Parameters:
**		s1 -- first string.
**		s2 -- second string.
**
**	Returns:
**		strcasecmp(reverse(s1), reverse(s2))
*/

int
sm_strrevcasecmp(s1, s2)
	const char *s1, *s2;
{
	register int i1, i2;

	i1 = strlen(s1) - 1;
	i2 = strlen(s2) - 1;
	while (i1 >= 0 && i2 >= 0 &&
	       charmap[(unsigned char) s1[i1]] ==
	       charmap[(unsigned char) s2[i2]])
	{
		--i1;
		--i2;
	}
	if (i1 < 0)
	{
		if (i2 < 0)
			return 0;
		else
			return -1;
	}
	else
	{
		if (i2 < 0)
			return 1;
		else
			return (charmap[(unsigned char) s1[i1]] -
				charmap[(unsigned char) s2[i2]]);
	}
}
/*
**  SM_STRREVCMP -- compare two strings starting at the end
**
**	Parameters:
**		s1 -- first string.
**		s2 -- second string.
**
**	Returns:
**		strcmp(reverse(s1), reverse(s2))
*/

int
sm_strrevcmp(s1, s2)
	const char *s1, *s2;
{
	register int i1, i2;

	i1 = strlen(s1) - 1;
	i2 = strlen(s2) - 1;
	while (i1 >= 0 && i2 >= 0 && s1[i1] == s2[i2])
	{
		--i1;
		--i2;
	}
	if (i1 < 0)
	{
		if (i2 < 0)
			return 0;
		else
			return -1;
	}
	else
	{
		if (i2 < 0)
			return 1;
		else
			return s1[i1] - s2[i2];
	}
}