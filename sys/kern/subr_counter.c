
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
#include <sys/counter.h>
#include <sys/kernel.h>
#include <sys/smp.h>
#include <sys/sysctl.h>
#include <vm/uma.h>
 
static uma_zone_t uint64_pcpu_zone;

void
counter_u64_zero(counter_u64_t c)
{
	int i;

	for (i = 0; i < mp_ncpus; i++)
		*(uint64_t *)((char *)c + sizeof(struct pcpu) * i) = 0;
}

uint64_t
counter_u64_fetch(counter_u64_t c)
{
	uint64_t r;
	int i;

	r = 0;
	for (i = 0; i < mp_ncpus; i++)
		r += *(uint64_t *)((char *)c + sizeof(struct pcpu) * i);

	return (r);
}

counter_u64_t
counter_u64_alloc(int flags)
{
	counter_u64_t r;

	r = uma_zalloc(uint64_pcpu_zone, flags);
	if (r != NULL)
		counter_u64_zero(r);

	return (r);
}

void
counter_u64_free(counter_u64_t c)
{

	uma_zfree(uint64_pcpu_zone, c);
}

int
sysctl_handle_counter_u64(SYSCTL_HANDLER_ARGS)
{
	uint64_t out;
	int error;

	out = counter_u64_fetch(*(counter_u64_t *)arg1);

	error = SYSCTL_OUT(req, &out, sizeof(uint64_t));

	if (error || !req->newptr)
		return (error);

	/*
	 * Any write attempt to a counter zeroes it.
	 */
	counter_u64_zero(*(counter_u64_t *)arg1);

	return (0);
}

static void
counter_startup(void)
{

	uint64_pcpu_zone = uma_zcreate("uint64 pcpu", sizeof(uint64_t),
	    NULL, NULL, NULL, NULL, UMA_ALIGN_PTR, UMA_ZONE_PCPU);
}
SYSINIT(counter, SI_SUB_CPU, SI_ORDER_FOURTH, counter_startup, NULL);