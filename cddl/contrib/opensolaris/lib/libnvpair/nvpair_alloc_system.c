
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
#include <stdlib.h>

/*ARGSUSED*/
static void *
nv_alloc_sys(nv_alloc_t *nva, size_t size)
{
	return (malloc(size));
}

/*ARGSUSED*/
static void
nv_free_sys(nv_alloc_t *nva, void *buf, size_t size)
{
	free(buf);
}

const nv_alloc_ops_t system_ops_def = {
	NULL,			/* nv_ao_init() */
	NULL,			/* nv_ao_fini() */
	nv_alloc_sys,		/* nv_ao_alloc() */
	nv_free_sys,		/* nv_ao_free() */
	NULL			/* nv_ao_reset() */
};

nv_alloc_t nv_alloc_nosleep_def = {
	&system_ops_def,
	NULL
};

nv_alloc_t *nv_alloc_nosleep = &nv_alloc_nosleep_def;