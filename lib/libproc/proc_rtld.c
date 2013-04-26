
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rtld_db.h>
#include "libproc.h"
#include "_libproc.h"

static int
map_iter(const rd_loadobj_t *lop, void *arg)
{
	struct proc_handle *phdl = arg;

	if (phdl->nobjs >= phdl->rdobjsz) {
		phdl->rdobjsz *= 2;
		phdl->rdobjs = reallocf(phdl->rdobjs, sizeof(*phdl->rdobjs) *
		    phdl->rdobjsz);
		if (phdl->rdobjs == NULL)
			return (-1);
	}
	memcpy(&phdl->rdobjs[phdl->nobjs++], lop, sizeof(*lop));

	return (0);
}

rd_agent_t *
proc_rdagent(struct proc_handle *phdl)
{
	if (phdl->rdap == NULL && phdl->status != PS_UNDEAD &&
	    phdl->status != PS_IDLE) {
		if ((phdl->rdap = rd_new(phdl)) != NULL) {
			phdl->rdobjs = malloc(sizeof(*phdl->rdobjs) * 64);
			phdl->rdobjsz = 64;
			if (phdl->rdobjs == NULL)
				return (phdl->rdap);
			rd_loadobj_iter(phdl->rdap, map_iter, phdl);
		}
	}

	return (phdl->rdap);
}

void
proc_updatesyms(struct proc_handle *phdl)
{

	memset(phdl->rdobjs, 0, sizeof(*phdl->rdobjs) * phdl->rdobjsz);
	phdl->nobjs = 0;
	rd_loadobj_iter(phdl->rdap, map_iter, phdl);
}