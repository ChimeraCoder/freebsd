
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

#include "cryptlib.h"
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include "pcy_int.h"

/* Set policy mapping entries in cache.
 * Note: this modifies the passed POLICY_MAPPINGS structure
 */

int policy_cache_set_mapping(X509 *x, POLICY_MAPPINGS *maps)
	{
	POLICY_MAPPING *map;
	X509_POLICY_DATA *data;
	X509_POLICY_CACHE *cache = x->policy_cache;
	int i;
	int ret = 0;
	if (sk_POLICY_MAPPING_num(maps) == 0)
		{
		ret = -1;
		goto bad_mapping;
		}
	for (i = 0; i < sk_POLICY_MAPPING_num(maps); i++)
		{
		map = sk_POLICY_MAPPING_value(maps, i);
		/* Reject if map to or from anyPolicy */
		if ((OBJ_obj2nid(map->subjectDomainPolicy) == NID_any_policy)
		   || (OBJ_obj2nid(map->issuerDomainPolicy) == NID_any_policy))
			{
			ret = -1;
			goto bad_mapping;
			}

		/* Attempt to find matching policy data */
		data = policy_cache_find_data(cache, map->issuerDomainPolicy);
		/* If we don't have anyPolicy can't map */
		if (!data && !cache->anyPolicy)
			continue;

		/* Create a NODE from anyPolicy */
		if (!data)
			{
			data = policy_data_new(NULL, map->issuerDomainPolicy,
					cache->anyPolicy->flags
						& POLICY_DATA_FLAG_CRITICAL);
			if (!data)
				goto bad_mapping;
			data->qualifier_set = cache->anyPolicy->qualifier_set;
			/*map->issuerDomainPolicy = NULL;*/
			data->flags |= POLICY_DATA_FLAG_MAPPED_ANY;
			data->flags |= POLICY_DATA_FLAG_SHARED_QUALIFIERS;
			if (!sk_X509_POLICY_DATA_push(cache->data, data))
				{
				policy_data_free(data);
				goto bad_mapping;
				}
			}
		else
			data->flags |= POLICY_DATA_FLAG_MAPPED;
		if (!sk_ASN1_OBJECT_push(data->expected_policy_set, 
						map->subjectDomainPolicy))
			goto bad_mapping;
		map->subjectDomainPolicy = NULL;

		}

	ret = 1;
	bad_mapping:
	if (ret == -1)
		x->ex_flags |= EXFLAG_INVALID_POLICY;
	sk_POLICY_MAPPING_pop_free(maps, POLICY_MAPPING_free);
	return ret;

	}