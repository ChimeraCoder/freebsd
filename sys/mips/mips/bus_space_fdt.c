
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

/*
 * Copyright (c) 1996 Charles M. Hannum.  All rights reserved.
 * Copyright (c) 1996 Christopher G. Demetriou.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou
 *	for the NetBSD Project.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	from: src/sys/alpha/include/bus.h,v 1.5 1999/08/28 00:38:40 peter
 * $FreeBSD$
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/ktr.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_kern.h>
#include <vm/vm_extern.h>

#include <machine/bus.h>
#include <machine/cache.h>

static int	fdt_bs_map(void *, bus_addr_t, bus_size_t, int,
		    bus_space_handle_t *);

static struct bus_space fdt_space = {
	/* cookie */
	(void *) 0,

	/* mapping/unmapping */
	fdt_bs_map,
	generic_bs_unmap,
	generic_bs_subregion,

	/* allocation/deallocation */
	generic_bs_alloc,
	generic_bs_free,

	/* barrier */
	generic_bs_barrier,

	/* read (single) */
	generic_bs_r_1,
	generic_bs_r_2,
	generic_bs_r_4,
	generic_bs_r_8,

	/* read multiple */
	generic_bs_rm_1,
	generic_bs_rm_2,
	generic_bs_rm_4,
	generic_bs_rm_8,

	/* read region */
	generic_bs_rr_1,
	generic_bs_rr_2,
	generic_bs_rr_4,
	generic_bs_rr_8,

	/* write (single) */
	generic_bs_w_1,
	generic_bs_w_2,
	generic_bs_w_4,
	generic_bs_w_8,

	/* write multiple */
	generic_bs_wm_1,
	generic_bs_wm_2,
	generic_bs_wm_4,
	generic_bs_wm_8,

	/* write region */
	generic_bs_wr_1,
	generic_bs_wr_2,
	generic_bs_wr_4,
	generic_bs_wr_8,

	/* set multiple */
	generic_bs_sm_1,
	generic_bs_sm_2,
	generic_bs_sm_4,
	generic_bs_sm_8,

	/* set region */
	generic_bs_sr_1,
	generic_bs_sr_2,
	generic_bs_sr_4,
	generic_bs_sr_8,

	/* copy */
	generic_bs_c_1,
	generic_bs_c_2,
	generic_bs_c_4,
	generic_bs_c_8,

	/* read (single) stream */
	generic_bs_r_1,
	generic_bs_r_2,
	generic_bs_r_4,
	generic_bs_r_8,

	/* read multiple stream */
	generic_bs_rm_1,
	generic_bs_rm_2,
	generic_bs_rm_4,
	generic_bs_rm_8,

	/* read region stream */
	generic_bs_rr_1,
	generic_bs_rr_2,
	generic_bs_rr_4,
	generic_bs_rr_8,

	/* write (single) stream */
	generic_bs_w_1,
	generic_bs_w_2,
	generic_bs_w_4,
	generic_bs_w_8,

	/* write multiple stream */
	generic_bs_wm_1,
	generic_bs_wm_2,
	generic_bs_wm_4,
	generic_bs_wm_8,

	/* write region stream */
	generic_bs_wr_1,
	generic_bs_wr_2,
	generic_bs_wr_4,
	generic_bs_wr_8,
};

/* generic bus_space tag */
bus_space_tag_t	mips_bus_space_fdt = &fdt_space;

static int
fdt_bs_map(void *t __unused, bus_addr_t addr, bus_size_t size __unused,
    int flags __unused, bus_space_handle_t *bshp)
{

	*bshp = MIPS_PHYS_TO_DIRECT_UNCACHED(addr);
	return (0);
}