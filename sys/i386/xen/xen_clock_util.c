
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/clock.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/time.h>

#include <xen/xen_intr.h>
#include <vm/vm.h>
#include <vm/pmap.h>
#include <machine/pmap.h>
#include <xen/hypervisor.h>
#include <machine/xen/xen-os.h>
#include <machine/xen/xenfunc.h>
#include <xen/interface/io/xenbus.h>
#include <xen/interface/vcpu.h>
#include <machine/cpu.h>

#include <machine/xen/xen_clock_util.h>

/*
 * Read the current hypervisor start time (wall clock) from Xen.
 */
void
xen_fetch_wallclock(struct timespec *ts)
{ 
        shared_info_t *s = HYPERVISOR_shared_info;
        uint32_t ts_version;
   
        do {
                ts_version = s->wc_version;
                rmb();
                ts->tv_sec  = s->wc_sec;
                ts->tv_nsec = s->wc_nsec;
                rmb();
        }
        while ((s->wc_version & 1) | (ts_version ^ s->wc_version));
}

/*
 * Read the current hypervisor system uptime value from Xen.
 */
void
xen_fetch_uptime(struct timespec *ts)
{
        shared_info_t           *s = HYPERVISOR_shared_info;
        struct vcpu_time_info   *src;
	struct shadow_time_info	dst;
        uint32_t pre_version, post_version;
        
        src = &s->vcpu_info[smp_processor_id()].time;

        spinlock_enter();
        do {
                pre_version = dst.version = src->version;
                rmb();
                dst.system_timestamp  = src->system_time;
                rmb();
                post_version = src->version;
        }
        while ((pre_version & 1) | (pre_version ^ post_version));

        spinlock_exit();

	ts->tv_sec = dst.system_timestamp / 1000000000;
	ts->tv_nsec = dst.system_timestamp % 1000000000;
}