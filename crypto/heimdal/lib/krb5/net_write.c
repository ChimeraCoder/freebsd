
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

#include "krb5_locl.h"

KRB5_LIB_FUNCTION krb5_ssize_t KRB5_LIB_CALL
krb5_net_write (krb5_context context,
		void *p_fd,
		const void *buf,
		size_t len)
{
    krb5_socket_t fd = *((krb5_socket_t *)p_fd);
    return net_write(fd, buf, len);
}

KRB5_LIB_FUNCTION krb5_ssize_t KRB5_LIB_CALL
krb5_net_write_block(krb5_context context,
		     void *p_fd,
		     const void *buf,
		     size_t len,
		     time_t timeout)
{
  krb5_socket_t fd = *((krb5_socket_t *)p_fd);
  int ret;
  struct timeval tv, *tvp;
  const char *cbuf = (const char *)buf;
  size_t rem = len;
  ssize_t count;
  fd_set wfds;

  do {
      FD_ZERO(&wfds);
      FD_SET(fd, &wfds);

      if (timeout != 0) {
	  tv.tv_sec = timeout;
	  tv.tv_usec = 0;
	  tvp = &tv;
      } else
	  tvp = NULL;

      ret = select(fd + 1, NULL, &wfds, NULL, tvp);
      if (rk_IS_SOCKET_ERROR(ret)) {
	  if (rk_SOCK_ERRNO == EINTR)
	      continue;
	  return -1;
      }

#ifdef HAVE_WINSOCK
      if (ret == 0) {
	  WSASetLastError( WSAETIMEDOUT );
	  return 0;
      }

      count = send (fd, cbuf, rem, 0);

      if (rk_IS_SOCKET_ERROR(count)) {
	  return -1;
      }

#else
      if (ret == 0) {
	  return 0;
      }

      if (!FD_ISSET(fd, &wfds)) {
	  errno = ETIMEDOUT;
	  return -1;
      }

      count = write (fd, cbuf, rem);

      if (count < 0) {
	  if (errno == EINTR)
	      continue;
	  else
	      return count;
      }

#endif

      cbuf += count;
      rem -= count;

  } while (rem > 0);

  return len;
}