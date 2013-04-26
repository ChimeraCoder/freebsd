
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
static char sccsid[] = "@(#)isctype.c	8.3 (Berkeley) 2/24/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <ctype.h>

#undef digittoint
int
digittoint(c)
	int c;
{
	return (__sbmaskrune(c, 0xFF));
}

#undef isalnum
int
isalnum(c)
	int c;
{
	return (__sbistype(c, _CTYPE_A|_CTYPE_D));
}

#undef isalpha
int
isalpha(c)
	int c;
{
	return (__sbistype(c, _CTYPE_A));
}

#undef isascii
int
isascii(c)
	int c;
{
	return ((c & ~0x7F) == 0);
}

#undef isblank
int
isblank(c)
	int c;
{
	return (__sbistype(c, _CTYPE_B));
}

#undef iscntrl
int
iscntrl(c)
	int c;
{
	return (__sbistype(c, _CTYPE_C));
}

#undef isdigit
int
isdigit(c)
	int c;
{
	return (__isctype(c, _CTYPE_D));
}

#undef isgraph
int
isgraph(c)
	int c;
{
	return (__sbistype(c, _CTYPE_G));
}

#undef ishexnumber 
int
ishexnumber(c)
	int c;
{
	return (__sbistype(c, _CTYPE_X));
}

#undef isideogram
int
isideogram(c)
	int c;
{
	return (__sbistype(c, _CTYPE_I));
}

#undef islower
int
islower(c)
	int c;
{
	return (__sbistype(c, _CTYPE_L));
}

#undef isnumber
int
isnumber(c)
	int c;
{
	return (__sbistype(c, _CTYPE_D));
}

#undef isphonogram	
int
isphonogram(c)
	int c;
{
	return (__sbistype(c, _CTYPE_Q));
}

#undef isprint
int
isprint(c)
	int c;
{
	return (__sbistype(c, _CTYPE_R));
}

#undef ispunct
int
ispunct(c)
	int c;
{
	return (__sbistype(c, _CTYPE_P));
}

#undef isrune
int
isrune(c)
	int c;
{
	return (__sbistype(c, 0xFFFFFF00L));
}

#undef isspace
int
isspace(c)
	int c;
{
	return (__sbistype(c, _CTYPE_S));
}

#undef isspecial
int
isspecial(c)
	int c;
{
	return (__sbistype(c, _CTYPE_T));
}

#undef isupper
int
isupper(c)
	int c;
{
	return (__sbistype(c, _CTYPE_U));
}

#undef isxdigit
int
isxdigit(c)
	int c;
{
	return (__isctype(c, _CTYPE_X));
}

#undef toascii
int
toascii(c)
	int c;
{
	return (c & 0x7F);
}

#undef tolower
int
tolower(c)
	int c;
{
	return (__sbtolower(c));
}

#undef toupper
int
toupper(c)
	int c;
{
	return (__sbtoupper(c));
}