
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

/*-
 * util.c --
 *	General utilitarian routines for make(1).
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "globals.h"
#include "job.h"
#include "targ.h"
#include "util.h"

static void enomem(void) __dead2;

/*-
 * Debug --
 *	Print a debugging message given its format.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The message is printed.
 */
/* VARARGS */
void
Debug(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fflush(stderr);
}

/*-
 * Print a debugging message given its format and append the current
 * errno description. Terminate with a newline.
 */
/* VARARGS */
void
DebugM(const char *fmt, ...)
{
	va_list	ap;
	int e = errno;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ": %s\n", strerror(e));
	va_end(ap);
	fflush(stderr);
}

/*-
 * Error --
 *	Print an error message given its format.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The message is printed.
 */
/* VARARGS */
void
Error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	fflush(stderr);
}

/*-
 * Fatal --
 *	Produce a Fatal error message. If jobs are running, waits for them
 *	to finish.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The program exits
 */
/* VARARGS */
void
Fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (jobsRunning)
		Job_Wait();

	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	fflush(stderr);

	if (DEBUG(GRAPH2))
		Targ_PrintGraph(2);
	exit(2);		/* Not 1 so -q can distinguish error */
}

/*
 * Punt --
 *	Major exception once jobs are being created. Kills all jobs, prints
 *	a message and exits.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	All children are killed indiscriminately and the program Lib_Exits
 */
/* VARARGS */
void
Punt(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "make: ");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	fflush(stderr);

	DieHorribly();
}

/*-
 * DieHorribly --
 *	Exit without giving a message.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	A big one...
 */
void
DieHorribly(void)
{
	if (jobsRunning)
		Job_AbortAll();
	if (DEBUG(GRAPH2))
		Targ_PrintGraph(2);
	exit(2);		/* Not 1, so -q can distinguish error */
}

/*
 * Finish --
 *	Called when aborting due to errors in child shell to signal
 *	abnormal exit, with the number of errors encountered in Make_Make.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The program exits
 */
void
Finish(int errors)
{

	Fatal("%d error%s", errors, errors == 1 ? "" : "s");
}

/*
 * emalloc --
 *	malloc, but die on error.
 */
void *
emalloc(size_t len)
{
	void *p;

	if ((p = malloc(len)) == NULL)
		enomem();
	return (p);
}

/*
 * estrdup --
 *	strdup, but die on error.
 */
char *
estrdup(const char *str)
{
	char *p;

	if ((p = strdup(str)) == NULL)
		enomem();
	return (p);
}

/*
 * erealloc --
 *	realloc, but die on error.
 */
void *
erealloc(void *ptr, size_t size)
{

	if ((ptr = realloc(ptr, size)) == NULL)
		enomem();
	return (ptr);
}

/*
 * enomem --
 *	die when out of memory.
 */
static void
enomem(void)
{
	err(2, NULL);
}

/*
 * enunlink --
 *	Remove a file carefully, avoiding directories.
 */
int
eunlink(const char *file)
{
	struct stat st;

	if (lstat(file, &st) == -1)
		return (-1);

	if (S_ISDIR(st.st_mode)) {
		errno = EISDIR;
		return (-1);
	}
	return (unlink(file));
}

/*
 * Convert a flag word to a printable thing and print it
 */
void
print_flags(FILE *fp, const struct flag2str *tab, u_int flags, int par)
{
	int first = 1;

	if (par)
		fprintf(fp, "(");
	while (tab->str != NULL) {
		if (flags & tab->flag) {
			if (!first)
				fprintf(fp, par ? "|" : " ");
			first = 0;
			fprintf(fp, "%s", tab->str);
		}
		tab++;
	}
	if (par)
		fprintf(fp, ")");
}