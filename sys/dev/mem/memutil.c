
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
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/memrange.h>
#include <sys/rwlock.h>
#include <sys/systm.h>

static struct rwlock	mr_lock;

/*
 * Implementation-neutral, kernel-callable functions for manipulating
 * memory range attributes.
 */
void
mem_range_init(void)
{

	if (mem_range_softc.mr_op == NULL)
		return;
	rw_init(&mr_lock, "memrange");
	mem_range_softc.mr_op->init(&mem_range_softc);
}

void
mem_range_destroy(void)
{

	if (mem_range_softc.mr_op == NULL)
		return;
	rw_destroy(&mr_lock);
}

int
mem_range_attr_get(struct mem_range_desc *mrd, int *arg)
{
	int nd;

	if (mem_range_softc.mr_op == NULL)
		return (EOPNOTSUPP);
	nd = *arg;
	rw_rlock(&mr_lock);
	if (nd == 0)
		*arg = mem_range_softc.mr_ndesc;
	else
		bcopy(mem_range_softc.mr_desc, mrd, nd * sizeof(*mrd));
	rw_runlock(&mr_lock);
	return (0);
}

int
mem_range_attr_set(struct mem_range_desc *mrd, int *arg)
{
	int ret;

	if (mem_range_softc.mr_op == NULL)
		return (EOPNOTSUPP);
	rw_wlock(&mr_lock);
	ret = mem_range_softc.mr_op->set(&mem_range_softc, mrd, arg);
	rw_wunlock(&mr_lock);
	return (ret);
}