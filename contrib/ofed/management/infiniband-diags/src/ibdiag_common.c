
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

/**
 * Define common functions which can be included in the various C based diags.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ibdiag_common.h"

int ibdebug;

void
iberror(const char *fn, char *msg, ...)
{
	char buf[512], *s;
	va_list va;
	int n;

	va_start(va, msg);
	n = vsprintf(buf, msg, va);
	va_end(va);
	buf[n] = 0;

	if ((s = strrchr(argv0, '/')))
		argv0 = s + 1;

	if (ibdebug)
		printf("%s: iberror: [pid %d] %s: failed: %s\n", argv0, getpid(), fn, buf);
	else
		printf("%s: iberror: failed: %s\n", argv0, buf);

	exit(-1);
}