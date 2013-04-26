
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/nvpair.h>

static void *
nv_alloc_sys(nv_alloc_t *nva, size_t size)
{
	return (kmem_alloc(size, (int)(uintptr_t)nva->nva_arg));
}

/*ARGSUSED*/
static void
nv_free_sys(nv_alloc_t *nva, void *buf, size_t size)
{
	kmem_free(buf, size);
}

static const nv_alloc_ops_t system_ops = {
	NULL,			/* nv_ao_init() */
	NULL,			/* nv_ao_fini() */
	nv_alloc_sys,		/* nv_ao_alloc() */
	nv_free_sys,		/* nv_ao_free() */
	NULL			/* nv_ao_reset() */
};

nv_alloc_t nv_alloc_sleep_def = {
	&system_ops,
	(void *)KM_SLEEP
};

nv_alloc_t nv_alloc_nosleep_def = {
	&system_ops,
	(void *)KM_NOSLEEP
};

nv_alloc_t *nv_alloc_sleep = &nv_alloc_sleep_def;
nv_alloc_t *nv_alloc_nosleep = &nv_alloc_nosleep_def;