
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

/**
 * \file
 *
 * This file contains structures combining types and functions to
 * manipulate those structures that help building DNS lookup trees.
 */
#include "config.h"
#include "util/storage/dnstree.h"
#include "util/data/dname.h"
#include "util/net_help.h"

int name_tree_compare(const void* k1, const void* k2)
{
        struct name_tree_node* x = (struct name_tree_node*)k1;
        struct name_tree_node* y = (struct name_tree_node*)k2;
        int m;
        if(x->dclass != y->dclass) {
                if(x->dclass < y->dclass)
                        return -1;
                return 1;
        }
        return dname_lab_cmp(x->name, x->labs, y->name, y->labs, &m);
}

int addr_tree_compare(const void* k1, const void* k2)
{
        struct addr_tree_node* n1 = (struct addr_tree_node*)k1;
        struct addr_tree_node* n2 = (struct addr_tree_node*)k2;
        int r = sockaddr_cmp_addr(&n1->addr, n1->addrlen, &n2->addr,
                n2->addrlen);
        if(r != 0) return r;
        if(n1->net < n2->net)
                return -1;
        if(n1->net > n2->net)
                return 1;
        return 0;
}

void name_tree_init(rbtree_t* tree)
{
	rbtree_init(tree, &name_tree_compare);
}

void addr_tree_init(rbtree_t* tree)
{
	rbtree_init(tree, &addr_tree_compare);
}

int name_tree_insert(rbtree_t* tree, struct name_tree_node* node, 
        uint8_t* name, size_t len, int labs, uint16_t dclass)
{
	node->node.key = node;
	node->name = name;
	node->len = len;
	node->labs = labs;
	node->dclass = dclass;
	node->parent = NULL;
	return rbtree_insert(tree, &node->node) != NULL;
}

int addr_tree_insert(rbtree_t* tree, struct addr_tree_node* node,
        struct sockaddr_storage* addr, socklen_t addrlen, int net)
{
	node->node.key = node;
	memcpy(&node->addr, addr, addrlen);
	node->addrlen = addrlen;
	node->net = net;
	node->parent = NULL;
	return rbtree_insert(tree, &node->node) != NULL;
}

void addr_tree_init_parents(rbtree_t* tree)
{
        struct addr_tree_node* node, *prev = NULL, *p;
        int m;
        RBTREE_FOR(node, struct addr_tree_node*, tree) {
                node->parent = NULL;
                if(!prev || prev->addrlen != node->addrlen) {
                        prev = node;
                        continue;
                }
                m = addr_in_common(&prev->addr, prev->net, &node->addr,
                        node->net, node->addrlen);
                /* sort order like: ::/0, 1::/2, 1::/4, ... 2::/2 */
                /* find the previous, or parent-parent-parent */
                for(p = prev; p; p = p->parent)
                        if(p->net <= m) {
                                /* ==: since prev matched m, this is closest*/
                                /* <: prev matches more, but is not a parent,
				 * this one is a (grand)parent */
                                node->parent = p;
                                break;
                        }
                prev = node;
        }
}

void name_tree_init_parents(rbtree_t* tree)
{
        struct name_tree_node* node, *prev = NULL, *p;
        int m;
        RBTREE_FOR(node, struct name_tree_node*, tree) {
                node->parent = NULL;
                if(!prev || prev->dclass != node->dclass) {
                        prev = node;
                        continue;
                }
                (void)dname_lab_cmp(prev->name, prev->labs, node->name,
                        node->labs, &m); /* we know prev is smaller */
		/* sort order like: . com. bla.com. zwb.com. net. */
                /* find the previous, or parent-parent-parent */
                for(p = prev; p; p = p->parent)
                        if(p->labs <= m) {
                                /* ==: since prev matched m, this is closest*/
                                /* <: prev matches more, but is not a parent,
				 * this one is a (grand)parent */
                                node->parent = p;
                                break;
                        }
                prev = node;
        }
}

