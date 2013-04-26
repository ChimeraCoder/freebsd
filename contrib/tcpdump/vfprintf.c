
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

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/vfprintf.c,v 1.6 2003-11-16 09:36:45 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include "interface.h"

/*
 * Stock 4.3 doesn't have vfprintf.
 * This routine is due to Chris Torek.
 */
vfprintf(f, fmt, args)
	FILE *f;
	char *fmt;
	va_list args;
{
	int ret;

	if ((f->_flag & _IOWRT) == 0) {
		if (f->_flag & _IORW)
			f->_flag |= _IOWRT;
		else
			return EOF;
	}
	ret = _doprnt(fmt, args, f);
	return ferror(f) ? EOF : ret;
}