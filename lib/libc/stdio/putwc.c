
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

#include "namespace.h"
#include <stdio.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#include "xlocale_private.h"

#undef putwc

/*
 * Synonym for fputwc(). The only difference is that putwc(), if it is a
 * macro, may evaluate `fp' more than once.
 */
wint_t
putwc_l(wchar_t wc, FILE *fp, locale_t locale)
{
	FIX_LOCALE(locale);
	return (fputwc_l(wc, fp, locale));
}
wint_t
putwc(wchar_t wc, FILE *fp)
{
	return putwc_l(wc, fp, __get_locale());
}