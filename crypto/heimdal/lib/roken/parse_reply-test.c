
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

#include <config.h>

#include <sys/types.h>
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <fcntl.h>

#include "roken.h"
#include "resolve.h"

struct dns_reply*
parse_reply(const unsigned char *, size_t);

enum { MAX_BUF = 36};

static struct testcase {
    unsigned char buf[MAX_BUF];
    size_t buf_len;
} tests[] = {
    {{0x12, 0x67, 0x84, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
     0x03, 'f', 'o', 'o', 0x00,
     0x00, 0x10, 0x00, 0x01,
     0x03, 'f', 'o', 'o', 0x00,
     0x00, 0x10, 0x00, 0x01,
      0x00, 0x00, 0x12, 0x67, 0xff, 0xff}, 36}
};

#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

static sig_atomic_t val = 0;

static RETSIGTYPE
segv_handler(int sig)
{
    val = 1;
}

int
main(int argc, char **argv)
{
#ifndef HAVE_MMAP
    return 77;			/* signal to automake that this test
                                   cannot be run */
#else /* HAVE_MMAP */
    int ret;
    int i;
    struct sigaction sa;

    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = segv_handler;
    sigaction (SIGSEGV, &sa, NULL);

    for (i = 0; val == 0 && i < sizeof(tests)/sizeof(tests[0]); ++i) {
	const struct testcase *t = &tests[i];
	unsigned char *p1, *p2;
	int flags;
	int fd;
	size_t pagesize = getpagesize();
	unsigned char *buf;

#ifdef MAP_ANON
	flags = MAP_ANON;
	fd = -1;
#else
	flags = 0;
	fd = open ("/dev/zero", O_RDONLY);
	if(fd < 0)
	    err (1, "open /dev/zero");
#endif
	flags |= MAP_PRIVATE;

	p1 = (unsigned char *)mmap(0, 2 * pagesize, PROT_READ | PROT_WRITE,
		  flags, fd, 0);
	if (p1 == (unsigned char *)MAP_FAILED)
	    err (1, "mmap");
	p2 = p1 + pagesize;
	ret = mprotect ((void *)p2, pagesize, 0);
	if (ret < 0)
	    err (1, "mprotect");
	buf = p2 - t->buf_len;
	memcpy (buf, t->buf, t->buf_len);
	parse_reply (buf, t->buf_len);
	ret = munmap ((void *)p1, 2 * pagesize);
	if (ret < 0)
	    err (1, "munmap");
    }
    return val;
#endif /* HAVE_MMAP */
}