
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

#include <sendmail.h>

SM_RCSID("@(#)$Id: snprintf.c,v 8.44 2001/09/11 04:04:56 gshapiro Exp $")

/*
**  SHORTENSTRING -- return short version of a string
**
**	If the string is already short, just return it.  If it is too
**	long, return the head and tail of the string.
**
**	Parameters:
**		s -- the string to shorten.
**		m -- the max length of the string (strlen()).
**
**	Returns:
**		Either s or a short version of s.
*/

char *
shortenstring(s, m)
	register const char *s;
	size_t m;
{
	size_t l;
	static char buf[MAXSHORTSTR + 1];

	l = strlen(s);
	if (l < m)
		return (char *) s;
	if (m > MAXSHORTSTR)
		m = MAXSHORTSTR;
	else if (m < 10)
	{
		if (m < 5)
		{
			(void) sm_strlcpy(buf, s, m + 1);
			return buf;
		}
		(void) sm_strlcpy(buf, s, m - 2);
		(void) sm_strlcat(buf, "...", sizeof buf);
		return buf;
	}
	m = (m - 3) / 2;
	(void) sm_strlcpy(buf, s, m + 1);
	(void) sm_strlcat2(buf, "...", s + l - m, sizeof buf);
	return buf;
}