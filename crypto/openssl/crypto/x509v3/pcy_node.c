
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

#include <openssl/asn1.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include "pcy_int.h"

static int node_cmp(const X509_POLICY_NODE * const *a,
			const X509_POLICY_NODE * const *b)
	{
	return OBJ_cmp((*a)->data->valid_policy, (*b)->data->valid_policy);
	}

STACK_OF(X509_POLICY_NODE) *policy_node_cmp_new(void)
	{
	return sk_X509_POLICY_NODE_new(node_cmp);
	}

X509_POLICY_NODE *tree_find_sk(STACK_OF(X509_POLICY_NODE) *nodes,
					const ASN1_OBJECT *id)
	{
	X509_POLICY_DATA n;
	X509_POLICY_NODE l;
	int idx;

	n.valid_policy = (ASN1_OBJECT *)id;
	l.data = &n;

	idx = sk_X509_POLICY_NODE_find(nodes, &l);
	if (idx == -1)
		return NULL;

	return sk_X509_POLICY_NODE_value(nodes, idx);

	}

X509_POLICY_NODE *level_find_node(const X509_POLICY_LEVEL *level,
					const X509_POLICY_NODE *parent,	
					const ASN1_OBJECT *id)
	{
	X509_POLICY_NODE *node;
	int i;
	for (i = 0; i < sk_X509_POLICY_NODE_num(level->nodes); i++)
		{
		node = sk_X509_POLICY_NODE_value(level->nodes, i);
		if (node->parent == parent)
			{
			if (!OBJ_cmp(node->data->valid_policy, id))
				return node;
			}
		}
	return NULL;
	}

X509_POLICY_NODE *level_add_node(X509_POLICY_LEVEL *level,
			const X509_POLICY_DATA *data,
			X509_POLICY_NODE *parent,
			X509_POLICY_TREE *tree)
	{
	X509_POLICY_NODE *node;
	node = OPENSSL_malloc(sizeof(X509_POLICY_NODE));
	if (!node)
		return NULL;
	node->data = data;
	node->parent = parent;
	node->nchild = 0;
	if (level)
		{
		if (OBJ_obj2nid(data->valid_policy) == NID_any_policy)
			{
			if (level->anyPolicy)
				goto node_error;
			level->anyPolicy = node;
			}
		else
			{

			if (!level->nodes)
				level->nodes = policy_node_cmp_new();
			if (!level->nodes)
				goto node_error;
			if (!sk_X509_POLICY_NODE_push(level->nodes, node))
				goto node_error;
			}
		}

	if (tree)
		{
		if (!tree->extra_data)
			 tree->extra_data = sk_X509_POLICY_DATA_new_null();
		if (!tree->extra_data)
			goto node_error;
		if (!sk_X509_POLICY_DATA_push(tree->extra_data, data))
			goto node_error;
		}

	if (parent)
		parent->nchild++;

	return node;

	node_error:
	policy_node_free(node);
	return 0;

	}

void policy_node_free(X509_POLICY_NODE *node)
	{
	OPENSSL_free(node);
	}

/* See if a policy node matches a policy OID. If mapping enabled look through
 * expected policy set otherwise just valid policy.
 */

int policy_node_match(const X509_POLICY_LEVEL *lvl,
		      const X509_POLICY_NODE *node, const ASN1_OBJECT *oid)
	{
	int i;
	ASN1_OBJECT *policy_oid;
	const X509_POLICY_DATA *x = node->data;

	if (	    (lvl->flags & X509_V_FLAG_INHIBIT_MAP)
		|| !(x->flags & POLICY_DATA_FLAG_MAP_MASK))
		{
		if (!OBJ_cmp(x->valid_policy, oid))
			return 1;
		return 0;
		}

	for (i = 0; i < sk_ASN1_OBJECT_num(x->expected_policy_set); i++)
		{
		policy_oid = sk_ASN1_OBJECT_value(x->expected_policy_set, i);
		if (!OBJ_cmp(policy_oid, oid))
			return 1;
		}
	return 0;

	}