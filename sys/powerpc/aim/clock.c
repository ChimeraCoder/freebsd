
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
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/interrupt.h>
#include <sys/pcpu.h>
#include <sys/sysctl.h>
#include <sys/timeet.h>
#include <sys/timetc.h>

#include <dev/ofw/openfirm.h>

#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/intr_machdep.h>
#include <machine/md_var.h>
#include <machine/smp.h>

/*
 * Initially we assume a processor with a bus frequency of 12.5 MHz.
 */
static int		initialized = 0;
static u_long		ns_per_tick = 80;
static u_long		ticks_per_sec = 12500000;
static u_long		*decr_counts[MAXCPU];

static int		decr_et_start(struct eventtimer *et,
    sbintime_t first, sbintime_t period);
static int		decr_et_stop(struct eventtimer *et);
static timecounter_get_t	decr_get_timecount;

struct decr_state {
	int	mode;	/* 0 - off, 1 - periodic, 2 - one-shot. */
	int32_t	div;	/* Periodic divisor. */
};
static DPCPU_DEFINE(struct decr_state, decr_state);

static struct eventtimer	decr_et;
static struct timecounter	decr_tc = {
	decr_get_timecount,	/* get_timecount */
	0,			/* no poll_pps */
	~0u,			/* counter_mask */
	0,			/* frequency */
	"timebase"		/* name */
};

/*
 * Decrementer interrupt handler.
 */
void
decr_intr(struct trapframe *frame)
{
	struct decr_state *s = DPCPU_PTR(decr_state);
	int		nticks = 0;
	int32_t		val;

	if (!initialized)
		return;

	(*decr_counts[curcpu])++;

	if (s->mode == 1) {
		/*
		 * Based on the actual time delay since the last decrementer
		 * reload, we arrange for earlier interrupt next time.
		 */
		__asm ("mfdec %0" : "=r"(val));
		while (val < 0) {
			val += s->div;
			nticks++;
		}
		mtdec(val);
	} else if (s->mode == 2) {
		nticks = 1;
		decr_et_stop(NULL);
	}

	while (nticks-- > 0) {
		if (decr_et.et_active)
			decr_et.et_event_cb(&decr_et, decr_et.et_arg);
	}
}

/*
 * BSP early initialization.
 */
void
decr_init(void)
{
	struct cpuref cpu;
	char buf[32];

	/*
	 * Check the BSP's timebase frequency. Sometimes we can't find the BSP, so fall
	 * back to the first CPU in this case.
	 */
	if (platform_smp_get_bsp(&cpu) != 0)
		platform_smp_first_cpu(&cpu);
	ticks_per_sec = platform_timebase_freq(&cpu);
	ns_per_tick = 1000000000 / ticks_per_sec;

	set_cputicker(mftb, ticks_per_sec, 0);
	snprintf(buf, sizeof(buf), "cpu%d:decrementer", curcpu);
	intrcnt_add(buf, &decr_counts[curcpu]);
	decr_et_stop(NULL);
	initialized = 1;
}

#ifdef SMP
/*
 * AP early initialization.
 */
void
decr_ap_init(void)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "cpu%d:decrementer", curcpu);
	intrcnt_add(buf, &decr_counts[curcpu]);
	decr_et_stop(NULL);
}
#endif

/*
 * Final initialization.
 */
void
decr_tc_init(void)
{

	decr_tc.tc_frequency = ticks_per_sec;
	tc_init(&decr_tc);
	decr_et.et_name = "decrementer";
	decr_et.et_flags = ET_FLAGS_PERIODIC | ET_FLAGS_ONESHOT |
	    ET_FLAGS_PERCPU;
	decr_et.et_quality = 1000;
	decr_et.et_frequency = ticks_per_sec;
	decr_et.et_min_period = (0x00000002LLU << 32) / ticks_per_sec;
	decr_et.et_max_period = (0x7fffffffLLU << 32) / ticks_per_sec;
	decr_et.et_start = decr_et_start;
	decr_et.et_stop = decr_et_stop;
	decr_et.et_priv = NULL;
	et_register(&decr_et);
}

/*
 * Event timer start method.
 */
static int
decr_et_start(struct eventtimer *et,
    sbintime_t first, sbintime_t period)
{
	struct decr_state *s = DPCPU_PTR(decr_state);
	uint32_t fdiv;

	if (period != 0) {
		s->mode = 1;
		s->div = (decr_et.et_frequency * period) >> 32;
	} else {
		s->mode = 2;
		s->div = 0;
	}
	if (first != 0) {
		fdiv = (decr_et.et_frequency * first) >> 32;
	} else
		fdiv = s->div;

	mtdec(fdiv);
	return (0);
}

/*
 * Event timer stop method.
 */
static int
decr_et_stop(struct eventtimer *et)
{
	struct decr_state *s = DPCPU_PTR(decr_state);

	s->mode = 0;
	s->div = 0x7fffffff;
	mtdec(s->div);
	return (0);
}

/*
 * Timecounter get method.
 */
static unsigned
decr_get_timecount(struct timecounter *tc)
{
	register_t tb;

	__asm __volatile("mftb %0" : "=r"(tb));
	return (tb);
}

/*
 * Wait for about n microseconds (at least!).
 */
void
DELAY(int n)
{
	u_quad_t	tb, ttb;

	tb = mftb();
	ttb = tb + (n * 1000 + ns_per_tick - 1) / ns_per_tick;
	while (tb < ttb)
		tb = mftb();
}