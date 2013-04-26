
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
 * Simple audit pipe regression test to confirm that the ioctls for queue
 * limit information basically work.  No attempt is made to validate the
 * queue length returned, however.
 */

#include <sys/types.h>
#include <sys/ioctl.h>

#include <security/audit/audit_ioctl.h>

#include <err.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
	u_int len, minlen, maxlen;
	u_int64_t astat;
	int fd;

	fd = open("/dev/auditpipe", O_RDONLY);
	if (fd < 0)
		err(-1, "/dev/auditpipe");

	/*
	 * First, test that we can read the queue length, queue limit, and
	 * bounds on queue length limits.
	 */
	len = (u_int)(-1);
	if (ioctl(fd, AUDITPIPE_GET_QLEN, &len) < 0)
		err(-1, "AUDITPIPE_GET_QLEN");
	if (len == (u_int)(-1))
		errx(-1, "AUDITPIPE_GET_QLEN: unchanged");

	minlen = (u_int)(-1);
	if (ioctl(fd, AUDITPIPE_GET_QLIMIT_MIN, &minlen) < 0)
		err(-1, "AUDITPIPE_GET_QLIMIT_MIN");
	if (minlen == (u_int)(-1))
		errx(-1, "AUDITPIPE_GET_QLIMIT_MIN: unchanged");

	maxlen = (u_int)(-1);
	if (ioctl(fd, AUDITPIPE_GET_QLIMIT_MAX, &maxlen) < 0)
		err(-1, "AUDITPIPE_GET_QLIMIT_MAX");
	if (maxlen == (u_int)(-1))
		errx(-1, "AUDITPIPE_GET_QLIMIT_MAX: unchanged");

	len = (u_int)(-1);
	if (ioctl(fd, AUDITPIPE_GET_QLIMIT, &len) < 0)
		err(-1, "AUDITPIPE_GET_QLIMIT");
	if (len == (u_int)(-1))
		errx(-1, "AUDITPIPE_GET_QLIMIT: unchanged");

	if (!(len >= minlen))
		errx(-1, "queue length < minlen");

	if (!(len <= maxlen))
		errx(-1, "queue length > maxlen");

	/*
	 * Try setting the queue length to first minimum, then maximum
	 * lengths.  Query after each to make sure it changed.
	 */
	len = minlen;
	if (ioctl(fd, AUDITPIPE_SET_QLIMIT, &len) < 0)
		err(-1, "AUDITPIPE_SET_QLIMIT(min)");

	if (ioctl(fd, AUDITPIPE_GET_QLIMIT, &len) < 0)
		err(-1, "AUDITPIPE_GET_QLIMIT");

	if (len != minlen)
		errx(-1, "set to minlen didn't work");

	len = maxlen;
	if (ioctl(fd, AUDITPIPE_SET_QLIMIT, &len) < 0)
		err(-1, "AUDITPIPE_SET_QLIMIT(max)");

	if (ioctl(fd, AUDITPIPE_GET_QLIMIT, &len) < 0)
		err(-1, "AUDITPIPE_GETQLIMIT");

	if (len != maxlen)
		errx(-1, "set to maxlen didn't work");

	/*
	 * Check that we can query the defined stats.  No attempt to
	 * validate.
	 */
	astat = (u_int64_t)(int64_t)(-1);
	if (ioctl(fd, AUDITPIPE_GET_INSERTS, &astat) < 0)
		err(-1, "AUDITPIPE_GET_INSERTS");
	if (astat == (u_int64_t)(int64_t)(-1))
		errx(-1, "AUDITPIPE_GET_INSERTS: unchanged");

	astat = (u_int64_t)(int64_t)(-1);
	if (ioctl(fd, AUDITPIPE_GET_READS, &astat) < 0)
		err(-1, "AUDITPIPE_GET_READS");
	if (astat == (u_int64_t)(int64_t)(-1))
		errx(-1, "AUDITPIPE_GET_READS: unchanged");

	astat = (u_int64_t)(int64_t)(-1);
	if (ioctl(fd, AUDITPIPE_GET_DROPS, &astat) < 0)
		err(-1, "AUDITPIPE_GET_DROPS");
	if (astat == (u_int64_t)(int64_t)(-1))
		errx(-1, "AUDITPIPE_GET_DROPS: unchanged");

	astat = (u_int64_t)(int64_t)(-1);
	if (ioctl(fd, AUDITPIPE_GET_TRUNCATES, &astat) < 0)
		err(-1, "AUDITPIPE_GET_TRUNCATES");
	if (astat == (u_int64_t)(int64_t)(-1))
		errx(-1, "AUDITPIPE_GET_TRUNCATES: unchanged");

	return (0);
}