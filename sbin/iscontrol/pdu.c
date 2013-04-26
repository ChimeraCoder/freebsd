
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
 | $Id: pdu.c,v 2.2 2006/12/01 09:11:56 danny Exp danny $
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <camlib.h>

#include <dev/iscsi/initiator/iscsi.h>
#include "iscontrol.h"

static void	pukeText(char *it, pdu_t *pp);

int
xmitpdu(isess_t *sess, pdu_t *pp)
{
     if(ioctl(sess->fd, ISCSISEND, pp)) {
	  perror("xmitpdu");
	  return -1;
     }
     if(vflag)
	  pukeText("I-", pp);

     return 0;
}

int
recvpdu(isess_t *sess, pdu_t *pp)
{
     if(ioctl(sess->fd, ISCSIRECV, pp)) {
	  perror("recvpdu");
	  return -1;
     }
     // XXX: return error if truncated via
     // the FUDGE factor.
     if(vflag)
	  pukeText("T-", pp);

     return 0;
}

int
sendPDU(isess_t *sess, pdu_t *pp, handler_t *hdlr)
{
     if(xmitpdu(sess, pp))
	  return 0;
     if(hdlr) {
	  int res;

	  pp->ahs_size = 8 * 1024;
	  if((pp->ahs_addr = malloc(pp->ahs_size)) == NULL) {
	       fprintf(stderr, "out of mem!");
	       return -1;
	  }
	  pp->ds_size = 0;
	  if((res = recvpdu(sess, pp)) != 0) {
	       fprintf(stderr, "recvpdu failed\n");
	       return res;
	  }
	  res = hdlr(sess, pp);
	  freePDU(pp);
	  return res;
     }
     return 1;
}


#define FUDGE (512 * 8)
/*
 | We use the same memory for the response
 | so make enough room ...
 | XXX: must find a better way.
 */
int
addText(pdu_t *pp, char *fmt, ...)
{
     u_int	len;
     char	*str;
     va_list	ap;

     va_start(ap, fmt);
     len = vasprintf(&str, fmt, ap) + 1;
     if((pp->ds_len + len) > 0xffffff) {
	  printf("ds overflow\n");
	  free(str);
	  return 0;
     }

     if((pp->ds_len + len) > pp->ds_size) {
	  u_char	*np;

	  np = realloc(pp->ds_addr, pp->ds_size + len + FUDGE);
	  if(np == NULL) {
	       free(str);
	       //XXX: out of memory!
	       return -1;
	  }
	  pp->ds_addr = np;
	  pp->ds_size += len + FUDGE;
     }
     memcpy(pp->ds_addr + pp->ds_len, str, len);
     pp->ds_len += len;
     free(str);
     return len;
}

void
freePDU(pdu_t *pp)
{
     if(pp->ahs_size)
	  free(pp->ahs_addr);
     if(pp->ds_size)
	  free(pp->ds_addr);
     bzero(&pp->ipdu, sizeof(union ipdu_u));
     pp->ahs_addr = NULL;
     pp->ds_addr = NULL;
     pp->ahs_size = 0;
     pp->ds_size = pp->ds_len = 0;
}

static void
pukeText(char *it, pdu_t *pp)
{
     char	*ptr;
     int	cmd;
     size_t	len, n;

     len = pp->ds_len;
     ptr = (char *)pp->ds_addr;
     cmd = pp->ipdu.bhs.opcode;

     printf("%s: cmd=0x%x len=%d\n", it, cmd, (int)len);
     while(len > 0) {
	  printf("\t%s\n", ptr);
	  n = strlen(ptr) + 1;
	  len -= n;
	  ptr += n;
     }
}