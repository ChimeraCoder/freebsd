
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
recvmsg(int s, struct msghdr *msg, int flags)
{
    ssize_t ret, nb;
    size_t tot = 0;
    int i;
    char *buf, *p;
    struct iovec *iov = msg->msg_iov;

    for(i = 0; i < msg->msg_iovlen; ++i)
	tot += iov[i].iov_len;
    buf = malloc(tot);
    if (tot != 0 && buf == NULL) {
	errno = ENOMEM;
	return -1;
    }
    nb = ret = recvfrom (s, buf, tot, flags, msg->msg_name, &msg->msg_namelen);
    p = buf;
    while (nb > 0) {
	ssize_t cnt = min(nb, iov->iov_len);

	memcpy (iov->iov_base, p, cnt);
	p += cnt;
	nb -= cnt;
	++iov;
    }
    free(buf);
    return ret;
}