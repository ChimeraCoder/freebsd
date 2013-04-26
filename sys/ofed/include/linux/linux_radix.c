
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/radix-tree.h>
#include <linux/err.h>

static MALLOC_DEFINE(M_RADIX, "radix", "Linux radix compat");

static inline int
radix_max(struct radix_tree_root *root)
{
	return (1 << (root->height * RADIX_TREE_MAP_SHIFT)) - 1;
}

static inline int
radix_pos(long id, int height)
{
	return (id >> (RADIX_TREE_MAP_SHIFT * height)) & RADIX_TREE_MAP_MASK;
}

void *
radix_tree_lookup(struct radix_tree_root *root, unsigned long index)
{
	struct radix_tree_node *node;
	void *item;
	int height;

	item = NULL;
	node = root->rnode;
	height = root->height - 1;
	if (index > radix_max(root))
		goto out;
	while (height && node)
		node = node->slots[radix_pos(index, height--)];
	if (node)
		item = node->slots[radix_pos(index, 0)];

out:
	return (item);
}

void *
radix_tree_delete(struct radix_tree_root *root, unsigned long index)
{
	struct radix_tree_node *stack[RADIX_TREE_MAX_HEIGHT];
	struct radix_tree_node *node;
	void *item;
	int height;
	int idx;

	item = NULL;
	node = root->rnode;
	height = root->height - 1;
	if (index > radix_max(root))
		goto out;
	/*
	 * Find the node and record the path in stack.
	 */
	while (height && node) {
		stack[height] = node;
		node = node->slots[radix_pos(index, height--)];
	}
	idx = radix_pos(index, 0);
	if (node)
		item = node->slots[idx];
	/*
	 * If we removed something reduce the height of the tree.
	 */
	if (item)
		for (;;) {
			node->slots[idx] = NULL;
			node->count--;
			if (node->count > 0)
				break;
			free(node, M_RADIX);
			if (node == root->rnode) {
				root->rnode = NULL;
				root->height = 0;
				break;
			}
			height++;
			node = stack[height];
			idx = radix_pos(index, height);
		}
out:
	return (item);
}

int
radix_tree_insert(struct radix_tree_root *root, unsigned long index, void *item)
{
	struct radix_tree_node *node;
	int height;
	int idx;

	/*
 	 * Expand the tree to fit indexes as big as requested.
	 */
	while (root->rnode == NULL || radix_max(root) < index) {
		node = malloc(sizeof(*node), M_RADIX, root->gfp_mask | M_ZERO);
		if (node == NULL)
			return (-ENOMEM);
		node->slots[0] = root->rnode;
		if (root->rnode)
			node->count++;
		root->rnode = node;
		root->height++;
	}
	node = root->rnode;
	height = root->height - 1;
	/*
	 * Walk down the tree finding the correct node and allocating any
	 * missing nodes along the way.
	 */
	while (height) {
		idx = radix_pos(index, height);
		if (node->slots[idx] == NULL) {
			node->slots[idx] = malloc(sizeof(*node), M_RADIX,
			    root->gfp_mask | M_ZERO);
			if (node->slots[idx] == NULL)
				return (-ENOMEM);
			node->count++;
		}
		node = node->slots[idx];
		height--;
	}
	/*
	 * Insert and adjust count if the item does not already exist.
	 */
	idx = radix_pos(index, 0);
	if (node->slots[idx])
		return (-EEXIST);
	node->slots[idx] = item;
	node->count++;
	
	return (0);
}