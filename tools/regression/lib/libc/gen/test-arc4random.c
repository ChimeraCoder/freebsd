
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
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * BUFSIZE is the number of bytes of rc4 output to compare.  The probability
 * that this test fails spuriously is 2**(-BUFSIZE * 8).
 */
#define	BUFSIZE		8

/*
 * Test whether arc4random_buf() returns the same sequence of bytes in both
 * parent and child processes.  (Hint: It shouldn't.)
 */
int main(int argc, char *argv[]) {
	struct shared_page {
		char parentbuf[BUFSIZE];
		char childbuf[BUFSIZE];
	} *page;
	pid_t pid;
	char c;

	printf("1..1\n");

	page = mmap(NULL, sizeof(struct shared_page), PROT_READ | PROT_WRITE,
		    MAP_ANON | MAP_SHARED, -1, 0);
	if (page == MAP_FAILED) {
		printf("fail 1 - mmap\n");
		exit(1);
	}

	arc4random_buf(&c, 1);

	pid = fork();
	if (pid < 0) {
		printf("fail 1 - fork\n");
		exit(1);
	}
	if (pid == 0) {
		/* child */
		arc4random_buf(page->childbuf, BUFSIZE);
		exit(0);
	} else {
		/* parent */
		int status;
		arc4random_buf(page->parentbuf, BUFSIZE);
		wait(&status);
	}
	if (memcmp(page->parentbuf, page->childbuf, BUFSIZE) == 0) {
		printf("fail 1 - sequences are the same\n");
		exit(1);
	}

	printf("ok 1 - sequences are different\n");
	exit(0);
}