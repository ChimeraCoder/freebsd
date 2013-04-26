
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

/**
 * \file
 *
 * This file contains fake callback functions, so that the symbols exist
 * and the fptr_wlist continues to work even if the daemon/worker is not
 * linked into the resulting program.
 */
#include "config.h"
#include "util/log.h"
#include "services/mesh.h"
struct comm_reply;
struct comm_point;
struct module_qstate;
struct tube;

void worker_handle_control_cmd(struct tube* ATTR_UNUSED(tube),
	uint8_t* ATTR_UNUSED(buffer), size_t ATTR_UNUSED(len),
	int ATTR_UNUSED(error), void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

int worker_handle_request(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(repinfo))
{
	log_assert(0);
	return 0;
}

int worker_handle_reply(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(reply_info))
{
	log_assert(0);
	return 0;
}

int worker_handle_service_reply(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(reply_info))
{
	log_assert(0);
	return 0;
}

int remote_accept_callback(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(repinfo))
{
	log_assert(0);
	return 0;
}

int remote_control_callback(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(repinfo))
{
	log_assert(0);
	return 0;
}

void worker_sighandler(int ATTR_UNUSED(sig), void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

struct outbound_entry* worker_send_query(uint8_t* ATTR_UNUSED(qname), 
	size_t ATTR_UNUSED(qnamelen), uint16_t ATTR_UNUSED(qtype), 
	uint16_t ATTR_UNUSED(qclass), uint16_t ATTR_UNUSED(flags), 
	int ATTR_UNUSED(dnssec), int ATTR_UNUSED(want_dnssec), 
	struct sockaddr_storage* ATTR_UNUSED(addr), 
	socklen_t ATTR_UNUSED(addrlen), struct module_qstate* ATTR_UNUSED(q))
{
	log_assert(0);
	return 0;
}

#ifdef UB_ON_WINDOWS
void
worker_win_stop_cb(int ATTR_UNUSED(fd), short ATTR_UNUSED(ev), void* 
	ATTR_UNUSED(arg)) {
	log_assert(0);
}

void
wsvc_cron_cb(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}
#endif /* UB_ON_WINDOWS */

void 
worker_alloc_cleanup(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

struct outbound_entry* libworker_send_query(uint8_t* ATTR_UNUSED(qname), 
	size_t ATTR_UNUSED(qnamelen), uint16_t ATTR_UNUSED(qtype), 
	uint16_t ATTR_UNUSED(qclass), uint16_t ATTR_UNUSED(flags), 
	int ATTR_UNUSED(dnssec), int ATTR_UNUSED(want_dnssec),
	struct sockaddr_storage* ATTR_UNUSED(addr), 
	socklen_t ATTR_UNUSED(addrlen), struct module_qstate* ATTR_UNUSED(q))
{
	log_assert(0);
	return 0;
}

int libworker_handle_reply(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(reply_info))
{
	log_assert(0);
	return 0;
}

int libworker_handle_service_reply(struct comm_point* ATTR_UNUSED(c), 
	void* ATTR_UNUSED(arg), int ATTR_UNUSED(error),
        struct comm_reply* ATTR_UNUSED(reply_info))
{
	log_assert(0);
	return 0;
}

void libworker_handle_control_cmd(struct tube* ATTR_UNUSED(tube),
        uint8_t* ATTR_UNUSED(buffer), size_t ATTR_UNUSED(len),
        int ATTR_UNUSED(error), void* ATTR_UNUSED(arg))
{
        log_assert(0);
}

void libworker_fg_done_cb(void* ATTR_UNUSED(arg), int ATTR_UNUSED(rcode), 
	ldns_buffer* ATTR_UNUSED(buf), enum sec_status ATTR_UNUSED(s),
	char* ATTR_UNUSED(why_bogus))
{
	log_assert(0);
}

void libworker_bg_done_cb(void* ATTR_UNUSED(arg), int ATTR_UNUSED(rcode), 
	ldns_buffer* ATTR_UNUSED(buf), enum sec_status ATTR_UNUSED(s),
	char* ATTR_UNUSED(why_bogus))
{
	log_assert(0);
}

int context_query_cmp(const void* ATTR_UNUSED(a), const void* ATTR_UNUSED(b))
{
	log_assert(0);
	return 0;
}

void worker_stat_timer_cb(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

void worker_probe_timer_cb(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

void worker_start_accept(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

void worker_stop_accept(void* ATTR_UNUSED(arg))
{
	log_assert(0);
}

/** keep track of lock id in lock-verify application */
struct order_id {
        /** the thread id that created it */
        int thr;
        /** the instance number of creation */
        int instance;
};

int order_lock_cmp(const void* e1, const void* e2)
{
        struct order_id* o1 = (struct order_id*)e1;
        struct order_id* o2 = (struct order_id*)e2;
        if(o1->thr < o2->thr) return -1;
        if(o1->thr > o2->thr) return 1;
        if(o1->instance < o2->instance) return -1;
        if(o1->instance > o2->instance) return 1;
        return 0;
}

int
codeline_cmp(const void* a, const void* b)
{
        return strcmp((const char*)a, (const char*)b);
}

int replay_var_compare(const void* ATTR_UNUSED(a), const void* ATTR_UNUSED(b))
{
        log_assert(0);
        return 0;
}

void remote_get_opt_ssl(char* ATTR_UNUSED(str), void* ATTR_UNUSED(arg))
{
        log_assert(0);
}