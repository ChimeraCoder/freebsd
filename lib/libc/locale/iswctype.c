
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

#include <wctype.h>

#undef iswalnum
int
iswalnum(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_A|_CTYPE_D));
}

#undef iswalpha
int
iswalpha(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_A));
}

#undef iswascii
int
iswascii(wc)
	wint_t wc;
{
	return ((wc & ~0x7F) == 0);
}

#undef iswblank
int
iswblank(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_B));
}

#undef iswcntrl
int
iswcntrl(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_C));
}

#undef iswdigit
int
iswdigit(wc)
	wint_t wc;
{
	return (__isctype(wc, _CTYPE_D));
}

#undef iswgraph
int
iswgraph(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_G));
}

#undef iswhexnumber 
int
iswhexnumber(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_X));
}

#undef iswideogram
int
iswideogram(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_I));
}

#undef iswlower
int
iswlower(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_L));
}

#undef iswnumber
int
iswnumber(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_D));
}

#undef iswphonogram	
int
iswphonogram(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_Q));
}

#undef iswprint
int
iswprint(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_R));
}

#undef iswpunct
int
iswpunct(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_P));
}

#undef iswrune
int
iswrune(wc)
	wint_t wc;
{
	return (__istype(wc, 0xFFFFFF00L));
}

#undef iswspace
int
iswspace(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_S));
}

#undef iswspecial
int
iswspecial(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_T));
}

#undef iswupper
int
iswupper(wc)
	wint_t wc;
{
	return (__istype(wc, _CTYPE_U));
}

#undef iswxdigit
int
iswxdigit(wc)
	wint_t wc;
{
	return (__isctype(wc, _CTYPE_X));
}

#undef towlower
wint_t
towlower(wc)
	wint_t wc;
{
        return (__tolower(wc));
}

#undef towupper
wint_t
towupper(wc)
	wint_t wc;
{
        return (__toupper(wc));
}