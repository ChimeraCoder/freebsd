
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
 *    Implementation of opensm pkey manipulation functions.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <complib/cl_debug.h>
#include <iba/ib_types.h>
#include <opensm/osm_pkey.h>
#include <opensm/osm_log.h>
#include <opensm/osm_port.h>
#include <opensm/osm_node.h>
#include <opensm/osm_switch.h>
#include <opensm/osm_helper.h>

/**********************************************************************
 **********************************************************************/
void osm_pkey_tbl_construct(IN osm_pkey_tbl_t * p_pkey_tbl)
{
	cl_ptr_vector_construct(&p_pkey_tbl->blocks);
	cl_ptr_vector_construct(&p_pkey_tbl->new_blocks);
	cl_map_construct(&p_pkey_tbl->keys);
}

/**********************************************************************
 **********************************************************************/
void osm_pkey_tbl_destroy(IN osm_pkey_tbl_t * p_pkey_tbl)
{
	ib_pkey_table_t *p_block;
	uint16_t num_blocks, i;

	num_blocks = (uint16_t) (cl_ptr_vector_get_size(&p_pkey_tbl->blocks));
	for (i = 0; i < num_blocks; i++)
		if ((p_block = cl_ptr_vector_get(&p_pkey_tbl->blocks, i)))
			free(p_block);
	cl_ptr_vector_destroy(&p_pkey_tbl->blocks);

	num_blocks =
	    (uint16_t) (cl_ptr_vector_get_size(&p_pkey_tbl->new_blocks));
	for (i = 0; i < num_blocks; i++)
		if ((p_block = cl_ptr_vector_get(&p_pkey_tbl->new_blocks, i)))
			free(p_block);
	cl_ptr_vector_destroy(&p_pkey_tbl->new_blocks);

	cl_map_remove_all(&p_pkey_tbl->keys);
	cl_map_destroy(&p_pkey_tbl->keys);
}

/**********************************************************************
 **********************************************************************/
ib_api_status_t osm_pkey_tbl_init(IN osm_pkey_tbl_t * p_pkey_tbl)
{
	cl_ptr_vector_init(&p_pkey_tbl->blocks, 0, 1);
	cl_ptr_vector_init(&p_pkey_tbl->new_blocks, 0, 1);
	cl_map_init(&p_pkey_tbl->keys, 1);
	cl_qlist_init(&p_pkey_tbl->pending);
	p_pkey_tbl->used_blocks = 0;
	p_pkey_tbl->max_blocks = 0;
	return (IB_SUCCESS);
}

/**********************************************************************
 **********************************************************************/
void osm_pkey_tbl_init_new_blocks(IN const osm_pkey_tbl_t * p_pkey_tbl)
{
	ib_pkey_table_t *p_block;
	size_t b, num_blocks = cl_ptr_vector_get_size(&p_pkey_tbl->new_blocks);

	for (b = 0; b < num_blocks; b++)
		if ((p_block = cl_ptr_vector_get(&p_pkey_tbl->new_blocks, b)))
			memset(p_block, 0, sizeof(*p_block));
}

/**********************************************************************
 **********************************************************************/
void osm_pkey_tbl_cleanup_pending(IN osm_pkey_tbl_t * p_pkey_tbl)
{
	cl_list_item_t *p_item;

	p_item = cl_qlist_remove_head(&p_pkey_tbl->pending);
	while (p_item != cl_qlist_end(&p_pkey_tbl->pending)) {
		free((osm_pending_pkey_t *) p_item);
	}
}

/**********************************************************************
 **********************************************************************/
