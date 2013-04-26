
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

/* accessor functions */

/* X509_POLICY_TREE stuff */

int X509_policy_tree_level_count(const X509_POLICY_TREE *tree)
	{
	if (!tree)
		return 0;
	return tree->nlevel;
	}

X509_POLICY_LEVEL *
	X509_policy_tree_get0_level(const X509_POLICY_TREE *tree, int i)
	{
	if (!tree || (i < 0) || (i >= tree->nlevel))
		return NULL;
	return tree->levels + i;
	}

STACK_OF(X509_POLICY_NODE) *
		X509_policy_tree_get0_policies(const X509_POLICY_TREE *tree)
	{
	if (!tree)
		return NULL;
	return tree->auth_policies;
	}

STACK_OF(X509_POLICY_NODE) *
	X509_policy_tree_get0_user_policies(const X509_POLICY_TREE *tree)
	{
	if (!tree)
		return NULL;
	if (tree->flags & POLICY_FLAG_ANY_POLICY)
		return tree->auth_policies;
	else
		return tree->user_policies;
	}

/* X509_POLICY_LEVEL stuff */

int X509_policy_level_node_count(X509_POLICY_LEVEL *level)
	{
	int n;
	if (!level)
		return 0;
	if (level->anyPolicy)
		n = 1;
	else
		n = 0;
	if (level->nodes)
		n += sk_X509_POLICY_NODE_num(level->nodes);
	return n;
	}

X509_POLICY_NODE *X509_policy_level_get0_node(X509_POLICY_LEVEL *level, int i)
	{
	if (!level)
		return NULL;
	if (level->anyPolicy)
		{
		if (i == 0)
			return level->anyPolicy;
		i--;
		}
	return sk_X509_POLICY_NODE_value(level->nodes, i);
	}

/* X509_POLICY_NODE stuff */

const ASN1_OBJECT *X509_policy_node_get0_policy(const X509_POLICY_NODE *node)
	{
	if (!node)
		return NULL;
	return node->data->valid_policy;
	}

#if 0
int X509_policy_node_get_critical(const X509_POLICY_NODE *node)
	{
	if (node_critical(node))
		return 1;
	return 0;
	}
#endif

STACK_OF(POLICYQUALINFO) *
		X509_policy_node_get0_qualifiers(const X509_POLICY_NODE *node)
	{
	if (!node)
		return NULL;
	return node->data->qualifier_set;
	}

const X509_POLICY_NODE *
		X509_policy_node_get0_parent(const X509_POLICY_NODE *node)
	{
	if (!node)
		return NULL;
	return node->parent;
	}