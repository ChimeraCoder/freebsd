
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

/* $Id: ondestroy.c,v 1.16 2007/06/19 23:47:17 tbox Exp $ */

/*! \file */

#include <config.h>

#include <stddef.h>

#include <isc/event.h>
#include <isc/magic.h>
#include <isc/ondestroy.h>
#include <isc/task.h>
#include <isc/util.h>

#define ONDESTROY_MAGIC		ISC_MAGIC('D', 'e', 'S', 't')
#define VALID_ONDESTROY(s)	ISC_MAGIC_VALID(s, ONDESTROY_MAGIC)

void
isc_ondestroy_init(isc_ondestroy_t *ondest) {
	ondest->magic = ONDESTROY_MAGIC;
	ISC_LIST_INIT(ondest->events);
}

isc_result_t
isc_ondestroy_register(isc_ondestroy_t *ondest, isc_task_t *task,
		       isc_event_t **eventp)
{
	isc_event_t *theevent;
	isc_task_t *thetask = NULL;

	REQUIRE(VALID_ONDESTROY(ondest));
	REQUIRE(task != NULL);
	REQUIRE(eventp != NULL);

	theevent = *eventp;

	REQUIRE(theevent != NULL);

	isc_task_attach(task, &thetask);

	theevent->ev_sender = thetask;

	ISC_LIST_APPEND(ondest->events, theevent, ev_link);

	return (ISC_R_SUCCESS);
}

void
isc_ondestroy_notify(isc_ondestroy_t *ondest, void *sender) {
	isc_event_t *eventp;
	isc_task_t *task;

	REQUIRE(VALID_ONDESTROY(ondest));

	eventp = ISC_LIST_HEAD(ondest->events);
	while (eventp != NULL) {
		ISC_LIST_UNLINK(ondest->events, eventp, ev_link);

		task = eventp->ev_sender;
		eventp->ev_sender = sender;

		isc_task_sendanddetach(&task, &eventp);

		eventp = ISC_LIST_HEAD(ondest->events);
	}
}