ib_api_status_t
osm_pkey_tbl_set(IN osm_pkey_tbl_t * p_pkey_tbl,
		 IN uint16_t block, IN ib_pkey_table_t * p_tbl)
{
	uint16_t b, i;
	ib_pkey_table_t *p_pkey_block;
	uint16_t *p_prev_pkey;
	ib_net16_t pkey;

	/* make sure the block is allocated */
	if (cl_ptr_vector_get_size(&p_pkey_tbl->blocks) > block)
		p_pkey_block =
		    (ib_pkey_table_t *) cl_ptr_vector_get(&p_pkey_tbl->blocks,
							  block);
	else
		p_pkey_block = NULL;

	if (!p_pkey_block) {
		p_pkey_block =
		    (ib_pkey_table_t *) malloc(sizeof(ib_pkey_table_t));
		if (!p_pkey_block)
			return (IB_ERROR);
		memset(p_pkey_block, 0, sizeof(ib_pkey_table_t));
		cl_ptr_vector_set(&p_pkey_tbl->blocks, block, p_pkey_block);
	}

	/* sets the block values */
	memcpy(p_pkey_block, p_tbl, sizeof(ib_pkey_table_t));

	/*
	   NOTE: as the spec does not require uniqueness of PKeys in
	   tables there is no other way but to refresh the entire keys map.

	   Moreover, if the same key exists but with full membership it should
	   have precedence on the key with limited membership !
	 */
	cl_map_remove_all(&p_pkey_tbl->keys);

	for (b = 0; b < cl_ptr_vector_get_size(&p_pkey_tbl->blocks); b++) {

		p_pkey_block = cl_ptr_vector_get(&p_pkey_tbl->blocks, b);
		if (!p_pkey_block)
			continue;

		for (i = 0; i < IB_NUM_PKEY_ELEMENTS_IN_BLOCK; i++) {
			pkey = p_pkey_block->pkey_entry[i];
			if (ib_pkey_is_invalid(pkey))
				continue;

			/*
			   ignore the PKey Full Member bit in the key but store
			   the pointer to the table element as the map value
			 */
			p_prev_pkey =
			    cl_map_get(&p_pkey_tbl->keys,
				       ib_pkey_get_base(pkey));

			/* we only insert if no previous or it is not full member */
			if ((p_prev_pkey == NULL) ||
			    (cl_ntoh16(*p_prev_pkey) < cl_ntoh16(pkey)))
				cl_map_insert(&p_pkey_tbl->keys,
					      ib_pkey_get_base(pkey),
					      &(p_pkey_block->pkey_entry[i])
				    );
		}
	}
	return (IB_SUCCESS);
}

/**********************************************************************
 **********************************************************************/
/*
  Store the given pkey in the "new" blocks array.
  Also, make sure the regular block exists.
*/
ib_api_status_t
osm_pkey_tbl_set_new_entry(IN osm_pkey_tbl_t * p_pkey_tbl,
			   IN uint16_t block_idx,
			   IN uint8_t pkey_idx, IN uint16_t pkey)
{
	ib_pkey_table_t *p_block;

	if (!(p_block = osm_pkey_tbl_new_block_get(p_pkey_tbl, block_idx))) {
		p_block = (ib_pkey_table_t *) malloc(sizeof(ib_pkey_table_t));
		if (!p_block)
			return (IB_ERROR);
		memset(p_block, 0, sizeof(ib_pkey_table_t));
		cl_ptr_vector_set(&p_pkey_tbl->new_blocks, block_idx, p_block);
	}

	p_block->pkey_entry[pkey_idx] = pkey;
	if (p_pkey_tbl->used_blocks <= block_idx)
		p_pkey_tbl->used_blocks = block_idx + 1;

	return (IB_SUCCESS);
}

/**********************************************************************
 **********************************************************************/
boolean_t
osm_pkey_find_next_free_entry(IN osm_pkey_tbl_t * p_pkey_tbl,
			      OUT uint16_t * p_block_idx,
			      OUT uint8_t * p_pkey_idx)
{
	ib_pkey_table_t *p_new_block;

	CL_ASSERT(p_block_idx);
	CL_ASSERT(p_pkey_idx);

	while (*p_block_idx < p_pkey_tbl->max_blocks) {
		if (*p_pkey_idx > IB_NUM_PKEY_ELEMENTS_IN_BLOCK - 1) {
			*p_pkey_idx = 0;
			(*p_block_idx)++;
			if (*p_block_idx >= p_pkey_tbl->max_blocks)
				return FALSE;
		}

		p_new_block =
		    osm_pkey_tbl_new_block_get(p_pkey_tbl, *p_block_idx);

		if (!p_new_block ||
		    ib_pkey_is_invalid(p_new_block->pkey_entry[*p_pkey_idx]))
			return TRUE;
		else
			(*p_pkey_idx)++;
	}
	return FALSE;
}

