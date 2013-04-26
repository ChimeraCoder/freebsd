
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

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <vm/uma.h>

#include <machine/octeon_cop2.h>

static uma_zone_t ctxzone;

static void 
octeon_cop2_init(void* dummy)
{
	printf("Create COP2 context zone\n");
	ctxzone = uma_zcreate("COP2 context",
	                        sizeof(struct octeon_cop2_state), 
				NULL, NULL, NULL, NULL, 8, 0);
}

struct octeon_cop2_state *
octeon_cop2_alloc_ctx()
{
	return uma_zalloc(ctxzone, M_NOWAIT);
}

void
octeon_cop2_free_ctx(struct octeon_cop2_state *ctx)
{
	uma_zfree(ctxzone, ctx);
}

SYSINIT(octeon_cop2, SI_SUB_CPU, SI_ORDER_FIRST, octeon_cop2_init, NULL);