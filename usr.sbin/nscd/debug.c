
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

#include <stdio.h>

#include "debug.h"

static	int	trace_level = 0;
static	int	trace_level_bk = 0;

void
nscd_trace_in(const char *s, const char *f, int l)
{
	int i;
	if (trace_level < TRACE_WANTED)
	{
		for (i = 0; i < trace_level; ++i)
			printf("\t");

		printf("=> %s\n", s);
	}

	++trace_level;
}

void
nscd_trace_point(const char *f, int l)
{
	int i;

	if (trace_level < TRACE_WANTED)
	{
		for (i = 0; i < trace_level - 1; ++i)
			printf("\t");

		printf("= %s: %d\n", f, l);
	}
}

void
nscd_trace_msg(const char *msg, const char *f, int l)
{
	int i;

	if (trace_level < TRACE_WANTED)
	{
		for (i = 0; i < trace_level - 1; ++i)
			printf("\t");

		printf("= MSG %s, %s: %d\n", msg, f, l);
	}
}

void
nscd_trace_ptr(const char *desc, const void *p, const char *f, int l)
{
	int i;

	if (trace_level < TRACE_WANTED)
	{
		for (i = 0; i < trace_level - 1; ++i)
			printf("\t");

		printf("= PTR %s: %p, %s: %d\n", desc, p, f, l);
	}
}

void
nscd_trace_int(const char *desc, int i, const char *f, int l)
{
	int j;

	if (trace_level < TRACE_WANTED)
	{
		for (j = 0; j < trace_level - 1; ++j)
			printf("\t");

		printf("= INT %s: %i, %s: %d\n",desc, i, f, l);
	}
}

void
nscd_trace_str(const char *desc, const char *s, const char *f, int l)
{
	int i;

	if (trace_level < TRACE_WANTED)
	{
		for (i = 0; i < trace_level - 1; ++i)
			printf("\t");

		printf("= STR %s: '%s', %s: %d\n", desc, s, f, l);
	}
}

void
nscd_trace_out(const char *s, const char *f, int l)
{
	int i;

	--trace_level;
	if (trace_level < TRACE_WANTED)
	{
		for (i = 0; i < trace_level; ++i)
			printf("\t");

		printf("<= %s\n", s);
	}
}

void
nscd_trace_on(void)
{
	trace_level = trace_level_bk;
	trace_level_bk = 0;
}

void
nscd_trace_off(void)
{
	trace_level_bk = trace_level;
	trace_level = 1024;
}