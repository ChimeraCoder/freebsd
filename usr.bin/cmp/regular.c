
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)regular.c	8.3 (Berkeley) 4/2/94";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <err.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "extern.h"

static u_char *remmap(u_char *, int, off_t);
static void segv_handler(int);
#define MMAP_CHUNK (8*1024*1024)

#define ROUNDPAGE(i) ((i) & ~pagemask)

void
c_regular(int fd1, const char *file1, off_t skip1, off_t len1,
    int fd2, const char *file2, off_t skip2, off_t len2)
{
	u_char ch, *p1, *p2, *m1, *m2, *e1, *e2;
	off_t byte, length, line;
	int dfound;
	off_t pagemask, off1, off2;
	size_t pagesize;
	struct sigaction act, oact;

	if (skip1 > len1)
		eofmsg(file1);
	len1 -= skip1;
	if (skip2 > len2)
		eofmsg(file2);
	len2 -= skip2;

	if (sflag && len1 != len2)
		exit(DIFF_EXIT);

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	act.sa_handler = segv_handler;
	if (sigaction(SIGSEGV, &act, &oact))
		err(ERR_EXIT, "sigaction()");

	pagesize = getpagesize();
	pagemask = (off_t)pagesize - 1;
	off1 = ROUNDPAGE(skip1);
	off2 = ROUNDPAGE(skip2);

	length = MIN(len1, len2);

	if ((m1 = remmap(NULL, fd1, off1)) == NULL) {
		c_special(fd1, file1, skip1, fd2, file2, skip2);
		return;
	}

	if ((m2 = remmap(NULL, fd2, off2)) == NULL) {
		munmap(m1, MMAP_CHUNK);
		c_special(fd1, file1, skip1, fd2, file2, skip2);
		return;
	}

	dfound = 0;
	e1 = m1 + MMAP_CHUNK;
	e2 = m2 + MMAP_CHUNK;
	p1 = m1 + (skip1 - off1);
	p2 = m2 + (skip2 - off2);

	for (byte = line = 1; length--; ++byte) {
		if ((ch = *p1) != *p2) {
			if (xflag) {
				dfound = 1;
				(void)printf("%08llx %02x %02x\n",
				    (long long)byte - 1, ch, *p2);
			} else if (lflag) {
				dfound = 1;
				(void)printf("%6lld %3o %3o\n",
				    (long long)byte, ch, *p2);
			} else
				diffmsg(file1, file2, byte, line);
				/* NOTREACHED */
		}
		if (ch == '\n')
			++line;
		if (++p1 == e1) {
			off1 += MMAP_CHUNK;
			if ((p1 = m1 = remmap(m1, fd1, off1)) == NULL) {
				munmap(m2, MMAP_CHUNK);
				err(ERR_EXIT, "remmap %s", file1);
			}
			e1 = m1 + MMAP_CHUNK;
		}
		if (++p2 == e2) {
			off2 += MMAP_CHUNK;
			if ((p2 = m2 = remmap(m2, fd2, off2)) == NULL) {
				munmap(m1, MMAP_CHUNK);
				err(ERR_EXIT, "remmap %s", file2);
			}
			e2 = m2 + MMAP_CHUNK;
		}
	}
	munmap(m1, MMAP_CHUNK);
	munmap(m2, MMAP_CHUNK);

	if (sigaction(SIGSEGV, &oact, NULL))
		err(ERR_EXIT, "sigaction()");

	if (len1 != len2)
		eofmsg (len1 > len2 ? file2 : file1);
	if (dfound)
		exit(DIFF_EXIT);
}

static u_char *
remmap(u_char *mem, int fd, off_t offset)
{
	if (mem != NULL)
		munmap(mem, MMAP_CHUNK);
	mem = mmap(NULL, MMAP_CHUNK, PROT_READ, MAP_SHARED, fd, offset);
	if (mem == MAP_FAILED)
		return (NULL);
	madvise(mem, MMAP_CHUNK, MADV_SEQUENTIAL);
	return (mem);
}

static void
segv_handler(int sig __unused) {
	static const char msg[] = "cmp: Input/output error (caught SIGSEGV)\n";

	write(STDERR_FILENO, msg, sizeof(msg));
	_exit(EXIT_FAILURE);
}