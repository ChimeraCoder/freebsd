
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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <aio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/*
 * This is a bit of a quick hack to do parallel IO testing through POSIX AIO.
 * Its specifically designed to work under FreeBSD and its derivatives;
 * note how I cheat by using aio_waitcomplete().
 *
 * TODO:
 *
 * + Add write support; so we can make sure we're not hitting throughput issues
 *   with read/modify/write of entire tracks of the disk
 * + Add in per-op stats - time and offset - so one could start mapping out
 *   the speed hotspots of the disk
 * + Add in different distributions - random, normal, left/right skewed normal,
 *   zipf, etc - and perhaps add the ability to run concurrent distributions
 *   (so a normal and a zipf; and also a random read; zipf write, etc.)
 *
 * Adrian Chadd <adrian@creative.net.au>
 */

typedef enum {
	IOT_NONE = 0x00,
	IOT_READ = 0x01,
	IOT_WRITE = 0x02
} iot_t;

static size_t
disk_getsize(int fd)
{
	off_t mediasize;	

	if (ioctl(fd, DIOCGMEDIASIZE, &mediasize) < 0) {
		perror("ioctl(DIOCGMEDIASIZE)");
		exit(1);
	}
	return mediasize;
}

iot_t
choose_aio(iot_t iomask)
{
	/* choose a random read or write event, limited by the mask */
	if (iomask == IOT_READ)
		return IOT_READ;
	else if (iomask == IOT_WRITE)
		return IOT_WRITE;
	return (random() & 0x01 ? IOT_READ : IOT_WRITE);
}

void
set_aio(struct aiocb *a, iot_t iot, int fd, off_t offset, int size, char *buf)
{
	int r;
	bzero(a, sizeof(*a));
	a->aio_fildes = fd;
	a->aio_nbytes = size;
	a->aio_offset = offset;
	a->aio_buf = buf;
	if (iot == IOT_READ)
		r = aio_read(a);
	else
		r = aio_write(a);
	if (r != 0) {
		perror("set_aio");
		exit(1);
	}
}

int
main(int argc, char *argv[])
{
	int fd;
	struct stat sb;
	struct aiocb *aio;
	char **abuf;
	const char *fn;
	int aio_len;
	int io_size, nrun;
	off_t file_size, offset;
	struct aiocb *a;
	int i, n;
        struct timeval st, et, rt;
        float f_rt;
	iot_t iowhat;


	if (argc < 6) {
		printf("Usage: %s <file> <io size> <number of runs> <concurrency> <ro|wo|rw>\n", argv[0]);
		exit(1);
	}

	fn = argv[1];
	io_size = atoi(argv[2]);
	nrun = atoi(argv[3]);
	aio_len = atoi(argv[4]);
	if (strcmp(argv[5], "ro") == 0) {
		iowhat = IOT_READ;
	} else if (strcmp(argv[5], "rw") == 0) {
		iowhat = IOT_READ | IOT_WRITE;
	} else if (strcmp(argv[5], "wo") == 0) {
		iowhat = IOT_WRITE;
	} else {
		fprintf(stderr, "needs to be ro, rw, wo!\n");
		exit(1);
	}

	/*
	 * Random returns values between 0 and (2^32)-1; only good for 4 gig.
	 * Lets instead treat random() as returning a block offset w/ block size
	 * being "io_size", so we can handle > 4 gig files.
	 */
	if (iowhat == IOT_READ)
		fd = open(fn, O_RDONLY | O_DIRECT);
	else if (iowhat == IOT_WRITE)
		fd = open(fn, O_WRONLY | O_DIRECT);
	else
		fd = open(fn, O_RDWR | O_DIRECT);

	if (fd < 0) {
		perror("open");
		exit(1);
	}
	if (fstat(fd, &sb) < 0) {
		perror("fstat");
		exit(1);
	}
	if (S_ISREG(sb.st_mode)) {
		file_size = sb.st_size;
	} else if (S_ISBLK(sb.st_mode) || S_ISCHR(sb.st_mode)) {
		file_size = disk_getsize(fd);
	} else {
		perror("unknown file type\n");
		exit(1);
	}
	printf("File: %s; File size %qd bytes\n", fn, file_size);

	aio = calloc(aio_len, sizeof(struct aiocb));
	abuf = calloc(aio_len, sizeof(char *));
	for (i = 0; i < aio_len; i++) {
		abuf[i] = calloc(1, io_size * sizeof(char));
	}

	/* Fill with the initial contents */
        gettimeofday(&st, NULL);
	for (i = 0; i < aio_len; i++) {
                offset = random() % (file_size / io_size);
                offset *= io_size;
		set_aio(aio + i, choose_aio(iowhat), fd, offset, io_size, abuf[i]);
	}

	for (i = 0; i < nrun; i++) {
		aio_waitcomplete(&a, NULL);
		n = a - aio;
		assert(n < aio_len);
		assert(n >= 0);
                offset = random() % (file_size / io_size);
                offset *= io_size;
		set_aio(aio + n, choose_aio(iowhat), fd, offset, io_size, abuf[n]);
	}

        gettimeofday(&et, NULL);
        timersub(&et, &st, &rt);
        f_rt = ((float) (rt.tv_usec)) / 1000000.0;
        f_rt += (float) (rt.tv_sec);
        printf("Runtime: %.2f seconds, ", f_rt);
        printf("Op rate: %.2f ops/sec, ", ((float) (nrun))  / f_rt);
        printf("Avg transfer rate: %.2f bytes/sec\n", ((float) (nrun)) * ((float)io_size) / f_rt);



	exit(0);
}