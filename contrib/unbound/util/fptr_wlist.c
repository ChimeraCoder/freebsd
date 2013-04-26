
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
 * This file contains functions that check function pointers.
 * The functions contain a whitelist of known good callback values.
 * Any other values lead to an error. 
 *
 * Due to the listing nature, this file violates all the modularization
 * boundaries in the program.
 */
#include "config.h"
#include "util/fptr_wlist.h"
#include "util/mini_event.h"
#include "daemon/worker.h"
#include "daemon/remote.h"
#include "services/outside_network.h"
#include "services/mesh.h"
#include "services/localzone.h"
#include "services/cache/infra.h"
#include "services/cache/rrset.h"
#include "iterator/iterator.h"
#include "iterator/iter_fwd.h"
#include "validator/validator.h"
#include "validator/val_anchor.h"
#include "validator/val_nsec3.h"
#include "validator/val_sigcrypt.h"
#include "validator/val_kentry.h"
#include "validator/val_neg.h"
#include "validator/autotrust.h"
#include "util/data/msgreply.h"
#include "util/data/packed_rrset.h"
#include "util/storage/slabhash.h"
#include "util/storage/dnstree.h"
#include "util/locks.h"
#include "libunbound/libworker.h"
#include "libunbound/context.h"
#include "util/tube.h"
#include "util/config_file.h"
#ifdef UB_ON_WINDOWS
#include "winrc/win_svc.h"
#endif

#ifdef WITH_PYTHONMODULE
#include "pythonmod/pythonmod.h"
#endif

int 
fptr_whitelist_comm_point(comm_point_callback_t *fptr)
{
	if(fptr == &worker_handle_request) return 1;
	else if(fptr == &outnet_udp_cb) return 1;
	else if(fptr == &outnet_tcp_cb) return 1;
	else if(fptr == &tube_handle_listen) return 1;
	return 0;
}

int 
fptr_whitelist_comm_point_raw(comm_point_callback_t *fptr)
{
	if(fptr == &tube_handle_listen) return 1;
	else if(fptr == &tube_handle_write) return 1;
	else if(fptr == &remote_accept_callback) return 1;
	else if(fptr == &remote_control_callback) return 1;
	return 0;
}

int 
fptr_whitelist_comm_timer(void (*fptr)(void*))
{
	if(fptr == &pending_udp_timer_cb) return 1;
	else if(fptr == &outnet_tcptimer) return 1;
	else if(fptr == &worker_stat_timer_cb) return 1;
	else if(fptr == &worker_probe_timer_cb) return 1;
#ifdef UB_ON_WINDOWS
	else if(fptr == &wsvc_cron_cb) return 1;
#endif
	return 0;
}

int 
fptr_whitelist_comm_signal(void (*fptr)(int, void*))
{
	if(fptr == &worker_sighandler) return 1;
	return 0;
}

int fptr_whitelist_start_accept(void (*fptr)(void*))
{
	if(fptr == &worker_start_accept) return 1;
	return 0;
}

int fptr_whitelist_stop_accept(void (*fptr)(void*))
{
	if(fptr == &worker_stop_accept) return 1;
	return 0;
}

int 
fptr_whitelist_event(void (*fptr)(int, short, void *))
{
	if(fptr == &comm_point_udp_callback) return 1;
	else if(fptr == &comm_point_udp_ancil_callback) return 1;
	else if(fptr == &comm_point_tcp_accept_callback) return 1;
	else if(fptr == &comm_point_tcp_handle_callback) return 1;
	else if(fptr == &comm_timer_callback) return 1;
	else if(fptr == &comm_signal_callback) return 1;
	else if(fptr == &comm_point_local_handle_callback) return 1;
	else if(fptr == &comm_point_raw_handle_callback) return 1;
	else if(fptr == &tube_handle_signal) return 1;
	else if(fptr == &comm_base_handle_slow_accept) return 1;
#ifdef UB_ON_WINDOWS
	else if(fptr == &worker_win_stop_cb) return 1;
#endif
	return 0;
}

int 
fptr_whitelist_pending_udp(comm_point_callback_t *fptr)
{
	if(fptr == &serviced_udp_callback) return 1;
	else if(fptr == &worker_handle_reply) return 1;
	else if(fptr == &libworker_handle_reply) return 1;
	return 0;
}

