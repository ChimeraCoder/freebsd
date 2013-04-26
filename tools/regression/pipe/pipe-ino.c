
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

/*
 * $FreeBSD$
 * Test conformance to stat(2) SUSv4 description:
 *  "For all other file types defined in this volume of POSIX.1-2008, the
 *  structure members st_mode, st_ino, st_dev, st_uid, st_gid, st_atim,
 *  st_ctim, and st_mtim shall have meaningful values ...".
 * Check that st_dev and st_ino are meaningful.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
	int pipefd[2];
	struct stat st1, st2;

	if (pipe(pipefd) == -1)
		err(1, "FAIL: pipe");

	if (fstat(pipefd[0], &st1) == -1)
		err(1, "FAIL: fstat st1");
	if (fstat(pipefd[1], &st2) == -1)
		err(1, "FAIL: fstat st2");
	if (st1.st_dev != st2.st_dev || st1.st_dev == 0 || st2.st_dev == 0) {
		errx(1, "FAIL: wrong dev number %d %d",
		    st1.st_dev, st2.st_dev);
	}
	if (st1.st_ino == st2.st_ino)
		errx(1, "FAIL: inode numbers are equal: %d", st1.st_ino);
	close(pipefd[0]);
	close(pipefd[1]);
	printf("PASS\n");

	return (0);
}