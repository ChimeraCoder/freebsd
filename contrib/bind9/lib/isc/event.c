
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

/* $Id: event.c,v 1.21 2007/06/19 23:47:17 tbox Exp $ */

/*!
 * \file
 * \author Principal Author: Bob Halley
 */

#include <config.h>

#include <isc/event.h>
#include <isc/mem.h>
#include <isc/util.h>

/***
 *** Events.
 ***/

static void
destroy(isc_event_t *event) {
	isc_mem_t *mctx = event->ev_destroy_arg;

	isc_mem_put(mctx, event, event->ev_size);
}

isc_event_t *
isc_event_allocate(isc_mem_t *mctx, void *sender, isc_eventtype_t type,
		   isc_taskaction_t action, const void *arg, size_t size)
{
	isc_event_t *event;
	void *deconst_arg;

	REQUIRE(size >= sizeof(struct isc_event));
	REQUIRE(action != NULL);

	event = isc_mem_get(mctx, size);
	if (event == NULL)
		return (NULL);

	/*
	 * Removing the const attribute from "arg" is the best of two
	 * evils here.  If the event->ev_arg member is made const, then
	 * it affects a great many users of the task/event subsystem
	 * which are not passing in an "arg" which starts its life as
	 * const.  Changing isc_event_allocate() and isc_task_onshutdown()
	 * to not have "arg" prototyped as const (which is quite legitimate,
	 * because neither of those functions modify arg) can cause
	 * compiler whining anytime someone does want to use a const
	 * arg that they themselves never modify, such as with
	 * gcc -Wwrite-strings and using a string "arg".
	 */
	DE_CONST(arg, deconst_arg);

	ISC_EVENT_INIT(event, size, 0, NULL, type, action, deconst_arg,
		       sender, destroy, mctx);

	return (event);
}

void
isc_event_free(isc_event_t **eventp) {
	isc_event_t *event;

	REQUIRE(eventp != NULL);
	event = *eventp;
	REQUIRE(event != NULL);

	if (event->ev_destroy != NULL)
		(event->ev_destroy)(event);

	*eventp = NULL;
}