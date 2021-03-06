
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

#include "includes.h"
#include <assert.h>

#include "common.h"
#include "eloop.h"
#include "uuid.h"
#include "http_client.h"
#include "wps_defs.h"
#include "wps_upnp.h"
#include "wps_upnp_i.h"

/*
 * Event message generation (to subscribers)
 *
 * We make a separate copy for each message for each subscriber. This memory
 * wasted could be limited (adding code complexity) by sharing copies, keeping
 * a usage count and freeing when zero.
 *
 * Sending a message requires using a HTTP over TCP NOTIFY
 * (like a PUT) which requires a number of states..
 */

#define MAX_EVENTS_QUEUED 20   /* How far behind queued events */
#define EVENT_TIMEOUT_SEC 30   /* Drop sending event after timeout */

/* How long to wait before sending event */
#define EVENT_DELAY_SECONDS 0
#define EVENT_DELAY_MSEC 0

/*
 * Event information that we send to each subscriber is remembered in this
 * struct. The event cannot be sent by simple UDP; it has to be sent by a HTTP
 * over TCP transaction which requires various states.. It may also need to be
 * retried at a different address (if more than one is available).
 *
 * TODO: As an optimization we could share data between subscribers.
 */
struct wps_event_ {
	struct dl_list list;
	struct subscription *s;         /* parent */
	unsigned subscriber_sequence;   /* which event for this subscription*/
	unsigned int retry;             /* which retry */
	struct subscr_addr *addr;       /* address to connect to */
	struct wpabuf *data;            /* event data to send */
	struct http_client *http_event;
};


/* event_clean -- clean sockets etc. of event
 * Leaves data, retry count etc. alone.
 */
static void event_clean(struct wps_event_ *e)
{
	if (e->s->current_event == e)
		e->s->current_event = NULL;
	http_client_free(e->http_event);
	e->http_event = NULL;
}


/* event_delete -- delete single unqueued event
 * (be sure to dequeue first if need be)
 */
static void event_delete(struct wps_event_ *e)
{
	event_clean(e);
	wpabuf_free(e->data);
	os_free(e);
}


/* event_dequeue -- get next event from the queue
 * Returns NULL if empty.
 */
static struct wps_event_ *event_dequeue(struct subscription *s)
{
	struct wps_event_ *e;
	e = dl_list_first(&s->event_queue, struct wps_event_, list);
	if (e)
		dl_list_del(&e->list);
	return e;
}


/* event_delete_all -- delete entire event queue and current event */
void event_delete_all(struct subscription *s)
{
	struct wps_event_ *e;
	while ((e = event_dequeue(s)) != NULL)
		event_delete(e);
	if (s->current_event) {
		event_delete(s->current_event);
		/* will set: s->current_event = NULL;  */
	}
}


/**
 * event_retry - Called when we had a failure delivering event msg
 * @e: Event
 * @do_next_address: skip address e.g. on connect fail
 */
static void event_retry(struct wps_event_ *e, int do_next_address)
{
	struct subscription *s = e->s;
	struct upnp_wps_device_sm *sm = s->sm;

	event_clean(e);
	/* will set: s->current_event = NULL; */

	if (do_next_address)
		e->retry++;
	if (e->retry >= dl_list_len(&s->addr_list)) {
		wpa_printf(MSG_DEBUG, "WPS UPnP: Giving up on sending event "
			   "for %s", e->addr->domain_and_port);
		return;
	}
	dl_list_add(&s->event_queue, &e->list);
	event_send_all_later(sm);
}


static struct wpabuf * event_build_message(struct wps_event_ *e)
{
	struct wpabuf *buf;
	char *b;

	buf = wpabuf_alloc(1000 + wpabuf_len(e->data));
	if (buf == NULL)
		return NULL;
	wpabuf_printf(buf, "NOTIFY %s HTTP/1.1\r\n", e->addr->path);
	wpabuf_put_str(buf, "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n");
	wpabuf_printf(buf, "HOST: %s\r\n", e->addr->domain_and_port);
	wpabuf_put_str(buf, "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
		       "NT: upnp:event\r\n"
		       "NTS: upnp:propchange\r\n");
	wpabuf_put_str(buf, "SID: uuid:");
	b = wpabuf_put(buf, 0);
	uuid_bin2str(e->s->uuid, b, 80);
	wpabuf_put(buf, os_strlen(b));
	wpabuf_put_str(buf, "\r\n");
	wpabuf_printf(buf, "SEQ: %u\r\n", e->subscriber_sequence);
	wpabuf_printf(buf, "CONTENT-LENGTH: %d\r\n",
		      (int) wpabuf_len(e->data));
	wpabuf_put_str(buf, "\r\n"); /* terminating empty line */
	wpabuf_put_buf(buf, e->data);
	return buf;
}


static void event_http_cb(void *ctx, struct http_client *c,
			  enum http_client_event event)
{
	struct wps_event_ *e = ctx;
	struct subscription *s = e->s;