struct name_tree_node* name_tree_find(rbtree_t* tree, uint8_t* name, 
        size_t len, int labs, uint16_t dclass)
{
	struct name_tree_node key;
	key.node.key = &key;
	key.name = name;
	key.len = len;
	key.labs = labs;
	key.dclass = dclass;
	return (struct name_tree_node*)rbtree_search(tree, &key);
}

struct name_tree_node* name_tree_lookup(rbtree_t* tree, uint8_t* name,
        size_t len, int labs, uint16_t dclass)
{
        rbnode_t* res = NULL;
        struct name_tree_node *result;
        struct name_tree_node key;
        key.node.key = &key;
        key.name = name;
        key.len = len;
        key.labs = labs;
        key.dclass = dclass;
        if(rbtree_find_less_equal(tree, &key, &res)) {
                /* exact */
                result = (struct name_tree_node*)res;
        } else {
                /* smaller element (or no element) */
                int m;
                result = (struct name_tree_node*)res;
                if(!result || result->dclass != dclass)
                        return NULL;
                /* count number of labels matched */
                (void)dname_lab_cmp(result->name, result->labs, key.name,
                        key.labs, &m);
                while(result) { /* go up until qname is subdomain of stub */
                        if(result->labs <= m)
                                break;
                        result = result->parent;
                }
        }
	return result;
}

struct addr_tree_node* addr_tree_lookup(rbtree_t* tree, 
        struct sockaddr_storage* addr, socklen_t addrlen)
{
        rbnode_t* res = NULL;
        struct addr_tree_node* result;
        struct addr_tree_node key;
        key.node.key = &key;
        memcpy(&key.addr, addr, addrlen);
        key.addrlen = addrlen;
        key.net = (addr_is_ip6(addr, addrlen)?128:32);
        if(rbtree_find_less_equal(tree, &key, &res)) {
                /* exact */
                return (struct addr_tree_node*)res;
        } else {
                /* smaller element (or no element) */
                int m;
                result = (struct addr_tree_node*)res;
                if(!result || result->addrlen != addrlen)
                        return 0;
                /* count number of bits matched */
                m = addr_in_common(&result->addr, result->net, addr,
                        key.net, addrlen);
                while(result) { /* go up until addr is inside netblock */
                        if(result->net <= m)
                                break;
                        result = result->parent;
                }
        }
        return result;
}

int
name_tree_next_root(rbtree_t* tree, uint16_t* dclass)
{
	struct name_tree_node key;
	rbnode_t* n;
	struct name_tree_node* p;
	if(*dclass == 0) {
		/* first root item is first item in tree */
		n = rbtree_first(tree);
		if(n == RBTREE_NULL)
			return 0;
		p = (struct name_tree_node*)n;
		if(dname_is_root(p->name)) {
			*dclass = p->dclass;
			return 1;
		}
		/* root not first item? search for higher items */
		*dclass = p->dclass + 1;
		return name_tree_next_root(tree, dclass);
	}
	/* find class n in tree, we may get a direct hit, or if we don't
	 * this is the last item of the previous class so rbtree_next() takes
	 * us to the next root (if any) */
	key.node.key = &key;
	key.name = (uint8_t*)"\000";
	key.len = 1;
	key.labs = 0;
	key.dclass = *dclass;
	n = NULL;
	if(rbtree_find_less_equal(tree, &key, &n)) {
		/* exact */
		return 1;
	} else {
		/* smaller element */
		if(!n || n == RBTREE_NULL)
			return 0; /* nothing found */
		n = rbtree_next(n);
		if(n == RBTREE_NULL)
			return 0; /* no higher */
		p = (struct name_tree_node*)n;
		if(dname_is_root(p->name)) {
			*dclass = p->dclass;
			return 1;
		}
		/* not a root node, return next higher item */
		*dclass = p->dclass+1;
		return name_tree_next_root(tree, dclass);
	}
}