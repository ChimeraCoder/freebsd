
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
#include <sys/kthread.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/poll.h>
#include <sys/queue.h>
#include <sys/random.h>
#include <sys/selinfo.h>
#include <sys/syslog.h>
#include <sys/systm.h>
#include <sys/sysctl.h>

#include <machine/cpu.h>

#include <dev/random/randomdev_soft.h>

static int read_random_phony(void *, int);

/* Structure holding the desired entropy sources */
struct harvest_select harvest = { 1, 1, 1, 0 };
static int warned = 0;

/* hold the address of the routine which is actually called if
 * the randomdev is loaded
 */
static void (*reap_func)(u_int64_t, const void *, u_int, u_int, u_int,
    enum esource) = NULL;
static int (*read_func)(void *, int) = read_random_phony;

/* Initialise the harvester at load time */
void
random_yarrow_init_harvester(void (*reaper)(u_int64_t, const void *, u_int,
    u_int, u_int, enum esource), int (*reader)(void *, int))
{
	reap_func = reaper;
	read_func = reader;
}

/* Deinitialise the harvester at unload time */
void
random_yarrow_deinit_harvester(void)
{
	reap_func = NULL;
	read_func = read_random_phony;
	warned = 0;
}

/* Entropy harvesting routine. This is supposed to be fast; do
 * not do anything slow in here!
 * Implemented as in indirect call to allow non-inclusion of
 * the entropy device.
 *
 * XXXRW: get_cyclecount() is cheap on most modern hardware, where cycle
 * counters are built in, but on older hardware it will do a real time clock
 * read which can be quite expensive.
 */
void
random_harvest(void *entropy, u_int count, u_int bits, u_int frac,
    enum esource origin)
{
	if (reap_func)
		(*reap_func)(get_cyclecount(), entropy, count, bits, frac,
		    origin);
}

/* Userland-visible version of read_random */
int
read_random(void *buf, int count)
{
	return ((*read_func)(buf, count));
}

/* If the entropy device is not loaded, make a token effort to
 * provide _some_ kind of randomness. This should only be used
 * inside other RNG's, like arc4random(9).
 */
static int
read_random_phony(void *buf, int count)
{
	u_long randval;
	int size, i;

	if (!warned) {
		log(LOG_WARNING, "random device not loaded; using insecure entropy\n");
		warned = 1;
	}

	/* srandom() is called in kern/init_main.c:proc0_post() */

	/* Fill buf[] with random(9) output */
	for (i = 0; i < count; i+= (int)sizeof(u_long)) {
		randval = random();
		size = MIN(count - i, sizeof(u_long));
		memcpy(&((char *)buf)[i], &randval, (size_t)size);
	}

	return (count);
}

/* Helper routine to enable kproc_exit() to work while the module is
 * being (or has been) unloaded.
 * This routine is in this file because it is always linked into the kernel,
 * and will thus never be unloaded. This is critical for unloadable modules
 * that have threads.
 */
void
random_set_wakeup_exit(void *control)
{
	wakeup(control);
	kproc_exit(0);
	/* NOTREACHED */
}