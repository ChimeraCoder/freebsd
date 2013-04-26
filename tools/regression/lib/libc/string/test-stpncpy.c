
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

#include <sys/mman.h>
#include <sys/param.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *
makebuf(size_t len, int guard_at_end)
{
	char *buf;
	size_t alloc_size = roundup2(len, PAGE_SIZE) + PAGE_SIZE;

	buf = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_ANON, -1, 0);
	assert(buf);
	if (guard_at_end) {
		assert(munmap(buf + alloc_size - PAGE_SIZE, PAGE_SIZE) == 0);
		return (buf + alloc_size - PAGE_SIZE - len);
	} else {
		assert(munmap(buf, PAGE_SIZE) == 0);
		return (buf + PAGE_SIZE);
	}
}

static void
test_stpncpy(const char *s)
{
	char *src, *dst;
	size_t size, len, bufsize, x;
	int i, j;

	size = strlen(s) + 1;
	for (i = 0; i <= 1; i++) {
		for (j = 0; j <= 1; j++) {
			for (bufsize = 0; bufsize <= size + 10; bufsize++) {
				src = makebuf(size, i);
				memcpy(src, s, size);
				dst = makebuf(bufsize, j);
				memset(dst, 'X', bufsize);
				len = (bufsize < size) ? bufsize : size - 1;
				assert(stpncpy(dst, src, bufsize) == dst+len);
				assert(memcmp(src, dst, len) == 0);
				for (x = len; x < bufsize; x++)
					assert(dst[x] == '\0');
			}
		}
	}
}

int
main(int argc, char *argv[])
{

	printf("1..3\n");

	test_stpncpy("");
	printf("ok 1 - stpncpy\n");
	test_stpncpy("foo");
	printf("ok 2 - stpncpy\n");
	test_stpncpy("glorp");
	printf("ok 3 - stpncpy\n");

	exit(0);
}