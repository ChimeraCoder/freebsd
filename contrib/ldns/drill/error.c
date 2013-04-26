
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

#include "drill.h"
#include <ldns/ldns.h>

static void
warning_va_list(const char *fmt, va_list args)
{
        fprintf(stderr, "Warning: ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
}

void
warning(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	warning_va_list(fmt, args);
	va_end(args);
}

static void
error_va_list(const char *fmt, va_list args)
{
        fprintf(stderr, "Error: ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
}

void
error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	error_va_list(fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

static void
verbose_va_list(const char *fmt, va_list args)
{
        vfprintf(stdout, fmt, args);
        fprintf(stdout, "\n");
}

/* print stuff */
void
mesg(const char *fmt, ...)
{
	va_list args;
	if (verbosity == -1) {
		return;
	}
	fprintf(stdout, ";; ");
	va_start(args, fmt);
	verbose_va_list(fmt, args);
	va_end(args);
}

/* print stuff when in verbose mode (1) */
void
verbose(const char *fmt, ...)
{
	va_list args;
	if (verbosity < 1) {
		return;
	}

	va_start(args, fmt);
	verbose_va_list(fmt, args);
	va_end(args);
}

/* print stuff when in vverbose mode (2) */
void
vverbose(const char *fmt, ...)
{
	va_list args;
	if (verbosity < 2) {
		return;
	}

	va_start(args, fmt);
	verbose_va_list(fmt, args);
	va_end(args);
}

static void
debug_va_list(const char *fmt, va_list args)
{
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
}

void
debug(const char *fmt, ...)
{
	va_list args;
	fprintf(stderr, "[DEBUG] ");
	va_start(args, fmt);
	debug_va_list(fmt, args);
	va_end(args);
}