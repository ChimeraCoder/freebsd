
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
static char sccsid[] = "@(#)rec_get.c	8.9 (Berkeley) 8/18/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <db.h>
#include "recno.h"

/*
 * __REC_GET -- Get a record from the btree.
 *
 * Parameters:
 *	dbp:	pointer to access method
 *	key:	key to find
 *	data:	data to return
 *	flag:	currently unused
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS and RET_SPECIAL if the key not found.
 */
int
__rec_get(const DB *dbp, const DBT *key, DBT *data, u_int flags)
{
	BTREE *t;
	EPG *e;
	recno_t nrec;
	int status;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/* Get currently doesn't take any flags, and keys of 0 are illegal. */
	if (flags || (nrec = *(recno_t *)key->data) == 0) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	/*
	 * If we haven't seen this record yet, try to find it in the
	 * original file.
	 */
	if (nrec > t->bt_nrecs) {
		if (F_ISSET(t, R_EOF | R_INMEM))
			return (RET_SPECIAL);
		if ((status = t->bt_irec(t, nrec)) != RET_SUCCESS)
			return (status);
	}

	--nrec;
	if ((e = __rec_search(t, nrec, SEARCH)) == NULL)
		return (RET_ERROR);

	status = __rec_ret(t, e, 0, NULL, data);
	if (F_ISSET(t, B_DB_LOCK))
		mpool_put(t->bt_mp, e->page, 0);
	else
		t->bt_pinned = e->page;
	return (status);
}

/*
 * __REC_FPIPE -- Get fixed length records from a pipe.
 *
 * Parameters:
 *	t:	tree
 *	cnt:	records to read
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
__rec_fpipe(BTREE *t, recno_t top)
{
	DBT data;
	recno_t nrec;
	size_t len;
	int ch;
	u_char *p;

	if (t->bt_rdata.size < t->bt_reclen) {
		t->bt_rdata.data = reallocf(t->bt_rdata.data, t->bt_reclen);
		if (t->bt_rdata.data == NULL)
			return (RET_ERROR);
		t->bt_rdata.size = t->bt_reclen;
	}
	data.data = t->bt_rdata.data;
	data.size = t->bt_reclen;

	for (nrec = t->bt_nrecs; nrec < top;) {
		len = t->bt_reclen;
		for (p = t->bt_rdata.data;; *p++ = ch)
			if ((ch = getc(t->bt_rfp)) == EOF || !--len) {
				if (ch != EOF)
					*p = ch;
				if (len != 0)
					memset(p, t->bt_bval, len);
				if (__rec_iput(t,
				    nrec, &data, 0) != RET_SUCCESS)
					return (RET_ERROR);
				++nrec;
				break;
			}
		if (ch == EOF)
			break;
	}
	if (nrec < top) {
		F_SET(t, R_EOF);
		return (RET_SPECIAL);
	}
	return (RET_SUCCESS);
}

/*
 * __REC_VPIPE -- Get variable length records from a pipe.
 *
 * Parameters:
 *	t:	tree
 *	cnt:	records to read
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
__rec_vpipe(BTREE *t, recno_t top)
{
	DBT data;
	recno_t nrec;
	size_t len;
	size_t sz;
	int bval, ch;
	u_char *p;

	bval = t->bt_bval;
	for (nrec = t->bt_nrecs; nrec < top; ++nrec) {
		for (p = t->bt_rdata.data,
		    sz = t->bt_rdata.size;; *p++ = ch, --sz) {
			if ((ch = getc(t->bt_rfp)) == EOF || ch == bval) {
				data.data = t->bt_rdata.data;
				data.size = p - (u_char *)t->bt_rdata.data;
				if (ch == EOF && data.size == 0)
					break;
				if (__rec_iput(t, nrec, &data, 0)
				    != RET_SUCCESS)
					return (RET_ERROR);
				break;
			}
			if (sz == 0) {
				len = p - (u_char *)t->bt_rdata.data;
				t->bt_rdata.size += (sz = 256);
				t->bt_rdata.data = reallocf(t->bt_rdata.data, t->bt_rdata.size);
				if (t->bt_rdata.data == NULL)
					return (RET_ERROR);
				p = (u_char *)t->bt_rdata.data + len;
			}
		}
		if (ch == EOF)
			break;
	}
	if (nrec < top) {
		F_SET(t, R_EOF);
		return (RET_SPECIAL);
	}
	return (RET_SUCCESS);
}

/*
 * __REC_FMAP -- Get fixed length records from a file.
 *
 * Parameters:
 *	t:	tree
 *	cnt:	records to read
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
__rec_fmap(BTREE *t, recno_t top)
{
	DBT data;
	recno_t nrec;
	u_char *sp, *ep, *p;
	size_t len;

	if (t->bt_rdata.size < t->bt_reclen) {
		t->bt_rdata.data = reallocf(t->bt_rdata.data, t->bt_reclen);
		if (t->bt_rdata.data == NULL)
			return (RET_ERROR);
		t->bt_rdata.size = t->bt_reclen;
	}
	data.data = t->bt_rdata.data;
	data.size = t->bt_reclen;

	sp = (u_char *)t->bt_cmap;
	ep = (u_char *)t->bt_emap;
	for (nrec = t->bt_nrecs; nrec < top; ++nrec) {
		if (sp >= ep) {
			F_SET(t, R_EOF);
			return (RET_SPECIAL);
		}
		len = t->bt_reclen;
		for (p = t->bt_rdata.data;
		    sp < ep && len > 0; *p++ = *sp++, --len);
		if (len != 0)
			memset(p, t->bt_bval, len);
		if (__rec_iput(t, nrec, &data, 0) != RET_SUCCESS)
			return (RET_ERROR);
	}
	t->bt_cmap = (caddr_t)sp;
	return (RET_SUCCESS);
}

/*
 * __REC_VMAP -- Get variable length records from a file.
 *
 * Parameters:
 *	t:	tree
 *	cnt:	records to read
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
__rec_vmap(BTREE *t, recno_t top)
{
	DBT data;
	u_char *sp, *ep;
	recno_t nrec;
	int bval;

	sp = (u_char *)t->bt_cmap;
	ep = (u_char *)t->bt_emap;
	bval = t->bt_bval;

	for (nrec = t->bt_nrecs; nrec < top; ++nrec) {
		if (sp >= ep) {
			F_SET(t, R_EOF);
			return (RET_SPECIAL);
		}
		for (data.data = sp; sp < ep && *sp != bval; ++sp);
		data.size = sp - (u_char *)data.data;
		if (__rec_iput(t, nrec, &data, 0) != RET_SUCCESS)
			return (RET_ERROR);
		++sp;
	}
	t->bt_cmap = (caddr_t)sp;
	return (RET_SUCCESS);
}