
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
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/interrupt.h>
#include <sys/priority.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/sysctl.h>
#include <sys/systm.h>
#include <sys/timeet.h>
#include <sys/timetc.h>
#include <sys/pcpu.h>

#include <machine/cpu.h>
#include <machine/efi.h>
#include <machine/intr.h>
#include <machine/intrcnt.h>
#include <machine/md_var.h>
#include <machine/smp.h>

#define	CLOCK_ET_OFF		0
#define	CLOCK_ET_PERIODIC	1
#define	CLOCK_ET_ONESHOT	2

static struct eventtimer ia64_clock_et;
static u_int ia64_clock_xiv;

#ifndef SMP
static timecounter_get_t ia64_get_timecount;

static struct timecounter ia64_timecounter = {
	ia64_get_timecount,	/* get_timecount */
	0,			/* no poll_pps */
	~0u,			/* counter_mask */
	0,			/* frequency */
	"ITC"			/* name */
};

static u_int
ia64_get_timecount(struct timecounter* tc)
{
	return ia64_get_itc();
}
#endif

static u_int
ia64_ih_clock(struct thread *td, u_int xiv, struct trapframe *tf)
{
	struct eventtimer *et;
	uint64_t itc, load;
	uint32_t mode;

	PCPU_INC(md.stats.pcs_nclks);
	intrcnt[INTRCNT_CLOCK]++;

	itc = ia64_get_itc();
	PCPU_SET(md.clock, itc);

	mode = PCPU_GET(md.clock_mode);
	if (mode == CLOCK_ET_PERIODIC) {
		load = PCPU_GET(md.clock_load);
		ia64_set_itm(itc + load);
	} else
		ia64_set_itv((1 << 16) | xiv);

	ia64_set_eoi(0);
	ia64_srlz_d();

	et = &ia64_clock_et;
	if (et->et_active)
		et->et_event_cb(et, et->et_arg);
	return (1);
}

/*
 * Event timer start method.
 */
static int
ia64_clock_start(struct eventtimer *et, sbintime_t first, sbintime_t period)
{
	u_long itc, load;
	register_t is;

	if (period != 0) {
		PCPU_SET(md.clock_mode, CLOCK_ET_PERIODIC);
		load = (et->et_frequency * period) >> 32;
	} else {
		PCPU_SET(md.clock_mode, CLOCK_ET_ONESHOT);
		load = 0;
	}

	PCPU_SET(md.clock_load, load);

	if (first != 0)
		load = (et->et_frequency * first) >> 32;

	is = intr_disable();
	itc = ia64_get_itc();
	ia64_set_itm(itc + load);
	ia64_set_itv(ia64_clock_xiv);
	ia64_srlz_d();
	intr_restore(is);
	return (0);
}

/*
 * Event timer stop method.
 */
static int
ia64_clock_stop(struct eventtimer *et)
{

	ia64_set_itv((1 << 16) | ia64_clock_xiv);
	ia64_srlz_d();
	PCPU_SET(md.clock_mode, CLOCK_ET_OFF);
	PCPU_SET(md.clock_load, 0);
	return (0);
}

/*
 * We call cpu_initclocks() on the APs as well. It allows us to
 * group common initialization in the same function.
 */
void
cpu_initclocks()
{

	ia64_clock_stop(NULL);
	if (PCPU_GET(cpuid) == 0)
		cpu_initclocks_bsp();
	else
		cpu_initclocks_ap();
}

static void
clock_configure(void *dummy)
{
	struct eventtimer *et;
	u_long itc_freq;

	ia64_clock_xiv = ia64_xiv_alloc(PI_REALTIME, IA64_XIV_IPI,
	    ia64_ih_clock);
	if (ia64_clock_xiv == 0)
		panic("No XIV for clock interrupts");

	itc_freq = (u_long)ia64_itc_freq() * 1000000ul;

	et = &ia64_clock_et;
	et->et_name = "ITC";
	et->et_flags = ET_FLAGS_PERIODIC | ET_FLAGS_ONESHOT | ET_FLAGS_PERCPU;
	et->et_quality = 1000;
	et->et_frequency = itc_freq;
	et->et_min_period = SBT_1S / (10 * hz);
	et->et_max_period = (0xfffffffeul << 32) / itc_freq;
	et->et_start = ia64_clock_start;
	et->et_stop = ia64_clock_stop;
	et->et_priv = NULL;
	et_register(et);

#ifndef SMP
	ia64_timecounter.tc_frequency = itc_freq;
	tc_init(&ia64_timecounter);
#endif
}
SYSINIT(clkcfg, SI_SUB_CONFIGURE, SI_ORDER_SECOND, clock_configure, NULL);