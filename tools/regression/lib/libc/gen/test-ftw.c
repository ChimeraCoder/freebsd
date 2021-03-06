
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
 * Limited test program for nftw() as specified by IEEE Std. 1003.1-2008.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/wait.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spawn.h>

extern char **environ;

static char dir[] = "/tmp/testftw.XXXXXXXXXX";
static int failures;
static int ftwflags;

static void
cleanup(int ustatus __unused)
{
	int error, status;
	pid_t pid, waitres;
	const char *myargs[5];

	err_set_exit(NULL);
	myargs[0] = "rm";
	myargs[1] = "-rf";
	myargs[2] = "--";
	myargs[3] = dir;
	myargs[4] = NULL;
	error = posix_spawnp(&pid, myargs[0], NULL, NULL,
	    __DECONST(char **, myargs), environ);
	if (error != 0)
		warn("posix_spawnp rm");
	else {
		waitres = waitpid(pid, &status, 0);
		if (waitres != pid)
			warnx("waitpid rm failed");
		else if (status != 0)
			warnx("rm failed");
	}
}

static int
cb(const char *path, const struct stat *st, int type, struct FTW *f)
{

	switch (type) {
	case FTW_D:
		if ((ftwflags & FTW_DEPTH) == 0)
			return (0);
		break;
	case FTW_DP:
		if ((ftwflags & FTW_DEPTH) != 0)
			return (0);
		break;
	case FTW_SL:
		if ((ftwflags & FTW_PHYS) != 0)
			return (0);
		break;
	}
	warnx("unexpected path=%s type=%d f.level=%d\n",
	    path, type, f->level);
	failures++;
	return (0);
}

int
main(int argc, char *argv[])
{
	int fd;

	if (!mkdtemp(dir))
		err(2, "mkdtemp");

	err_set_exit(cleanup);

	fd = open(dir, O_DIRECTORY | O_RDONLY);
	if (fd == -1)
		err(2, "open %s", dir);

	if (mkdirat(fd, "d1", 0777) == -1)
		err(2, "mkdirat d1");

	if (symlinkat(dir, fd, "d1/looper") == -1)
		err(2, "symlinkat looper");

	ftwflags = FTW_PHYS;
	if (nftw(dir, cb, 10, ftwflags) == -1)
		err(2, "nftw FTW_PHYS");
	ftwflags = FTW_PHYS | FTW_DEPTH;
	if (nftw(dir, cb, 10, ftwflags) == -1)
		err(2, "nftw FTW_PHYS | FTW_DEPTH");
	ftwflags = 0;
	if (nftw(dir, cb, 10, ftwflags) == -1)
		err(2, "nftw 0");
	ftwflags = FTW_DEPTH;
	if (nftw(dir, cb, 10, ftwflags) == -1)
		err(2, "nftw FTW_DEPTH");

	close(fd);

	printf("PASS nftw()\n");

	cleanup(failures != 0);

	return (failures != 0);
}