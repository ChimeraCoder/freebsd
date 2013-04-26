
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

#include <sys/queue.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ar.h"

static void	bsdar_vwarnc(struct bsdar *, int code,
		    const char *fmt, va_list ap);
static void	bsdar_verrc(struct bsdar *bsdar, int code,
		    const char *fmt, va_list ap);

static void
bsdar_vwarnc(struct bsdar *bsdar, int code, const char *fmt, va_list ap)
{

	fprintf(stderr, "%s: warning: ", bsdar->progname);
	vfprintf(stderr, fmt, ap);
	if (code != 0)
		fprintf(stderr, ": %s", strerror(code));
	fprintf(stderr, "\n");
}

void
bsdar_warnc(struct bsdar *bsdar, int code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	bsdar_vwarnc(bsdar, code, fmt, ap);
	va_end(ap);
}

static void
bsdar_verrc(struct bsdar *bsdar, int code, const char *fmt, va_list ap)
{

	fprintf(stderr, "%s: fatal: ", bsdar->progname);
	vfprintf(stderr, fmt, ap);
	if (code != 0)
		fprintf(stderr, ": %s", strerror(code));
	fprintf(stderr, "\n");
}

void
bsdar_errc(struct bsdar *bsdar, int eval, int code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	bsdar_verrc(bsdar, code, fmt, ap);
	va_end(ap);
	exit(eval);
}