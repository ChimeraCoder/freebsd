
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

#include <errno.h>

#include "hast.h"
#include "hast_proto.h"
#include "hooks.h"
#include "nv.h"
#include "pjdlog.h"
#include "proto.h"
#include "subr.h"

#include "event.h"

void
event_send(const struct hast_resource *res, int event)
{
	struct nv *nvin, *nvout;
	int error;

	PJDLOG_ASSERT(res != NULL);
	PJDLOG_ASSERT(event >= EVENT_MIN && event <= EVENT_MAX);

	nvin = nvout = NULL;

	/*
	 * Prepare and send event to parent process.
	 */
	nvout = nv_alloc();
	nv_add_uint8(nvout, (uint8_t)event, "event");
	error = nv_error(nvout);
	if (error != 0) {
		pjdlog_common(LOG_ERR, 0, error,
		    "Unable to prepare event header");
		goto done;
	}
	if (hast_proto_send(res, res->hr_event, nvout, NULL, 0) == -1) {
		pjdlog_errno(LOG_ERR, "Unable to send event header");
		goto done;
	}
	if (hast_proto_recv_hdr(res->hr_event, &nvin) == -1) {
		pjdlog_errno(LOG_ERR, "Unable to receive event header");
		goto done;
	}
	/*
	 * Do nothing with the answer. We only wait for it to be sure not
	 * to exit too quickly after sending an event and exiting immediately.
	 */
done:
	if (nvin != NULL)
		nv_free(nvin);
	if (nvout != NULL)
		nv_free(nvout);
}

int
event_recv(const struct hast_resource *res)
{
	struct nv *nvin, *nvout;
	const char *evstr;
	uint8_t event;
	int error;

	PJDLOG_ASSERT(res != NULL);

	nvin = nvout = NULL;

	if (hast_proto_recv_hdr(res->hr_event, &nvin) == -1) {
		/*
		 * First error log as debug. This is because worker process
		 * most likely exited.
		 */
		pjdlog_common(LOG_DEBUG, 1, errno,
		    "Unable to receive event header");
		goto fail;
	}

	event = nv_get_uint8(nvin, "event");
	if (event == EVENT_NONE) {
		pjdlog_error("Event header is missing 'event' field.");
		goto fail;
	}

	switch (event) {
	case EVENT_CONNECT:
		evstr = "connect";
		break;
	case EVENT_DISCONNECT:
		evstr = "disconnect";
		break;
	case EVENT_SYNCSTART:
		evstr = "syncstart";
		break;
	case EVENT_SYNCDONE:
		evstr = "syncdone";
		break;
	case EVENT_SYNCINTR:
		evstr = "syncintr";
		break;
	case EVENT_SPLITBRAIN:
		evstr = "split-brain";
		break;
	default:
		pjdlog_error("Event header contain invalid event number (%hhu).",
		    event);
		goto fail;
	}

	pjdlog_prefix_set("[%s] (%s) ", res->hr_name, role2str(res->hr_role));
	hook_exec(res->hr_exec, evstr, res->hr_name, NULL);
	pjdlog_prefix_set("%s", "");

	nvout = nv_alloc();
	nv_add_int16(nvout, 0, "error");
	error = nv_error(nvout);
	if (error != 0) {
		pjdlog_common(LOG_ERR, 0, error,
		    "Unable to prepare event header");
		goto fail;
	}
	if (hast_proto_send(res, res->hr_event, nvout, NULL, 0) == -1) {
		pjdlog_errno(LOG_ERR, "Unable to send event header");
		goto fail;
	}
	nv_free(nvin);
	nv_free(nvout);
	return (0);
fail:
	if (nvin != NULL)
		nv_free(nvin);
	if (nvout != NULL)
		nv_free(nvout);
	return (-1);
}