	switch (event) {
	case HTTP_CLIENT_OK:
		wpa_printf(MSG_DEBUG,
			   "WPS UPnP: Got event reply OK from "
			   "%s", e->addr->domain_and_port);
		event_delete(e);

		/* Schedule sending more if there is more to send */
		if (!dl_list_empty(&s->event_queue))
			event_send_all_later(s->sm);
		break;
	case HTTP_CLIENT_FAILED:
	case HTTP_CLIENT_INVALID_REPLY:
		wpa_printf(MSG_DEBUG, "WPS UPnP: Failed to send event to %s",
			   e->addr->domain_and_port);

		/*
		 * If other side doesn't like what we say, forget about them.
		 * (There is no way to tell other side that we are dropping
		 * them...).
		 * Alternately, we could just do event_delete(e)
		 */
		wpa_printf(MSG_DEBUG, "WPS UPnP: Deleting subscription due to "
			   "errors");
		dl_list_del(&s->list);
		subscription_destroy(s);
		break;
	case HTTP_CLIENT_TIMEOUT:
		wpa_printf(MSG_DEBUG, "WPS UPnP: Event send timeout");
		event_retry(e, 1);
	}
}


/* event_send_start -- prepare to send a event message to subscriber
 *
 * This gets complicated because:
 * -- The message is sent via TCP and we have to keep the stream open
 *      for 30 seconds to get a response... then close it.
 * -- But we might have other event happen in the meantime...
 *      we have to queue them, if we lose them then the subscriber will
 *      be forced to unsubscribe and subscribe again.
 * -- If multiple URLs are provided then we are supposed to try successive
 *      ones after 30 second timeout.
 * -- The URLs might use domain names instead of dotted decimal addresses,
 *      and resolution of those may cause unwanted sleeping.
 * -- Doing the initial TCP connect can take a while, so we have to come
 *      back after connection and then send the data.
 *
 * Returns nonzero on error;
 *
 * Prerequisite: No current event send (s->current_event == NULL)
 *      and non-empty queue.
 */
static int event_send_start(struct subscription *s)
{
	struct wps_event_ *e;
	unsigned int itry;
	struct wpabuf *buf;

	/*
	 * Assume we are called ONLY with no current event and ONLY with
	 * nonempty event queue and ONLY with at least one address to send to.
	 */
	assert(!dl_list_empty(&s->addr_list));
	assert(s->current_event == NULL);
	assert(!dl_list_empty(&s->event_queue));

	s->current_event = e = event_dequeue(s);

	/* Use address according to number of retries */
	itry = 0;
	dl_list_for_each(e->addr, &s->addr_list, struct subscr_addr, list)
		if (itry++ == e->retry)
			break;
	if (itry < e->retry)
		return -1;

	buf = event_build_message(e);
	if (buf == NULL) {
		event_retry(e, 0);
		return -1;
	}

	e->http_event = http_client_addr(&e->addr->saddr, buf, 0,
					 event_http_cb, e);
	if (e->http_event == NULL) {
		wpabuf_free(buf);
		event_retry(e, 0);
		return -1;
	}

	return 0;
}


/* event_send_all_later_handler -- actually send events as needed */
static void event_send_all_later_handler(void *eloop_data, void *user_ctx)
{
	struct upnp_wps_device_sm *sm = user_ctx;
	struct subscription *s, *tmp;
	int nerrors = 0;

	sm->event_send_all_queued = 0;
	dl_list_for_each_safe(s, tmp, &sm->subscriptions, struct subscription,
			      list) {
		if (dl_list_empty(&s->addr_list)) {
			/* if we've given up on all addresses */
			wpa_printf(MSG_DEBUG, "WPS UPnP: Removing "
				   "subscription with no addresses");
			dl_list_del(&s->list);
			subscription_destroy(s);
		} else {
			if (s->current_event == NULL /* not busy */ &&
			    !dl_list_empty(&s->event_queue) /* more to do */) {
				if (event_send_start(s))
					nerrors++;
			}
		}
	}

	if (nerrors) {
		/* Try again later */
		event_send_all_later(sm);
	}
}


/* event_send_all_later -- schedule sending events to all subscribers
 * that need it.
 * This avoids two problems:
 * -- After getting a subscription, we should not send the first event
 *      until after our reply is fully queued to be sent back,
 * -- Possible stack depth or infinite recursion issues.
 */
void event_send_all_later(struct upnp_wps_device_sm *sm)
{
	/*
	 * The exact time in the future isn't too important. Waiting a bit
	 * might let us do several together.
	 */
	if (sm->event_send_all_queued)
		return;
	sm->event_send_all_queued = 1;
	eloop_register_timeout(EVENT_DELAY_SECONDS, EVENT_DELAY_MSEC,
			       event_send_all_later_handler, NULL, sm);
}


/* event_send_stop_all -- cleanup */
void event_send_stop_all(struct upnp_wps_device_sm *sm)
{
	if (sm->event_send_all_queued)
		eloop_cancel_timeout(event_send_all_later_handler, NULL, sm);
	sm->event_send_all_queued = 0;
}


/**
 * event_add - Add a new event to a queue
 * @s: Subscription
 * @data: Event data (is copied; caller retains ownership)
 * Returns: 0 on success, 1 on error
 */
int event_add(struct subscription *s, const struct wpabuf *data)
{
	struct wps_event_ *e;

	if (dl_list_len(&s->event_queue) >= MAX_EVENTS_QUEUED) {
		wpa_printf(MSG_DEBUG, "WPS UPnP: Too many events queued for "
			   "subscriber");
		return 1;
	}

	e = os_zalloc(sizeof(*e));
	if (e == NULL)
		return 1;
	dl_list_init(&e->list);
	e->s = s;
	e->data = wpabuf_dup(data);
	if (e->data == NULL) {
		os_free(e);
		return 1;
	}
	e->subscriber_sequence = s->next_subscriber_sequence++;
	if (s->next_subscriber_sequence == 0)
		s->next_subscriber_sequence++;
	dl_list_add_tail(&s->event_queue, &e->list);
	event_send_all_later(s->sm);
	return 0;
}