/**********************************************************************
 **********************************************************************/
ib_api_status_t
osm_pkey_tbl_get_block_and_idx(IN osm_pkey_tbl_t * p_pkey_tbl,
			       IN uint16_t * p_pkey,
			       OUT uint16_t * p_block_idx,
			       OUT uint8_t * p_pkey_idx)
{
	uint16_t num_of_blocks;
	uint16_t block_index;
	ib_pkey_table_t *block;

	CL_ASSERT(p_block_idx != NULL);
	CL_ASSERT(p_pkey_idx != NULL);

	num_of_blocks = (uint16_t) cl_ptr_vector_get_size(&p_pkey_tbl->blocks);
	for (block_index = 0; block_index < num_of_blocks; block_index++) {
		block = osm_pkey_tbl_block_get(p_pkey_tbl, block_index);
		if ((block->pkey_entry <= p_pkey) &&
		    (p_pkey <
		     block->pkey_entry + IB_NUM_PKEY_ELEMENTS_IN_BLOCK)) {
			*p_block_idx = block_index;
			*p_pkey_idx = (uint8_t) (p_pkey - block->pkey_entry);
			return (IB_SUCCESS);
		}
	}
	return (IB_NOT_FOUND);
}

/**********************************************************************
 **********************************************************************/
static boolean_t
__osm_match_pkey(IN const ib_net16_t * pkey1, IN const ib_net16_t * pkey2)
{

	/* if both pkeys are not full member - this is not a match */
	if (!(ib_pkey_is_full_member(*pkey1) || ib_pkey_is_full_member(*pkey2)))
		return (FALSE);

	/* compare if the bases are the same. if they are - then
	   this is a match */
	if (ib_pkey_get_base(*pkey1) != ib_pkey_get_base(*pkey2))
		return (FALSE);

	return (TRUE);
}

/**********************************************************************
 **********************************************************************/
boolean_t
osm_physp_share_this_pkey(IN const osm_physp_t * const p_physp1,
			  IN const osm_physp_t * const p_physp2,
			  IN const ib_net16_t pkey)
{
	ib_net16_t *pkey1, *pkey2;

	pkey1 = cl_map_get(&(osm_physp_get_pkey_tbl(p_physp1))->keys,
			   ib_pkey_get_base(pkey));
	pkey2 = cl_map_get(&(osm_physp_get_pkey_tbl(p_physp2))->keys,
			   ib_pkey_get_base(pkey));
	return (pkey1 && pkey2 && __osm_match_pkey(pkey1, pkey2));
}

/**********************************************************************
 **********************************************************************/
ib_net16_t
osm_physp_find_common_pkey(IN const osm_physp_t * const p_physp1,
			   IN const osm_physp_t * const p_physp2)
{
	ib_net16_t *pkey1, *pkey2;
	uint64_t pkey1_base, pkey2_base;
	const osm_pkey_tbl_t *pkey_tbl1, *pkey_tbl2;
	cl_map_iterator_t map_iter1, map_iter2;

	pkey_tbl1 = osm_physp_get_pkey_tbl(p_physp1);
	pkey_tbl2 = osm_physp_get_pkey_tbl(p_physp2);

	map_iter1 = cl_map_head(&pkey_tbl1->keys);
	map_iter2 = cl_map_head(&pkey_tbl2->keys);

	/* we rely on the fact the map are sorted by pkey */
	while ((map_iter1 != cl_map_end(&pkey_tbl1->keys)) &&
	       (map_iter2 != cl_map_end(&pkey_tbl2->keys))) {
		pkey1 = (ib_net16_t *) cl_map_obj(map_iter1);
		pkey2 = (ib_net16_t *) cl_map_obj(map_iter2);

		if (__osm_match_pkey(pkey1, pkey2))
			return *pkey1;

		/* advance the lower value if they are not equal */
		pkey1_base = cl_map_key(map_iter1);
		pkey2_base = cl_map_key(map_iter2);
		if (pkey2_base == pkey1_base) {
			map_iter1 = cl_map_next(map_iter1);
			map_iter2 = cl_map_next(map_iter2);
		} else if (pkey2_base < pkey1_base)
			map_iter2 = cl_map_next(map_iter2);
		else
			map_iter1 = cl_map_next(map_iter1);
	}

	return 0;
}

