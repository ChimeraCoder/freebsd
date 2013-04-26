
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <complib/cl_debug.h>
#include <opensm/osm_db_pack.h>

static inline void __osm_pack_guid(uint64_t guid, char *p_guid_str)
{
	sprintf(p_guid_str, "0x%016" PRIx64, guid);
}

static inline uint64_t __osm_unpack_guid(char *p_guid_str)
{
	return strtoull(p_guid_str, NULL, 0);
}

static inline void
__osm_pack_lids(uint16_t min_lid, uint16_t max_lid, char *p_lid_str)
{
	sprintf(p_lid_str, "0x%04x 0x%04x", min_lid, max_lid);
}

static inline int
__osm_unpack_lids(IN char *p_lid_str,
		  OUT uint16_t * p_min_lid, OUT uint16_t * p_max_lid)
{
	unsigned long tmp;
	char *p_next;
	char *p_num;
	char lids_str[24];

	strncpy(lids_str, p_lid_str, 23);
	lids_str[23] = '\0';
	p_num = strtok_r(lids_str, " \t", &p_next);
	if (!p_num)
		return 1;
	tmp = strtoul(p_num, NULL, 0);
	CL_ASSERT(tmp < 0x10000);
	*p_min_lid = (uint16_t) tmp;

	p_num = strtok_r(NULL, " \t", &p_next);
	if (!p_num)
		return 1;
	tmp = strtoul(p_num, NULL, 0);
	CL_ASSERT(tmp < 0x10000);
	*p_max_lid = (uint16_t) tmp;

	return 0;
}

int
osm_db_guid2lid_guids(IN osm_db_domain_t * const p_g2l,
		      OUT cl_qlist_t * p_guid_list)
{
	char *p_key;
	cl_list_t keys;
	osm_db_guid_elem_t *p_guid_elem;

	cl_list_construct(&keys);
	cl_list_init(&keys, 10);

	if (osm_db_keys(p_g2l, &keys))
		return 1;

	while ((p_key = cl_list_remove_head(&keys)) != NULL) {
		p_guid_elem =
		    (osm_db_guid_elem_t *) malloc(sizeof(osm_db_guid_elem_t));
		CL_ASSERT(p_guid_elem != NULL);

		p_guid_elem->guid = __osm_unpack_guid(p_key);
		cl_qlist_insert_head(p_guid_list, &p_guid_elem->item);
	}

	cl_list_destroy(&keys);
	return 0;
}

int
osm_db_guid2lid_get(IN osm_db_domain_t * const p_g2l,
		    IN uint64_t guid,
		    OUT uint16_t * p_min_lid, OUT uint16_t * p_max_lid)
{
	char guid_str[20];
	char *p_lid_str;
	uint16_t min_lid, max_lid;

	__osm_pack_guid(guid, guid_str);
	p_lid_str = osm_db_lookup(p_g2l, guid_str);
	if (!p_lid_str)
		return 1;
	if (__osm_unpack_lids(p_lid_str, &min_lid, &max_lid))
		return 1;

	if (p_min_lid)
		*p_min_lid = min_lid;
	if (p_max_lid)
		*p_max_lid = max_lid;

	return 0;
}

int
osm_db_guid2lid_set(IN osm_db_domain_t * const p_g2l,
		    IN uint64_t guid, IN uint16_t min_lid, IN uint16_t max_lid)
{
	char guid_str[20];
	char lid_str[16];

	__osm_pack_guid(guid, guid_str);
	__osm_pack_lids(min_lid, max_lid, lid_str);

	return (osm_db_update(p_g2l, guid_str, lid_str));
}

int osm_db_guid2lid_delete(IN osm_db_domain_t * const p_g2l, IN uint64_t guid)
{
	char guid_str[20];
	__osm_pack_guid(guid, guid_str);
	return (osm_db_delete(p_g2l, guid_str));
}