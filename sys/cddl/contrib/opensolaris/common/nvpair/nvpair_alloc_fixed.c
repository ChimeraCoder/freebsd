
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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/nvpair.h>
#include <sys/sysmacros.h>
#if defined(_KERNEL) && !defined(_BOOT)
#include <sys/varargs.h>
#else
#include <stdarg.h>
#include <strings.h>
#endif

/*
 * This allocator is very simple.
 *  - it uses a pre-allocated buffer for memory allocations.
 *  - it does _not_ free memory in the pre-allocated buffer.
 *
 * The reason for the selected implemention is simplicity.
 * This allocator is designed for the usage in interrupt context when
 * the caller may not wait for free memory.
 */

/* pre-allocated buffer for memory allocations */
typedef struct nvbuf {
	uintptr_t	nvb_buf;	/* address of pre-allocated buffer */
	uintptr_t 	nvb_lim;	/* limit address in the buffer */
	uintptr_t	nvb_cur;	/* current address in the buffer */
} nvbuf_t;

/*
 * Initialize the pre-allocated buffer allocator. The caller needs to supply
 *
 *   buf	address of pre-allocated buffer
 *   bufsz	size of pre-allocated buffer
 *
 * nv_fixed_init() calculates the remaining members of nvbuf_t.
 */
static int
nv_fixed_init(nv_alloc_t *nva, va_list valist)
{
	uintptr_t base = va_arg(valist, uintptr_t);
	uintptr_t lim = base + va_arg(valist, size_t);
	nvbuf_t *nvb = (nvbuf_t *)P2ROUNDUP(base, sizeof (uintptr_t));

	if (base == 0 || (uintptr_t)&nvb[1] > lim)
		return (EINVAL);

	nvb->nvb_buf = (uintptr_t)&nvb[0];
	nvb->nvb_cur = (uintptr_t)&nvb[1];
	nvb->nvb_lim = lim;
	nva->nva_arg = nvb;

	return (0);
}

static void *
nv_fixed_alloc(nv_alloc_t *nva, size_t size)
{
	nvbuf_t *nvb = nva->nva_arg;
	uintptr_t new = nvb->nvb_cur;

	if (size == 0 || new + size > nvb->nvb_lim)
		return (NULL);

	nvb->nvb_cur = P2ROUNDUP(new + size, sizeof (uintptr_t));

	return ((void *)new);
}

/*ARGSUSED*/
static void
nv_fixed_free(nv_alloc_t *nva, void *buf, size_t size)
{
	/* don't free memory in the pre-allocated buffer */
}

static void
nv_fixed_reset(nv_alloc_t *nva)
{
	nvbuf_t *nvb = nva->nva_arg;

	nvb->nvb_cur = (uintptr_t)&nvb[1];
}

const nv_alloc_ops_t nv_fixed_ops_def = {
	nv_fixed_init,	/* nv_ao_init() */
	NULL,		/* nv_ao_fini() */
	nv_fixed_alloc,	/* nv_ao_alloc() */
	nv_fixed_free,	/* nv_ao_free() */
	nv_fixed_reset	/* nv_ao_reset() */
};

const nv_alloc_ops_t *nv_fixed_ops = &nv_fixed_ops_def;