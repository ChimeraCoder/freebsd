
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

#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "pathnames.h"
#include "rtadvd.h"
#include "if.h"
#include "control.h"
#include "control_client.h"

int
cm_handler_client(int fd, int state, char *buf_orig)
{
	char buf[CM_MSG_MAXLEN];
	struct ctrl_msg_hdr *cm;
	struct ctrl_msg_hdr *cm_orig;
	int error;
	char *msg;
	char *msg_orig;

	syslog(LOG_DEBUG, "<%s> enter", __func__);

	memset(buf, 0, sizeof(buf));
	cm = (struct ctrl_msg_hdr *)buf;
	cm_orig = (struct ctrl_msg_hdr *)buf_orig;
	msg = (char *)buf + sizeof(*cm);
	msg_orig = (char *)buf_orig + sizeof(*cm_orig);

	if (cm_orig->cm_len > CM_MSG_MAXLEN) {
		syslog(LOG_DEBUG, "<%s> msg too long", __func__);
		close(fd);
		return (-1);
	}
	cm->cm_type = cm_orig->cm_type;
	if (cm_orig->cm_len > sizeof(*cm_orig)) {
		memcpy(msg, msg_orig, cm_orig->cm_len - sizeof(*cm));
		cm->cm_len = cm_orig->cm_len;
	}
	while (state != CM_STATE_EOM) {
		syslog(LOG_DEBUG, "<%s> state = %d", __func__, state);

		switch (state) {
		case CM_STATE_INIT:
			state = CM_STATE_EOM;
			break;
		case CM_STATE_MSG_DISPATCH:
			cm->cm_version = CM_VERSION;
			error = cm_send(fd, buf);
			if (error)
				syslog(LOG_WARNING,
				    "<%s> cm_send()", __func__);
			state = CM_STATE_ACK_WAIT;
			break;
		case CM_STATE_ACK_WAIT:
			error = cm_recv(fd, buf);
			if (error) {
				syslog(LOG_ERR,
				    "<%s> cm_recv()", __func__);
				close(fd);
				return (-1);
			}
			switch (cm->cm_type) {
			case CM_TYPE_ACK:
				syslog(LOG_DEBUG,
				    "<%s> CM_TYPE_ACK", __func__);
				break;
			case CM_TYPE_ERR:
				syslog(LOG_DEBUG,
				    "<%s> CM_TYPE_ERR", __func__);
				close(fd);
				return (-1);
			default:
				syslog(LOG_DEBUG,
				    "<%s> unknown status", __func__);
				close(fd);
				return (-1);
			}
			memcpy(buf_orig, buf, cm->cm_len);
			state = CM_STATE_EOM;
			break;
		}
	}
	close(fd);
	return (0);
}