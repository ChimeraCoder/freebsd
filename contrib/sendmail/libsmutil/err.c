
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

SM_RCSID("@(#)$Id: err.c,v 8.5 2001/09/11 04:04:55 gshapiro Exp $")

#include <ctype.h>

/*VARARGS1*/
void
#ifdef __STDC__
message(const char *msg, ...)
#else /* __STDC__ */
message(msg, va_alist)
	const char *msg;
	va_dcl
#endif /* __STDC__ */
{
	const char *m;
	SM_VA_LOCAL_DECL

	m = msg;
	if (isascii(m[0]) && isdigit(m[0]) &&
	    isascii(m[1]) && isdigit(m[1]) &&
	    isascii(m[2]) && isdigit(m[2]) && m[3] == ' ')
		m += 4;
	SM_VA_START(ap, msg);
	(void) vfprintf(stderr, m, ap);
	SM_VA_END(ap);
	(void) fprintf(stderr, "\n");
}

/*VARARGS1*/
void
#ifdef __STDC__
syserr(const char *msg, ...)
#else /* __STDC__ */
syserr(msg, va_alist)
	const char *msg;
	va_dcl
#endif /* __STDC__ */
{
	const char *m;
	SM_VA_LOCAL_DECL

	m = msg;
	if (isascii(m[0]) && isdigit(m[0]) &&
	    isascii(m[1]) && isdigit(m[1]) &&
	    isascii(m[2]) && isdigit(m[2]) && m[3] == ' ')
		m += 4;
	SM_VA_START(ap, msg);
	(void) vfprintf(stderr, m, ap);
	SM_VA_END(ap);
	(void) fprintf(stderr, "\n");
}