int 
fptr_whitelist_pending_tcp(comm_point_callback_t *fptr)
{
	if(fptr == &serviced_tcp_callback) return 1;
	else if(fptr == &worker_handle_reply) return 1;
	else if(fptr == &libworker_handle_reply) return 1;
	return 0;
}

int 
fptr_whitelist_serviced_query(comm_point_callback_t *fptr)
{
	if(fptr == &worker_handle_service_reply) return 1;
	else if(fptr == &libworker_handle_service_reply) return 1;
	return 0;
}

int 
fptr_whitelist_rbtree_cmp(int (*fptr) (const void *, const void *))
{
	if(fptr == &mesh_state_compare) return 1;
	else if(fptr == &mesh_state_ref_compare) return 1;
	else if(fptr == &addr_tree_compare) return 1;
	else if(fptr == &local_zone_cmp) return 1;
	else if(fptr == &local_data_cmp) return 1;
	else if(fptr == &fwd_cmp) return 1;
	else if(fptr == &pending_cmp) return 1;
	else if(fptr == &serviced_cmp) return 1;
	else if(fptr == &name_tree_compare) return 1;
	else if(fptr == &order_lock_cmp) return 1;
	else if(fptr == &codeline_cmp) return 1;
	else if(fptr == &nsec3_hash_cmp) return 1;
	else if(fptr == &mini_ev_cmp) return 1;
	else if(fptr == &anchor_cmp) return 1;
	else if(fptr == &canonical_tree_compare) return 1;
	else if(fptr == &context_query_cmp) return 1;
	else if(fptr == &val_neg_data_compare) return 1;
	else if(fptr == &val_neg_zone_compare) return 1;
	else if(fptr == &probetree_cmp) return 1;
	else if(fptr == &replay_var_compare) return 1;
	return 0;
}

int 
fptr_whitelist_hash_sizefunc(lruhash_sizefunc_t fptr)
{
	if(fptr == &msgreply_sizefunc) return 1;
	else if(fptr == &ub_rrset_sizefunc) return 1;
	else if(fptr == &infra_sizefunc) return 1;
	else if(fptr == &key_entry_sizefunc) return 1;
	else if(fptr == &test_slabhash_sizefunc) return 1;
	return 0;
}

int 
fptr_whitelist_hash_compfunc(lruhash_compfunc_t fptr)
{
	if(fptr == &query_info_compare) return 1;
	else if(fptr == &ub_rrset_compare) return 1;
	else if(fptr == &infra_compfunc) return 1;
	else if(fptr == &key_entry_compfunc) return 1;
	else if(fptr == &test_slabhash_compfunc) return 1;
	return 0;
}

int 
fptr_whitelist_hash_delkeyfunc(lruhash_delkeyfunc_t fptr)
{
	if(fptr == &query_entry_delete) return 1;
	else if(fptr == &ub_rrset_key_delete) return 1;
	else if(fptr == &infra_delkeyfunc) return 1;
	else if(fptr == &key_entry_delkeyfunc) return 1;
	else if(fptr == &test_slabhash_delkey) return 1;
	return 0;
}

int 
fptr_whitelist_hash_deldatafunc(lruhash_deldatafunc_t fptr)
{
	if(fptr == &reply_info_delete) return 1;
	else if(fptr == &rrset_data_delete) return 1;
	else if(fptr == &infra_deldatafunc) return 1;
	else if(fptr == &key_entry_deldatafunc) return 1;
	else if(fptr == &test_slabhash_deldata) return 1;
	return 0;
}

int 
fptr_whitelist_hash_markdelfunc(lruhash_markdelfunc_t fptr)
{
	if(fptr == NULL) return 1;
	else if(fptr == &rrset_markdel) return 1;
	return 0;
}

/** whitelist env->send_query callbacks */
int 
fptr_whitelist_modenv_send_query(struct outbound_entry* (*fptr)(
        uint8_t* qname, size_t qnamelen, uint16_t qtype, uint16_t qclass,
        uint16_t flags, int dnssec, int want_dnssec, 
	struct sockaddr_storage* addr, socklen_t addrlen, 
	uint8_t* zone, size_t zonelen,
	struct module_qstate* q))
{
	if(fptr == &worker_send_query) return 1;
	else if(fptr == &libworker_send_query) return 1;
	return 0;
}

