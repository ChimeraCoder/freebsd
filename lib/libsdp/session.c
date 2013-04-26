
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

#include <bluetooth.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sdp-int.h>
#include <sdp.h>

void *
sdp_open(bdaddr_t const *l, bdaddr_t const *r)
{
	sdp_session_p		ss = NULL;
	struct sockaddr_l2cap	sa;
	socklen_t		size;

	if ((ss = calloc(1, sizeof(*ss))) == NULL)
		goto fail;

	if (l == NULL || r == NULL) {
		ss->error = EINVAL;
		goto fail;
	}

	ss->s = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BLUETOOTH_PROTO_L2CAP);
	if (ss->s < 0) {
		ss->error = errno;
		goto fail;
	}

	sa.l2cap_len = sizeof(sa);
	sa.l2cap_family = AF_BLUETOOTH;
	sa.l2cap_psm = 0;
	memcpy(&sa.l2cap_bdaddr, l, sizeof(sa.l2cap_bdaddr));
	if (bind(ss->s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		ss->error = errno;
		goto fail;
	}

	sa.l2cap_psm = htole16(NG_L2CAP_PSM_SDP);
	memcpy(&sa.l2cap_bdaddr, r, sizeof(sa.l2cap_bdaddr));
	if (connect(ss->s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		ss->error = errno;
		goto fail;
	}

	size = sizeof(ss->omtu);
	if (getsockopt(ss->s, SOL_L2CAP, SO_L2CAP_OMTU, &ss->omtu, &size) < 0) {
		ss->error = errno;
		goto fail;
	}
	if ((ss->req = malloc(ss->omtu)) == NULL) {
		ss->error = ENOMEM;
		goto fail;
	}
	ss->req_e = ss->req + ss->omtu;

	size = sizeof(ss->imtu);
	if (getsockopt(ss->s, SOL_L2CAP, SO_L2CAP_IMTU, &ss->imtu, &size) < 0) {
		ss->error = errno;
		goto fail;
	}
	if ((ss->rsp = malloc(ss->imtu)) == NULL) {
		ss->error = ENOMEM;
		goto fail;
	}
	ss->rsp_e = ss->rsp + ss->imtu;
	ss->error = 0;
fail:
	return ((void *) ss);
}

void *
sdp_open_local(char const *control)
{
	sdp_session_p		ss = NULL;
	struct sockaddr_un	sa;

	if ((ss = calloc(1, sizeof(*ss))) == NULL)
		goto fail;

	ss->s = socket(PF_UNIX, SOCK_STREAM, 0);
	if (ss->s < 0) {
		ss->error = errno;
		goto fail;
	}

	if (control == NULL)
		control = SDP_LOCAL_PATH;

	sa.sun_len = sizeof(sa);
	sa.sun_family = AF_UNIX;
	strlcpy(sa.sun_path, control, sizeof(sa.sun_path));

	if (connect(ss->s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		ss->error = errno;
		goto fail;
	}

	ss->flags |= SDP_SESSION_LOCAL;
	ss->imtu = ss->omtu = SDP_LOCAL_MTU;

	if ((ss->req = malloc(ss->omtu)) == NULL) {
		ss->error = ENOMEM;
		goto fail;
	}
	ss->req_e = ss->req + ss->omtu;

	if ((ss->rsp = malloc(ss->imtu)) == NULL) {
		ss->error = ENOMEM;
		goto fail;
	}
	ss->rsp_e = ss->rsp + ss->imtu;
	ss->error = 0;
fail:
	return ((void *) ss);
}

int32_t
sdp_close(void *xss)
{
	sdp_session_p	ss = (sdp_session_p) xss;

	if (ss != NULL) {
		if (ss->s >= 0)
			close(ss->s);

		if (ss->req != NULL)
			free(ss->req);
		if (ss->rsp != NULL)
			free(ss->rsp);

		memset(ss, 0, sizeof(*ss));
		free(ss);
	}

	return (0);
}

int32_t
sdp_error(void *xss)
{
	sdp_session_p	ss = (sdp_session_p) xss;

	return ((ss != NULL)? ss->error : EINVAL);
}