
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

#include <dev/drm2/drmP.h>
__FBSDID("$FreeBSD$");

struct drm_list_sort_thunk {
	int (*cmp)(void *, struct list_head *, struct list_head *);
	void *priv;
};

static int
drm_le_cmp(void *priv, const void *d1, const void *d2)
{
	struct list_head *le1, *le2;
	struct drm_list_sort_thunk *thunk;

	thunk = priv;
	le1 = __DECONST(struct list_head *, d1);
	le2 = __DECONST(struct list_head *, d2);
	return ((thunk->cmp)(thunk->priv, le1, le2));
}

/*
 * Punt and use array sort.
 */
void
drm_list_sort(void *priv, struct list_head *head, int (*cmp)(void *priv,
    struct list_head *a, struct list_head *b))
{
	struct drm_list_sort_thunk thunk;
	struct list_head **ar, *le;
	int count, i;

	count = 0;
	list_for_each(le, head)
		count++;
	ar = malloc(sizeof(struct list_head *) * count, M_TEMP, M_WAITOK);
	i = 0;
	list_for_each(le, head)
		ar[i++] = le;
	thunk.cmp = cmp;
	thunk.priv = priv;
	qsort_r(ar, count, sizeof(struct list_head *), &thunk, drm_le_cmp);
	INIT_LIST_HEAD(head);
	for (i = 0; i < count; i++)
		list_add_tail(ar[i], head);
	free(ar, M_TEMP);
}