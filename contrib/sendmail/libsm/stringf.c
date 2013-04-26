
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
SM_RCSID("@(#)$Id: stringf.c,v 1.15 2001/09/11 04:04:49 gshapiro Exp $")
#include <errno.h>
#include <stdio.h>
#include <sm/exc.h>
#include <sm/heap.h>
#include <sm/string.h>
#include <sm/varargs.h>

/*
**  SM_STRINGF_X -- printf() to dynamically allocated string.
**
**	Takes the same arguments as printf.
**	It returns a pointer to a dynamically allocated string
**	containing the text that printf would print to standard output.
**	It raises an exception on error.
**	The name comes from a PWB Unix function called stringf.
**
**	Parameters:
**		fmt -- format string.
**		... -- arguments for format.
**
**	Returns:
**		Pointer to a dynamically allocated string.
**
**	Exceptions:
**		F:sm_heap -- out of memory (via sm_vstringf_x()).
*/

char *
#if SM_VA_STD
sm_stringf_x(const char *fmt, ...)
#else /* SM_VA_STD */
sm_stringf_x(fmt, va_alist)
	const char *fmt;
	va_dcl
#endif /* SM_VA_STD */
{
	SM_VA_LOCAL_DECL
	char *s;

	SM_VA_START(ap, fmt);
	s = sm_vstringf_x(fmt, ap);
	SM_VA_END(ap);
	return s;
}

/*
**  SM_VSTRINGF_X -- printf() to dynamically allocated string.
**
**	Parameters:
**		fmt -- format string.
**		ap -- arguments for format.
**
**	Returns:
**		Pointer to a dynamically allocated string.
**
**	Exceptions:
**		F:sm_heap -- out of memory
*/

char *
sm_vstringf_x(fmt, ap)
	const char *fmt;
	SM_VA_LOCAL_DECL
{
	char *s;

	sm_vasprintf(&s, fmt, ap);
	if (s == NULL)
	{
		if (errno == ENOMEM)
			sm_exc_raise_x(&SmHeapOutOfMemory);
		sm_exc_raisenew_x(&SmEtypeOs, errno, "sm_vasprintf", NULL);
	}
	return s;
}