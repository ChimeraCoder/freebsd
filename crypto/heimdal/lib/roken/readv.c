
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

#include "roken.h"

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
readv(int d, const struct iovec *iov, int iovcnt)
{
    ssize_t ret, nb;
    size_t tot = 0;
    int i;
    char *buf, *p;

    for(i = 0; i < iovcnt; ++i)
	tot += iov[i].iov_len;
    buf = malloc(tot);
    if (tot != 0 && buf == NULL) {
	errno = ENOMEM;
	return -1;
    }
    nb = ret = read (d, buf, tot);
    p = buf;
    while (nb > 0) {
	ssize_t cnt = min(nb, iov->iov_len);

	memcpy (iov->iov_base, p, cnt);
	p += cnt;
	nb -= cnt;
    }
    free(buf);
    return ret;
}