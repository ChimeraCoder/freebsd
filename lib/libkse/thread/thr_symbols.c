
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

#include "namespace.h"
#include <sys/types.h>
#include <stddef.h>
#include <pthread.h>
#include <rtld.h>
#include "un-namespace.h"

#include "thr_private.h"

/* A collection of symbols needed by debugger */

/* int _libkse_debug */
int _thread_off_tcb = offsetof(struct pthread, tcb);
int _thread_off_tmbx = offsetof(struct tcb, tcb_tmbx);
int _thread_off_next = offsetof(struct pthread, tle.tqe_next);
int _thread_off_attr_flags = offsetof(struct pthread, attr.flags);
int _thread_off_kse = offsetof(struct pthread, kse);
int _thread_off_kse_locklevel = offsetof(struct kse, k_locklevel);
int _thread_off_thr_locklevel = offsetof(struct pthread, locklevel);
int _thread_off_linkmap = offsetof(Obj_Entry, linkmap);
int _thread_off_tlsindex = offsetof(Obj_Entry, tlsindex);
int _thread_size_key = sizeof(struct pthread_key);
int _thread_off_key_allocated = offsetof(struct pthread_key, allocated);
int _thread_off_key_destructor = offsetof(struct pthread_key, destructor);
int _thread_max_keys = PTHREAD_KEYS_MAX;
int _thread_off_dtv = DTV_OFFSET;
int _thread_off_state = offsetof(struct pthread, state);
int _thread_state_running = PS_RUNNING;
int _thread_state_zoombie = PS_DEAD;
int _thread_off_sigmask = offsetof(struct pthread, sigmask);
int _thread_off_sigpend = offsetof(struct pthread, sigpend);