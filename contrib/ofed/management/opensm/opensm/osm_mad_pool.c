
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
 * Abstract:
 *    Implementation of osm_mad_pool_t.
 * This object represents a pool of management datagram (MAD) objects.
 * This object is part of the opensm family of objects.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <opensm/osm_mad_pool.h>
#include <opensm/osm_madw.h>
#include <vendor/osm_vendor_api.h>

/**********************************************************************
 **********************************************************************/
void osm_mad_pool_construct(IN osm_mad_pool_t * const p_pool)
{
	CL_ASSERT(p_pool);

	memset(p_pool, 0, sizeof(*p_pool));
}

/**********************************************************************
 **********************************************************************/
void osm_mad_pool_destroy(IN osm_mad_pool_t * const p_pool)
{
	CL_ASSERT(p_pool);
}

/**********************************************************************
 **********************************************************************/
ib_api_status_t osm_mad_pool_init(IN osm_mad_pool_t * const p_pool)
{
	p_pool->mads_out = 0;

	return IB_SUCCESS;
}

/**********************************************************************
 **********************************************************************/
osm_madw_t *osm_mad_pool_get(IN osm_mad_pool_t * const p_pool,
			     IN osm_bind_handle_t h_bind,
			     IN const uint32_t total_size,
			     IN const osm_mad_addr_t * const p_mad_addr)
{
	osm_madw_t *p_madw;
	ib_mad_t *p_mad;

	CL_ASSERT(h_bind != OSM_BIND_INVALID_HANDLE);
	CL_ASSERT(total_size);

	/*
	   First, acquire a mad wrapper from the mad wrapper pool.
	 */
	p_madw = malloc(sizeof(*p_madw));
	if (p_madw == NULL)
		goto Exit;

	osm_madw_init(p_madw, h_bind, total_size, p_mad_addr);

	/*
	   Next, acquire a wire mad of the specified size.
	 */
	p_mad = osm_vendor_get(h_bind, total_size, &p_madw->vend_wrap);
	if (p_mad == NULL) {
		/* Don't leak wrappers! */
		free(p_madw);
		p_madw = NULL;
		goto Exit;
	}

	cl_atomic_inc(&p_pool->mads_out);
	/*
	   Finally, attach the wire MAD to this wrapper.
	 */
	osm_madw_set_mad(p_madw, p_mad);

Exit:
	return p_madw;
}

/**********************************************************************
 **********************************************************************/
osm_madw_t *osm_mad_pool_get_wrapper(IN osm_mad_pool_t * const p_pool,
				     IN osm_bind_handle_t h_bind,
				     IN const uint32_t total_size,
				     IN const ib_mad_t * const p_mad,
				     IN const osm_mad_addr_t * const p_mad_addr)
{
	osm_madw_t *p_madw;

	CL_ASSERT(h_bind != OSM_BIND_INVALID_HANDLE);
	CL_ASSERT(total_size);
	CL_ASSERT(p_mad);

	/*
	   First, acquire a mad wrapper from the mad wrapper pool.
	 */
	p_madw = malloc(sizeof(*p_madw));
	if (p_madw == NULL)
		goto Exit;

	/*
	   Finally, initialize the wrapper object.
	 */
	cl_atomic_inc(&p_pool->mads_out);
	osm_madw_init(p_madw, h_bind, total_size, p_mad_addr);
	osm_madw_set_mad(p_madw, p_mad);

Exit:
	return (p_madw);
}

/**********************************************************************
 **********************************************************************/
osm_madw_t *osm_mad_pool_get_wrapper_raw(IN osm_mad_pool_t * const p_pool)
{
	osm_madw_t *p_madw;

	p_madw = malloc(sizeof(*p_madw));
	if (!p_madw)
		return NULL;

	osm_madw_init(p_madw, 0, 0, 0);
	osm_madw_set_mad(p_madw, 0);
	cl_atomic_inc(&p_pool->mads_out);

	return (p_madw);
}

/**********************************************************************
 **********************************************************************/
void
osm_mad_pool_put(IN osm_mad_pool_t * const p_pool, IN osm_madw_t * const p_madw)
{
	CL_ASSERT(p_madw);

	/*
	   First, return the wire mad to the pool
	 */
	if (p_madw->p_mad)
		osm_vendor_put(p_madw->h_bind, &p_madw->vend_wrap);

	/*
	   Return the mad wrapper to the wrapper pool
	 */
	free(p_madw);
	cl_atomic_dec(&p_pool->mads_out);
}