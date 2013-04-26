
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
		/* Leak the rtree. */		return (NULL);
	}
	ret->height = height;
	if (bits_per_level * height > bits)
		ret->level2bits[0] = bits % bits_per_level;
	else
		ret->level2bits[0] = bits_per_level;
	for (i = 1; i < height; i++)
		ret->level2bits[i] = bits_per_level;

	ret->root = (void**)base_alloc(sizeof(void *) << ret->level2bits[0]);
	if (ret->root == NULL) {
		/*
		 * We leak the rtree here, since there's no generic base
		 * deallocation.
		 */
		return (NULL);
	}
	memset(ret->root, 0, sizeof(void *) << ret->level2bits[0]);

	return (ret);
}

void
rtree_prefork(rtree_t *rtree)
{

	malloc_mutex_prefork(&rtree->mutex);
}

void
rtree_postfork_parent(rtree_t *rtree)
{

	malloc_mutex_postfork_parent(&rtree->mutex);
}

void
rtree_postfork_child(rtree_t *rtree)
{

	malloc_mutex_postfork_child(&rtree->mutex);
}