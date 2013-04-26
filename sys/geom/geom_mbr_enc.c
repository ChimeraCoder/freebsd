
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

/* Functions to encode or decode struct dos_partition into a bytestream
 * of correct endianess and packing.  These functions do no validation
 * or sanity checking, they only pack/unpack the fields correctly.
 *
 * NB!  This file must be usable both in kernel and userland.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/diskmbr.h>
#include <sys/endian.h>

void
dos_partition_dec(void const *pp, struct dos_partition *d)
{
	unsigned char const *p = pp;

	d->dp_flag = p[0];
	d->dp_shd = p[1];
	d->dp_ssect = p[2];
	d->dp_scyl = p[3];
	d->dp_typ = p[4];
	d->dp_ehd = p[5];
	d->dp_esect = p[6];
	d->dp_ecyl = p[7];
	d->dp_start = le32dec(p + 8);
	d->dp_size = le32dec(p + 12);
}

void
dos_partition_enc(void *pp, struct dos_partition *d)
{
	unsigned char *p = pp;

	p[0] = d->dp_flag;
	p[1] = d->dp_shd;
	p[2] = d->dp_ssect;
	p[3] = d->dp_scyl;
	p[4] = d->dp_typ;
	p[5] = d->dp_ehd;
	p[6] = d->dp_esect;
	p[7] = d->dp_ecyl;
	le32enc(p + 8, d->dp_start);
	le32enc(p + 12, d->dp_size);
}