
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
static char sccsid[] = "@(#)bt_conv.c	8.5 (Berkeley) 8/17/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <stdio.h>

#include <db.h>
#include "btree.h"

static void mswap(PAGE *);

/*
 * __BT_BPGIN, __BT_BPGOUT --
 *	Convert host-specific number layout to/from the host-independent
 *	format stored on disk.
 *
 * Parameters:
 *	t:	tree
 *	pg:	page number
 *	h:	page to convert
 */
void
__bt_pgin(void *t, pgno_t pg, void *pp)
{
	PAGE *h;
	indx_t i, top;
	u_char flags;
	char *p;

	if (!F_ISSET(((BTREE *)t), B_NEEDSWAP))
		return;
	if (pg == P_META) {
		mswap(pp);
		return;
	}

	h = pp;
	M_32_SWAP(h->pgno);
	M_32_SWAP(h->prevpg);
	M_32_SWAP(h->nextpg);
	M_32_SWAP(h->flags);
	M_16_SWAP(h->lower);
	M_16_SWAP(h->upper);

	top = NEXTINDEX(h);
	if ((h->flags & P_TYPE) == P_BINTERNAL)
		for (i = 0; i < top; i++) {
			M_16_SWAP(h->linp[i]);
			p = (char *)GETBINTERNAL(h, i);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			P_32_SWAP(p);
			p += sizeof(pgno_t);
			if (*(u_char *)p & P_BIGKEY) {
				p += sizeof(u_char);
				P_32_SWAP(p);
				p += sizeof(pgno_t);
				P_32_SWAP(p);
			}
		}
	else if ((h->flags & P_TYPE) == P_BLEAF)
		for (i = 0; i < top; i++) {
			M_16_SWAP(h->linp[i]);
			p = (char *)GETBLEAF(h, i);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			flags = *(u_char *)p;
			if (flags & (P_BIGKEY | P_BIGDATA)) {
				p += sizeof(u_char);
				if (flags & P_BIGKEY) {
					P_32_SWAP(p);
					p += sizeof(pgno_t);
					P_32_SWAP(p);
				}
				if (flags & P_BIGDATA) {
					p += sizeof(u_int32_t);
					P_32_SWAP(p);
					p += sizeof(pgno_t);
					P_32_SWAP(p);
				}
			}
		}
}

void
__bt_pgout(void *t, pgno_t pg, void *pp)
{
	PAGE *h;
	indx_t i, top;
	u_char flags;
	char *p;

	if (!F_ISSET(((BTREE *)t), B_NEEDSWAP))
		return;
	if (pg == P_META) {
		mswap(pp);
		return;
	}

	h = pp;
	top = NEXTINDEX(h);
	if ((h->flags & P_TYPE) == P_BINTERNAL)
		for (i = 0; i < top; i++) {
			p = (char *)GETBINTERNAL(h, i);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			P_32_SWAP(p);
			p += sizeof(pgno_t);
			if (*(u_char *)p & P_BIGKEY) {
				p += sizeof(u_char);
				P_32_SWAP(p);
				p += sizeof(pgno_t);
				P_32_SWAP(p);
			}
			M_16_SWAP(h->linp[i]);
		}
	else if ((h->flags & P_TYPE) == P_BLEAF)
		for (i = 0; i < top; i++) {
			p = (char *)GETBLEAF(h, i);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			flags = *(u_char *)p;
			if (flags & (P_BIGKEY | P_BIGDATA)) {
				p += sizeof(u_char);
				if (flags & P_BIGKEY) {
					P_32_SWAP(p);
					p += sizeof(pgno_t);
					P_32_SWAP(p);
				}
				if (flags & P_BIGDATA) {
					p += sizeof(u_int32_t);
					P_32_SWAP(p);
					p += sizeof(pgno_t);
					P_32_SWAP(p);
				}
			}
			M_16_SWAP(h->linp[i]);
		}

	M_32_SWAP(h->pgno);
	M_32_SWAP(h->prevpg);
	M_32_SWAP(h->nextpg);
	M_32_SWAP(h->flags);
	M_16_SWAP(h->lower);
	M_16_SWAP(h->upper);
}

/*
 * MSWAP -- Actually swap the bytes on the meta page.
 *
 * Parameters:
 *	p:	page to convert
 */
static void
mswap(PAGE *pg)
{
	char *p;

	p = (char *)pg;
	P_32_SWAP(p);		/* magic */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* version */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* psize */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* free */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* nrecs */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* flags */
	p += sizeof(u_int32_t);
}