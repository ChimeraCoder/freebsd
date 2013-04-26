
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

/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/systm.h>
#include <sys/cmn_err.h>
#include <sys/kobj.h>

struct zchdr {
	uint_t zch_magic;
	uint_t zch_size;
};

#define	ZCH_MAGIC	0x3cc13cc1

/*ARGSUSED*/
void *
zcalloc(void *opaque, uint_t items, uint_t size)
{
	size_t nbytes = sizeof (struct zchdr) + items * size;
	struct zchdr *z = kobj_zalloc(nbytes, KM_NOWAIT|KM_TMP);

	if (z == NULL)
		return (NULL);

	z->zch_magic = ZCH_MAGIC;
	z->zch_size = nbytes;

	return (z + 1);
}

/*ARGSUSED*/
void
zcfree(void *opaque, void *ptr)
{
	struct zchdr *z = ((struct zchdr *)ptr) - 1;

	if (z->zch_magic != ZCH_MAGIC)
		panic("zcfree region corrupt: hdr=%p ptr=%p", (void *)z, ptr);

	kobj_free(z, z->zch_size);
}

void
zmemcpy(void *dest, const void *source, uint_t len)
{
	bcopy(source, dest, len);
}

int
zmemcmp(const void *s1, const void *s2, uint_t len)
{
	return (bcmp(s1, s2, len));
}

void
zmemzero(void *dest, uint_t len)
{
	bzero(dest, len);
}