
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
 *    Implementation of service record functions.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <complib/cl_debug.h>
#include <complib/cl_timer.h>
#include <opensm/osm_service.h>

/**********************************************************************
 **********************************************************************/
void osm_svcr_delete(IN osm_svcr_t * const p_svcr)
{
	free(p_svcr);
}

/**********************************************************************
 **********************************************************************/
void
osm_svcr_init(IN osm_svcr_t * const p_svcr,
	      IN const ib_service_record_t * p_svc_rec)
{
	CL_ASSERT(p_svcr);

	p_svcr->modified_time = cl_get_time_stamp_sec();

	/* We track the time left for this service in
	   an external field to avoid extra cl_ntoh/hton
	   required for working with the MAD field */
	p_svcr->lease_period = cl_ntoh32(p_svc_rec->service_lease);
	p_svcr->service_record = *p_svc_rec;
}

/**********************************************************************
 **********************************************************************/
osm_svcr_t *osm_svcr_new(IN const ib_service_record_t * p_svc_rec)
{
	osm_svcr_t *p_svcr;

	CL_ASSERT(p_svc_rec);

	p_svcr = (osm_svcr_t *) malloc(sizeof(*p_svcr));
	if (p_svcr) {
		memset(p_svcr, 0, sizeof(*p_svcr));
		osm_svcr_init(p_svcr, p_svc_rec);
	}

	return (p_svcr);
}

/**********************************************************************
 **********************************************************************/
static
    cl_status_t
__match_rid_of_svc_rec(IN const cl_list_item_t * const p_list_item,
		       IN void *context)
{
	ib_service_record_t *p_svc_rec = (ib_service_record_t *) context;
	osm_svcr_t *p_svcr = (osm_svcr_t *) p_list_item;
	int32_t count;

	count = memcmp(&p_svcr->service_record,
		       p_svc_rec,
		       sizeof(p_svc_rec->service_id) +
		       sizeof(p_svc_rec->service_gid) +
		       sizeof(p_svc_rec->service_pkey));

	if (count == 0)
		return CL_SUCCESS;
	else
		return CL_NOT_FOUND;

}

/**********************************************************************
 **********************************************************************/
osm_svcr_t *osm_svcr_get_by_rid(IN osm_subn_t const *p_subn,
				IN osm_log_t * p_log,
				IN ib_service_record_t * const p_svc_rec)
{
	cl_list_item_t *p_list_item;

	OSM_LOG_ENTER(p_log);

	p_list_item = cl_qlist_find_from_head(&p_subn->sa_sr_list,
					      __match_rid_of_svc_rec,
					      p_svc_rec);

	if (p_list_item == cl_qlist_end(&p_subn->sa_sr_list))
		p_list_item = NULL;

	OSM_LOG_EXIT(p_log);
	return (osm_svcr_t *) p_list_item;
}

/**********************************************************************
 **********************************************************************/
void
osm_svcr_insert_to_db(IN osm_subn_t * p_subn,
		      IN osm_log_t * p_log, IN osm_svcr_t * p_svcr)
{
	OSM_LOG_ENTER(p_log);

	OSM_LOG(p_log, OSM_LOG_DEBUG,
		"Inserting new Service Record into Database\n");

	cl_qlist_insert_head(&p_subn->sa_sr_list, &p_svcr->list_item);

	OSM_LOG_EXIT(p_log);
}

void
osm_svcr_remove_from_db(IN osm_subn_t * p_subn,
			IN osm_log_t * p_log, IN osm_svcr_t * p_svcr)
{
	OSM_LOG_ENTER(p_log);

	OSM_LOG(p_log, OSM_LOG_DEBUG,
		"Removing Service Record Name:%s ID:0x%016" PRIx64
		" from Database\n", p_svcr->service_record.service_name,
		p_svcr->service_record.service_id);

	cl_qlist_remove_item(&p_subn->sa_sr_list, &p_svcr->list_item);

	OSM_LOG_EXIT(p_log);
}