int 
fptr_whitelist_modenv_detach_subs(void (*fptr)(
        struct module_qstate* qstate))
{
	if(fptr == &mesh_detach_subs) return 1;
	return 0;
}

int 
fptr_whitelist_modenv_attach_sub(int (*fptr)(
        struct module_qstate* qstate, struct query_info* qinfo,
        uint16_t qflags, int prime, struct module_qstate** newq))
{
	if(fptr == &mesh_attach_sub) return 1;
	return 0;
}

int 
fptr_whitelist_modenv_kill_sub(void (*fptr)(struct module_qstate* newq))
{
	if(fptr == &mesh_state_delete) return 1;
	return 0;
}

int 
fptr_whitelist_modenv_detect_cycle(int (*fptr)(        
	struct module_qstate* qstate, struct query_info* qinfo,         
	uint16_t flags, int prime))
{
	if(fptr == &mesh_detect_cycle) return 1;
	return 0;
}

int 
fptr_whitelist_mod_init(int (*fptr)(struct module_env* env, int id))
{
	if(fptr == &iter_init) return 1;
	else if(fptr == &val_init) return 1;
#ifdef WITH_PYTHONMODULE
	else if(fptr == &pythonmod_init) return 1;
#endif
	return 0;
}

int 
fptr_whitelist_mod_deinit(void (*fptr)(struct module_env* env, int id))
{
	if(fptr == &iter_deinit) return 1;
	else if(fptr == &val_deinit) return 1;
#ifdef WITH_PYTHONMODULE
	else if(fptr == &pythonmod_deinit) return 1;
#endif
	return 0;
}

int 
fptr_whitelist_mod_operate(void (*fptr)(struct module_qstate* qstate,
        enum module_ev event, int id, struct outbound_entry* outbound))
{
	if(fptr == &iter_operate) return 1;
	else if(fptr == &val_operate) return 1;
#ifdef WITH_PYTHONMODULE
	else if(fptr == &pythonmod_operate) return 1;
#endif
	return 0;
}

int 
fptr_whitelist_mod_inform_super(void (*fptr)(
        struct module_qstate* qstate, int id, struct module_qstate* super))
{
	if(fptr == &iter_inform_super) return 1;
	else if(fptr == &val_inform_super) return 1;
#ifdef WITH_PYTHONMODULE
	else if(fptr == &pythonmod_inform_super) return 1;
#endif
	return 0;
}

int 
fptr_whitelist_mod_clear(void (*fptr)(struct module_qstate* qstate,
        int id))
{
	if(fptr == &iter_clear) return 1;
	else if(fptr == &val_clear) return 1;
#ifdef WITH_PYTHONMODULE
	else if(fptr == &pythonmod_clear) return 1;
#endif
	return 0;
}

int 
fptr_whitelist_mod_get_mem(size_t (*fptr)(struct module_env* env, int id))
{
	if(fptr == &iter_get_mem) return 1;
	else if(fptr == &val_get_mem) return 1;
#ifdef WITH_PYTHONMODULE
	else if(fptr == &pythonmod_get_mem) return 1;
#endif
	return 0;
}

int 
fptr_whitelist_alloc_cleanup(void (*fptr)(void*))
{
	if(fptr == &worker_alloc_cleanup) return 1;
	return 0;
}

int fptr_whitelist_tube_listen(tube_callback_t* fptr)
{
	if(fptr == &worker_handle_control_cmd) return 1;
	else if(fptr == &libworker_handle_control_cmd) return 1;
	return 0;
}

int fptr_whitelist_mesh_cb(mesh_cb_func_t fptr)
{
	if(fptr == &libworker_fg_done_cb) return 1;
	else if(fptr == &libworker_bg_done_cb) return 1;
	else if(fptr == &probe_answer_cb) return 1;
	return 0;
}

int fptr_whitelist_print_func(void (*fptr)(char*,void*))
{
	if(fptr == &config_print_func) return 1;
	else if(fptr == &config_collate_func) return 1;
	else if(fptr == &remote_get_opt_ssl) return 1;
	return 0;
}