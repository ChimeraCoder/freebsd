
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

#include <err.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mdef.h"
#include "stdd.h"
#include "extern.h"

FILE *traceout;

#define TRACE_ARGS	1
#define TRACE_EXPANSION 2
#define TRACE_QUOTE	4
#define TRACE_FILENAME	8
#define TRACE_LINENO	16
#define TRACE_CONT	32
#define TRACE_ID	64
#define TRACE_NEWFILE	128	/* not implemented yet */
#define TRACE_INPUT	256	/* not implemented yet */

static unsigned int letter_to_flag(int);
static void print_header(struct input_file *);
static int frame_level(void);


unsigned int trace_flags = TRACE_QUOTE | TRACE_EXPANSION;

void
trace_file(const char *name)
{

	if (traceout && traceout != stderr)
		fclose(traceout);
	traceout = fopen(name, "w");
	if (!traceout)
		err(1, "can't open %s", name);
}

static unsigned int
letter_to_flag(int c)
{
	switch(c) {
	case 'a':
		return TRACE_ARGS;
	case 'e':
		return TRACE_EXPANSION;
	case 'q':
		return TRACE_QUOTE;
	case 'c':
		return TRACE_CONT;
	case 'x':
		return TRACE_ID;
	case 'f':
		return TRACE_FILENAME;
	case 'l':
		return TRACE_LINENO;
	case 'p':
		return TRACE_NEWFILE;
	case 'i':
		return TRACE_INPUT;
	case 't':
		return TRACE_ALL;
	case 'V':
		return ~0;
	default:
		return 0;
	}
}

void
set_trace_flags(const char *s)
{
	char mode = 0;
	unsigned int f = 0;

	if (*s == '+' || *s == '-')
		mode = *s++;
	while (*s)
		f |= letter_to_flag(*s++);
	switch(mode) {
	case 0:
		trace_flags = f;
		break;
	case '+':
		trace_flags |= f;
		break;
	case '-':
		trace_flags &= ~f;
		break;
	}
}

static int
frame_level(void)
{
	int level;
	int framep;

	for (framep = fp, level = 0; framep != 0;
		level++,framep = mstack[framep-3].sfra)
		;
	return level;
}

static void
print_header(struct input_file *inp)
{
	fprintf(traceout, "m4trace:");
	if (trace_flags & TRACE_FILENAME)
		fprintf(traceout, "%s:", inp->name);
	if (trace_flags & TRACE_LINENO)
		fprintf(traceout, "%lu:", inp->lineno);
	fprintf(traceout, " -%d- ", frame_level());
	if (trace_flags & TRACE_ID)
		fprintf(traceout, "id %lu: ", expansion_id);
}

size_t
trace(const char *argv[], int argc, struct input_file *inp)
{
	if (!traceout)
		traceout = stderr;
	print_header(inp);
	if (trace_flags & TRACE_CONT) {
		fprintf(traceout, "%s ...\n", argv[1]);
		print_header(inp);
	}
	fprintf(traceout, "%s", argv[1]);
	if ((trace_flags & TRACE_ARGS) && argc > 2) {
		char delim[3];
		int i;

		delim[0] = LPAREN;
		delim[1] = EOS;
		for (i = 2; i < argc; i++) {
			fprintf(traceout, "%s%s%s%s", delim,
			    (trace_flags & TRACE_QUOTE) ? lquote : "",
			    argv[i],
			    (trace_flags & TRACE_QUOTE) ? rquote : "");
			delim[0] = COMMA;
			delim[1] = ' ';
			delim[2] = EOS;
		}
		fprintf(traceout, "%c", RPAREN);
	}
	if (trace_flags & TRACE_CONT) {
		fprintf(traceout, " -> ???\n");
		print_header(inp);
		fprintf(traceout, argc > 2 ? "%s(...)" : "%s", argv[1]);
	}
	if (trace_flags & TRACE_EXPANSION)
		return buffer_mark();
	else {
		fprintf(traceout, "\n");
		return SIZE_MAX;
	}
}

void
finish_trace(size_t mark)
{
	fprintf(traceout, " -> ");
	if (trace_flags & TRACE_QUOTE)
		fprintf(traceout, "%s", lquote);
	dump_buffer(traceout, mark);
	if (trace_flags & TRACE_QUOTE)
		fprintf(traceout, "%s", rquote);
	fprintf(traceout, "\n");
}