/**********************************************************************
 **********************************************************************/
boolean_t
osm_physp_share_pkey(IN osm_log_t * p_log,
		     IN const osm_physp_t * const p_physp_1,
		     IN const osm_physp_t * const p_physp_2)
{
	const osm_pkey_tbl_t *pkey_tbl1, *pkey_tbl2;

	if (p_physp_1 == p_physp_2)
		return TRUE;

	pkey_tbl1 = osm_physp_get_pkey_tbl(p_physp_1);
	pkey_tbl2 = osm_physp_get_pkey_tbl(p_physp_2);

	/*
	   The spec: 10.9.2 does not require each phys port to have PKey Table.
	   So actually if it does not, we need to use the default port instead.

	   HACK: meanwhile we will ignore the check
	 */
	if (cl_is_map_empty(&pkey_tbl1->keys)
	    || cl_is_map_empty(&pkey_tbl2->keys))
		return TRUE;

	return
	    !ib_pkey_is_invalid(osm_physp_find_common_pkey
				(p_physp_1, p_physp_2));
}

/**********************************************************************
 **********************************************************************/
boolean_t
osm_port_share_pkey(IN osm_log_t * p_log,
		    IN const osm_port_t * const p_port_1,
		    IN const osm_port_t * const p_port_2)
{

	osm_physp_t *p_physp1, *p_physp2;
	boolean_t ret;

	OSM_LOG_ENTER(p_log);

	if (!p_port_1 || !p_port_2) {
		ret = FALSE;
		goto Exit;
	}

	p_physp1 = p_port_1->p_physp;
	p_physp2 = p_port_2->p_physp;

	if (!p_physp1 || !p_physp2) {
		ret = FALSE;
		goto Exit;
	}

	ret = osm_physp_share_pkey(p_log, p_physp1, p_physp2);

Exit:
	OSM_LOG_EXIT(p_log);
	return ret;
}

/**********************************************************************
 **********************************************************************/
boolean_t
osm_physp_has_pkey(IN osm_log_t * p_log,
		   IN const ib_net16_t pkey,
		   IN const osm_physp_t * const p_physp)
{

	ib_net16_t *p_pkey, pkey_base;
	const osm_pkey_tbl_t *pkey_tbl;
	boolean_t res = FALSE;

	OSM_LOG_ENTER(p_log);

	OSM_LOG(p_log, OSM_LOG_DEBUG,
		"Search for PKey: 0x%04x\n", cl_ntoh16(pkey));

	/* if the pkey given is an invalid pkey - return TRUE. */
	if (ib_pkey_is_invalid(pkey)) {
		OSM_LOG(p_log, OSM_LOG_DEBUG,
			"Given invalid PKey - we treat it loosely and allow it\n");
		res = TRUE;
		goto Exit;
	}

	pkey_base = ib_pkey_get_base(pkey);

	pkey_tbl = osm_physp_get_pkey_tbl(p_physp);

	p_pkey = cl_map_get(&pkey_tbl->keys, pkey_base);
	if (p_pkey) {
		res = TRUE;
		OSM_LOG(p_log, OSM_LOG_DEBUG,
			"PKey 0x%04x was found\n", cl_ntoh16(pkey));
	} else {
		OSM_LOG(p_log, OSM_LOG_DEBUG,
			"PKey 0x%04x was not found\n", cl_ntoh16(pkey));
	}

Exit:
	OSM_LOG_EXIT(p_log);
	return res;
}