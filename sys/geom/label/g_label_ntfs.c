
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>

#include <geom/geom.h>
#include <geom/label/g_label.h>

#define	NTFS_A_VOLUMENAME	0x60
#define	NTFS_FILEMAGIC		((uint32_t)(0x454C4946))
#define	NTFS_VOLUMEINO		3

#define G_LABEL_NTFS_DIR	"ntfs"

struct ntfs_attr {
	uint32_t	a_type;
	uint32_t	reclen;
	uint8_t		a_flag;
	uint8_t		a_namelen;
	uint8_t		a_nameoff;
	uint8_t		reserved1;
	uint8_t		a_compression;
	uint8_t		reserved2;
	uint16_t	a_index;
	uint16_t	a_datalen;
	uint16_t	reserved3;
	uint16_t	a_dataoff;
	uint16_t	a_indexed;
} __packed;

struct ntfs_filerec {
	uint32_t	fr_hdrmagic;
	uint16_t	fr_hdrfoff;
	uint16_t	fr_hdrfnum;
	uint8_t		reserved[8];
	uint16_t	fr_seqnum;
	uint16_t	fr_nlink;
	uint16_t	fr_attroff;
	uint16_t	fr_flags;
	uint32_t	fr_size;
	uint32_t	fr_allocated;
	uint64_t	fr_mainrec;
	uint16_t	fr_attrnum;
} __packed;

struct ntfs_bootfile {
	uint8_t		reserved1[3];
	uint8_t		bf_sysid[8];
	uint16_t	bf_bps;
	uint8_t		bf_spc;
	uint8_t		reserved2[7];
	uint8_t		bf_media;
	uint8_t		reserved3[2];
	uint16_t	bf_spt;
	uint16_t	bf_heads;
	uint8_t		reserver4[12];
	uint64_t	bf_spv;
	uint64_t	bf_mftcn;
	uint64_t	bf_mftmirrcn;
	uint8_t		bf_mftrecsz;
	uint32_t	bf_ibsz;
	uint32_t	bf_volsn;
} __packed;

static void
g_label_ntfs_taste(struct g_consumer *cp, char *label, size_t size)
{
	struct g_provider *pp;
	struct ntfs_bootfile *bf;
	struct ntfs_filerec *fr;
	struct ntfs_attr *atr;
	off_t voloff;
	char *filerecp, *ap;
	char mftrecsz, vnchar;
	int recsize, j;

	g_topology_assert_not();

	label[0] = '\0';
	pp = cp->provider;
	filerecp = NULL;

	bf = (struct ntfs_bootfile *)g_read_data(cp, 0, pp->sectorsize, NULL);
	if (bf == NULL || strncmp(bf->bf_sysid, "NTFS    ", 8) != 0)
		goto done;

	mftrecsz = (char)bf->bf_mftrecsz;
	recsize = (mftrecsz > 0) ? (mftrecsz * bf->bf_bps * bf->bf_spc) : (1 << -mftrecsz);
	if (recsize == 0 || recsize % pp->sectorsize != 0)
		goto done;

	voloff = bf->bf_mftcn * bf->bf_spc * bf->bf_bps +
	    recsize * NTFS_VOLUMEINO;
	if (voloff % pp->sectorsize != 0)
		goto done;

	filerecp = g_read_data(cp, voloff, recsize, NULL);
	if (filerecp == NULL)
		goto done;
	fr = (struct ntfs_filerec *)filerecp;

	if (fr->fr_hdrmagic != NTFS_FILEMAGIC)
		goto done;

	for (ap = filerecp + fr->fr_attroff;
	    atr = (struct ntfs_attr *)ap, atr->a_type != -1;
	    ap += atr->reclen) {
		if (atr->a_type == NTFS_A_VOLUMENAME) {
			if(atr->a_datalen >= size *2){
				label[0] = 0;
				goto done;
			}
			/*
			 *UNICODE to ASCII.
			 * Should we need to use iconv(9)?
			 */
			for (j = 0; j < atr->a_datalen; j++) {
				vnchar = *(ap + atr->a_dataoff + j);
				if (j & 1) {
					if (vnchar) {
						label[0] = 0;
						goto done;
					}
				} else {
					label[j / 2] = vnchar;
				}
			}
			label[j / 2] = 0;
			break;
		}
	}
done:
	if (bf != NULL)
		g_free(bf);
	if (filerecp != NULL)
		g_free(filerecp);
}

struct g_label_desc g_label_ntfs = {
	.ld_taste = g_label_ntfs_taste,
	.ld_dir = G_LABEL_NTFS_DIR,
	.ld_enabled = 1
};

G_LABEL_INIT(ntfs, g_label_ntfs, "Create device nodes for NTFS volumes");