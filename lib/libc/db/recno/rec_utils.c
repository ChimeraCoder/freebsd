
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rec_utils.c	8.6 (Berkeley) 7/16/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>
#include "recno.h"

/*
 * __rec_ret --
 *	Build return data.
 *
 * Parameters:
 *	t:	tree
 *	e:	key/data pair to be returned
 *   nrec:	record number
 *    key:	user's key structure
 *   data:	user's data structure
 *
 * Returns:
 *	RET_SUCCESS, RET_ERROR.
 */
int
__rec_ret(BTREE *t, EPG *e, recno_t nrec, DBT *key, DBT *data)
{
	RLEAF *rl;
	void *p;

	if (key == NULL)
		goto dataonly;

	/* We have to copy the key, it's not on the page. */
	if (sizeof(recno_t) > t->bt_rkey.size) {
		p = realloc(t->bt_rkey.data, sizeof(recno_t));
		if (p == NULL)
			return (RET_ERROR);
		t->bt_rkey.data = p;
		t->bt_rkey.size = sizeof(recno_t);
	}
	memmove(t->bt_rkey.data, &nrec, sizeof(recno_t));
	key->size = sizeof(recno_t);
	key->data = t->bt_rkey.data;

dataonly:
	if (data == NULL)
		return (RET_SUCCESS);

	/*
	 * We must copy big keys/data to make them contigous.  Otherwise,
	 * leave the page pinned and don't copy unless the user specified
	 * concurrent access.
	 */
	rl = GETRLEAF(e->page, e->index);
	if (rl->flags & P_BIGDATA) {
		if (__ovfl_get(t, rl->bytes,
		    &data->size, &t->bt_rdata.data, &t->bt_rdata.size))
			return (RET_ERROR);
		data->data = t->bt_rdata.data;
	} else if (F_ISSET(t, B_DB_LOCK)) {
		/* Use +1 in case the first record retrieved is 0 length. */
		if (rl->dsize + 1 > t->bt_rdata.size) {
			p = realloc(t->bt_rdata.data, rl->dsize + 1);
			if (p == NULL)
				return (RET_ERROR);
			t->bt_rdata.data = p;
			t->bt_rdata.size = rl->dsize + 1;
		}
		memmove(t->bt_rdata.data, rl->bytes, rl->dsize);
		data->size = rl->dsize;
		data->data = t->bt_rdata.data;
	} else {
		data->size = rl->dsize;
		data->data = rl->bytes;
	}
	return (RET_SUCCESS);
}