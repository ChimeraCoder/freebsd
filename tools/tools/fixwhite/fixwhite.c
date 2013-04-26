
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

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static char *queue = NULL;
static size_t queuelen = 0, queuesize = 0;
static off_t column = 0;

static void
savebyte(char c)
{

	if (queuelen >= queuesize) {
		queuesize += 128;
		queue = realloc(queue, queuesize);
		if (queue == NULL) {
			perror("malloc");
			exit(1);
		}
	}
	queue[queuelen++] = c;

	switch (c) {
	case '\n':
		column = 0;
		break;
	case ' ':
		column++;
		break;
	case '\t':
		column = (column / 8 + 1) * 8;
		break;
	}
}

static bool
peekbyte(size_t back, char c)
{

	return (queuelen >= back && queue[queuelen - back] == c);
}

static void
savewhite(char c, bool leading)
{
	off_t ncolumn;

	switch (c) {
	case '\n':
		if (leading) {
			/* Remove empty lines before input. */
			queuelen = 0;
			column = 0;
		} else {
			/* Remove trailing whitespace. */
			while (peekbyte(1, ' ') || peekbyte(1, '\t'))
				queuelen--;
			/* Remove redundant empty lines. */
			if (peekbyte(2, '\n') && peekbyte(1, '\n'))
				return;
			savebyte('\n');
		}
		break;
	case ' ':
		savebyte(' ');
		break;
	case '\t':
		/* Convert preceeding spaces to tabs. */
		ncolumn = (column / 8 + 1) * 8;
		while (peekbyte(1, ' ')) {
			queuelen--;
			column--;
		}
		while (column < ncolumn)
			savebyte('\t');
		break;
	}
}

static void
printwhite(void)
{
	off_t i;

	/* Merge spaces at the start of a sentence to tabs if possible. */
	if ((column % 8) == 0) {
		for (i = 0; i < column; i++)
			if (!peekbyte(i + 1, ' '))
				break;
		if (i == column) {
			queuelen -= column;
			for (i = 0; i < column; i += 8)
				queue[queuelen++] = '\t';
		}
	}

	if (fwrite(queue, 1, queuelen, stdout) != queuelen) {
		perror("write");
		exit(1);
	}
	queuelen = 0;
}

static char
readchar(void)
{
	int c;

	c = getchar();
	if (c == EOF && ferror(stdin)) {
		perror("read");
		exit(1);
	}
	return (c);
}

static void
writechar(char c)
{

	if (putchar(c) == EOF) {
		perror("write");
		exit(1);
	}
	/* XXX: Multi-byte characters. */
	column++;
}

int
main(void)
{
	int c;
	bool leading = true;

	while ((c = readchar()) != EOF) {
		if (isspace(c))
			/* Save whitespace. */
			savewhite(c, leading);
		else {
			/* Reprint whitespace and print regular character. */
			printwhite();
			writechar(c);
			leading = false;
		}
	}
	/* Terminate non-empty files with a newline. */
	if (!leading)
		writechar('\n');
	return (0);
}