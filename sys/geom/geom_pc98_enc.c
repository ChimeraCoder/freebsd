
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/diskpc98.h>
#include <sys/endian.h>

void
pc98_partition_dec(void const *pp, struct pc98_partition *d)
{
	unsigned char const *ptr = pp;
	u_int i;

	d->dp_mid = ptr[0];
	d->dp_sid = ptr[1];
	d->dp_dum1 = ptr[2];
	d->dp_dum2 = ptr[3];
	d->dp_ipl_sct = ptr[4];
	d->dp_ipl_head = ptr[5];
	d->dp_ipl_cyl = le16dec(ptr + 6);
	d->dp_ssect = ptr[8];
	d->dp_shd = ptr[9];
	d->dp_scyl = le16dec(ptr + 10);
	d->dp_esect = ptr[12];
	d->dp_ehd = ptr[13];
	d->dp_ecyl = le16dec(ptr + 14);
	for (i = 0; i < sizeof (d->dp_name); i++)
		d->dp_name[i] = ptr[16 + i];
}

void
pc98_partition_enc(void *pp, struct pc98_partition *d)
{
	unsigned char *ptr = pp;
	u_int i;

	ptr[0] = d->dp_mid;
	ptr[1] = d->dp_sid;
	ptr[2] = d->dp_dum1;
	ptr[3] = d->dp_dum2;
	ptr[4] = d->dp_ipl_sct;
	ptr[5] = d->dp_ipl_head;
	le16enc(ptr + 6, d->dp_ipl_cyl);
	ptr[8] = d->dp_ssect;
	ptr[9] = d->dp_shd;
	le16enc(ptr + 10, d->dp_scyl);
	ptr[12] = d->dp_esect;
	ptr[13] = d->dp_ehd;
	le16enc(ptr + 14, d->dp_ecyl);
	for (i = 0; i < sizeof (d->dp_name); i++)
		ptr[16 + i] = d->dp_name[i];
}