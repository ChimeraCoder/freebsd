
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
 * Prerequisities:
 * - AIO support must be compiled into the kernel (see sys/<arch>/NOTES for
 *   more details).
 *
 * Note: it is a good idea to run this against a physical drive to 
 * exercise the physio fast path (ie. aio_kqueue /dev/<something safe>)
 */

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <aio.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATH_TEMPLATE   "/tmp/aio.XXXXXXXXXX"

#define MAX 128
#define MAX_RUNS 300
/* #define DEBUG */

int
main (int argc, char *argv[])
{
	int fd;
	struct aiocb *iocb[MAX], *kq_iocb;
	int i, result, run, error, j;
	char buffer[32768];
	int kq = kqueue();
	struct kevent ke, kq_returned;
	struct timespec ts;
	int cancel, pending, tmp_file = 0, failed = 0;
	char *file, pathname[sizeof(PATH_TEMPLATE)+1];

	if (kq < 0) {
		perror("No kqeueue\n");
		exit(1);
	}

	if (argc == 1) { 
		strcpy(pathname, PATH_TEMPLATE);
		fd = mkstemp(pathname);
		file = pathname;
		tmp_file = 1;
	} else {
		file = argv[1];
		fd = open(file, O_RDWR|O_CREAT, 0666);
	}
	if (fd == -1)
		err(1, "Can't open %s\n", file);

	for (run = 0; run < MAX_RUNS; run++){
#ifdef DEBUG
		printf("Run %d\n", run);
#endif
		for (i = 0; i < MAX; i++) {
			iocb[i] = (struct aiocb *)calloc(1,
			    sizeof(struct aiocb));
			if (iocb[i] == NULL)
				err(1, "calloc");
		}
		
		pending = 0;	
		for (i = 0; i < MAX; i++) {
			pending++;
			iocb[i]->aio_nbytes = sizeof(buffer);
			iocb[i]->aio_buf = buffer;
			iocb[i]->aio_fildes = fd;
			iocb[i]->aio_offset = iocb[i]->aio_nbytes * i * run;
			
			iocb[i]->aio_sigevent.sigev_notify_kqueue = kq;
			iocb[i]->aio_sigevent.sigev_value.sival_ptr = iocb[i];
			iocb[i]->aio_sigevent.sigev_notify = SIGEV_KEVENT;
			
			result = aio_write(iocb[i]);
			if (result != 0) {
				perror("aio_write");
				printf("Result %d iteration %d\n", result, i);
				exit(1);
			}
#ifdef DEBUG
			printf("WRITE %d is at %p\n", i, iocb[i]);
#endif
			result = rand();
			if (result < RAND_MAX/32) {
				if (result > RAND_MAX/64) {
					result = aio_cancel(fd, iocb[i]);
#ifdef DEBUG
					printf("Cancel %d %p result %d\n", i, iocb[i], result);
#endif
					if (result == AIO_CANCELED) {
						aio_return(iocb[i]);
						iocb[i] = NULL;
						pending--;
					}
				}
			}
		}
		cancel = MAX - pending;
		
		i = 0;
		while (pending) {

			for (;;) {

				bzero(&ke, sizeof(ke));
				bzero(&kq_returned, sizeof(ke));
				ts.tv_sec = 0;
				ts.tv_nsec = 1;
				result = kevent(kq, NULL, 0, 
						&kq_returned, 1, &ts);
				error = errno;
				if (result < 0)
					perror("kevent error: ");
				kq_iocb = kq_returned.udata;
#ifdef DEBUG
				printf("kevent %d %d errno %d return.ident %p "
				       "return.data %p return.udata %p %p\n", 
				       i, result, error, 
				       kq_returned.ident, kq_returned.data, 
				       kq_returned.udata, 
				       kq_iocb);
#endif
				
				if (kq_iocb)
					break;
#ifdef DEBUG
				printf("Try again left %d out of %d %d\n",
				    pending, MAX, cancel);
#endif
			}			
			
			for (j = 0; j < MAX && iocb[j] != kq_iocb;
			   j++) ;
#ifdef DEBUG
			printf("kq_iocb %p\n", kq_iocb);
			
			printf("Error Result for %d is %d pending %d\n",
			    j, result, pending);
#endif
			result = aio_return(kq_iocb);
#ifdef DEBUG
			printf("Return Result for %d is %d\n\n", j, result);
#endif
			if (result != sizeof(buffer)) {
				printf("FAIL: run %d, operation %d, result %d "
				    " (errno=%d) should be %d\n", run, pending,
				    result, errno, sizeof(buffer));
				failed++;
			} else
				printf("PASS: run %d, left %d\n", run,
				    pending - 1);

			free(kq_iocb);
			iocb[j] = NULL;
			pending--;
			i++;
		}	

		for (i = 0; i < MAX; i++)
			free(iocb[i]);

	}

	if (tmp_file)
		unlink(pathname);

	if (failed != 0)
		printf("FAIL: %d tests failed\n", failed);
	else
		printf("PASS: All tests passed\n");

	exit (failed == 0 ? 0 : 1);
}