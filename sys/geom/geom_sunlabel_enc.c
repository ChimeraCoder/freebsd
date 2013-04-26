
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
#include <sys/endian.h>
#include <sys/errno.h>
#include <sys/sun_disklabel.h>
#ifdef _KERNEL
#include <sys/systm.h>
#else
#include <string.h>
#endif

#define	SL_TEXT		0x0
#define	SL_TEXT_SIZEOF	0x80
#define	SL_VTOC_VERS	0x80
#define	SL_VTOC_VOLNAME	0x84
#define	SL_VTOC_NPART	0x8c
#define	SL_VTOC_MAP	0x8e
#define	SL_VTOC_SANITY	0xbc
#define	SL_RPM		0x1a4
#define	SL_PCYLINDERS	0x1a6
#define	SL_SPARESPERCYL	0x1a8
#define	SL_INTERLEAVE	0x1ae
#define	SL_NCYLINDERS	0x1b0
#define	SL_ACYLINDERS	0x1b2
#define	SL_NTRACKS	0x1b4
#define	SL_NSECTORS	0x1b6
#define	SL_PART		0x1bc
#define	SL_MAGIC	0x1fc
#define	SL_CKSUM	0x1fe

#define	SDKP_CYLOFFSET	0
#define	SDKP_NSECTORS	0x4
#define	SDKP_SIZEOF	0x8	/* size of a partition entry */

#define	SVTOC_TAG	0
#define	SVTOC_FLAG	0x2
#define	SVTOC_SIZEOF	0x4	/* size of a VTOC tag/flag entry */

/*
 * Decode the relevant fields of a sun disk label, and return zero if the
 * magic and checksum works out OK.
 */
int
sunlabel_dec(void const *pp, struct sun_disklabel *sl)
{
	const uint8_t *p;
	size_t i;
	u_int u;
	uint32_t vtocsane;
	uint16_t npart;

	p = pp;
	for (i = 0; i < sizeof(sl->sl_text); i++)
		sl->sl_text[i] = p[SL_TEXT + i];
	sl->sl_rpm = be16dec(p + SL_RPM);
	sl->sl_pcylinders = be16dec(p + SL_PCYLINDERS);
	sl->sl_sparespercyl = be16dec(p + SL_SPARESPERCYL);
	sl->sl_interleave = be16dec(p + SL_INTERLEAVE);
	sl->sl_ncylinders = be16dec(p + SL_NCYLINDERS);
	sl->sl_acylinders = be16dec(p + SL_ACYLINDERS);
	sl->sl_ntracks = be16dec(p + SL_NTRACKS);
	sl->sl_nsectors = be16dec(p + SL_NSECTORS);
	for (i = 0; i < SUN_NPART; i++) {
		sl->sl_part[i].sdkp_cyloffset = be32dec(p + SL_PART +
		    (i * SDKP_SIZEOF) + SDKP_CYLOFFSET);
		sl->sl_part[i].sdkp_nsectors = be32dec(p + SL_PART +
		    (i * SDKP_SIZEOF) + SDKP_NSECTORS);
	}
	sl->sl_magic = be16dec(p + SL_MAGIC);
	vtocsane = be32dec(p + SL_VTOC_SANITY);
	npart = be16dec(p + SL_VTOC_NPART);
	if (vtocsane == SUN_VTOC_SANE && npart == SUN_NPART) {
		/*
		 * Seems we've got SVR4-compatible VTOC information
		 * as well, decode it.
		 */
		sl->sl_vtoc_sane = vtocsane;
		sl->sl_vtoc_vers = be32dec(p + SL_VTOC_VERS);
		memcpy(sl->sl_vtoc_volname, p + SL_VTOC_VOLNAME,
		    SUN_VOLNAME_LEN);
		sl->sl_vtoc_nparts = SUN_NPART;
		for (i = 0; i < SUN_NPART; i++) {
			sl->sl_vtoc_map[i].svtoc_tag = be16dec(p +
				SL_VTOC_MAP + (i * SVTOC_SIZEOF) + SVTOC_TAG);
			sl->sl_vtoc_map[i].svtoc_flag = be16dec(p +
				SL_VTOC_MAP + (i * SVTOC_SIZEOF) + SVTOC_FLAG);
		}
	}
	for (i = u = 0; i < SUN_SIZE; i += 2)
		u ^= be16dec(p + i);
	if (u == 0 && sl->sl_magic == SUN_DKMAGIC)
		return (0);
	else
		return (EINVAL);
}

/*
 * Encode the relevant fields into a sun disklabel, compute new checksum.
 */
void
sunlabel_enc(void *pp, struct sun_disklabel *sl)
{
	uint8_t *p;
	size_t i;
	u_int u;

	p = pp;
	for (i = 0; i < SL_TEXT_SIZEOF; i++)
		p[SL_TEXT + i] = sl->sl_text[i];
	be16enc(p + SL_RPM, sl->sl_rpm);
	be16enc(p + SL_PCYLINDERS, sl->sl_pcylinders);
	be16enc(p + SL_SPARESPERCYL, sl->sl_sparespercyl);
	be16enc(p + SL_INTERLEAVE, sl->sl_interleave);
	be16enc(p + SL_NCYLINDERS, sl->sl_ncylinders);
	be16enc(p + SL_ACYLINDERS, sl->sl_acylinders);
	be16enc(p + SL_NTRACKS, sl->sl_ntracks);
	be16enc(p + SL_NSECTORS, sl->sl_nsectors);
	for (i = 0; i < SUN_NPART; i++) {
		be32enc(p + SL_PART + (i * SDKP_SIZEOF) + SDKP_CYLOFFSET,
		    sl->sl_part[i].sdkp_cyloffset);
		be32enc(p + SL_PART + (i * SDKP_SIZEOF) + SDKP_NSECTORS,
		    sl->sl_part[i].sdkp_nsectors);
	}
	be16enc(p + SL_MAGIC, sl->sl_magic);
	if (sl->sl_vtoc_sane == SUN_VTOC_SANE
	    && sl->sl_vtoc_nparts == SUN_NPART) {
		/*
		 * Write SVR4-compatible VTOC elements.
		 */
		be32enc(p + SL_VTOC_VERS, sl->sl_vtoc_vers);
		be32enc(p + SL_VTOC_SANITY, SUN_VTOC_SANE);
		memcpy(p + SL_VTOC_VOLNAME, sl->sl_vtoc_volname,
		    SUN_VOLNAME_LEN);
		be16enc(p + SL_VTOC_NPART, SUN_NPART);
		for (i = 0; i < SUN_NPART; i++) {
			be16enc(p + SL_VTOC_MAP + (i * SVTOC_SIZEOF)
				+ SVTOC_TAG,
				sl->sl_vtoc_map[i].svtoc_tag);
			be16enc(p + SL_VTOC_MAP + (i * SVTOC_SIZEOF)
				+ SVTOC_FLAG,
				sl->sl_vtoc_map[i].svtoc_flag);
		}
	}
	for (i = u = 0; i < SUN_SIZE; i += 2)
		u ^= be16dec(p + i);
	be16enc(p + SL_CKSUM, u);
}