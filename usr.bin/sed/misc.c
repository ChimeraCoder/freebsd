
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

#ifndef lint
static const char sccsid[] = "@(#)misc.c	8.1 (Berkeley) 6/6/93";
#endif

#include <sys/types.h>

#include <err.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "extern.h"

/*
 * Return a string for a regular expression error passed.  This is overkill,
 * because of the silly semantics of regerror (we can never know the size of
 * the buffer).
 */
char *
strregerror(int errcode, regex_t *preg)
{
	static char *oe;
	size_t s;

	if (oe != NULL)
		free(oe);
	s = regerror(errcode, preg, NULL, 0);
	if ((oe = malloc(s)) == NULL)
		err(1, "malloc");
	(void)regerror(errcode, preg, oe, s);
	return (oe);
}