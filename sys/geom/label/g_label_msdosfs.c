
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
#include <geom/label/g_label_msdosfs.h>

#define G_LABEL_MSDOSFS_DIR	"msdosfs"
#define LABEL_NO_NAME		"NO NAME    "

static void
g_label_msdosfs_taste(struct g_consumer *cp, char *label, size_t size)
{
	struct g_provider *pp;
	FAT_BSBPB *pfat_bsbpb;
	FAT32_BSBPB *pfat32_bsbpb;
	FAT_DES *pfat_entry;
	uint8_t *sector0, *sector;
	uint32_t i;

	g_topology_assert_not();
	pp = cp->provider;
	sector0 = NULL;
	sector = NULL;
	bzero(label, size);

	/* Check if the sector size of the medium is a valid FAT sector size. */
	switch(pp->sectorsize) {
	case 512:
	case 1024:
	case 2048:
	case 4096:
		break;
	default:
		G_LABEL_DEBUG(1, "MSDOSFS: %s: sector size %d not compatible.",
		    pp->name, pp->sectorsize);
		return;
	}

	/* Load 1st sector with boot sector and boot parameter block. */
	sector0 = (uint8_t *)g_read_data(cp, 0, pp->sectorsize, NULL);
	if (sector0 == NULL)
		return;

	/* Check for the FAT boot sector signature. */
	if (sector0[510] != 0x55 || sector0[511] != 0xaa) {
		G_LABEL_DEBUG(1, "MSDOSFS: %s: no FAT signature found.",
		    pp->name);
		goto error;
	}


	/*
	 * Test if this is really a FAT volume and determine the FAT type.
	 */

	pfat_bsbpb = (FAT_BSBPB *)sector0;
	pfat32_bsbpb = (FAT32_BSBPB *)sector0;

	if (UINT16BYTES(pfat_bsbpb->BPB_FATSz16) != 0) {
		/*
		 * If the BPB_FATSz16 field is not zero and the string "FAT" is
		 * at the right place, this should be a FAT12 or FAT16 volume.
		 */
		if (strncmp(pfat_bsbpb->BS_FilSysType, "FAT", 3) != 0) {
			G_LABEL_DEBUG(1,
			    "MSDOSFS: %s: FAT12/16 volume not valid.",
			    pp->name);
			goto error;
		}
		G_LABEL_DEBUG(1, "MSDOSFS: %s: FAT12/FAT16 volume detected.",
		    pp->name);

		/* A volume with no name should have "NO NAME    " as label. */
		if (strncmp(pfat_bsbpb->BS_VolLab, LABEL_NO_NAME,
		    sizeof(pfat_bsbpb->BS_VolLab)) == 0) {
			G_LABEL_DEBUG(1,
			    "MSDOSFS: %s: FAT12/16 volume has no name.",
			    pp->name);
			goto error;
		}
		strlcpy(label, pfat_bsbpb->BS_VolLab,
		    MIN(size, sizeof(pfat_bsbpb->BS_VolLab) + 1));
	} else if (UINT32BYTES(pfat32_bsbpb->BPB_FATSz32) != 0) {
		uint32_t fat_FirstDataSector, fat_BytesPerSector, offset;

		/*
		 * If the BPB_FATSz32 field is not zero and the string "FAT" is
		 * at the right place, this should be a FAT32 volume.
		 */
		if (strncmp(pfat32_bsbpb->BS_FilSysType, "FAT", 3) != 0) {
			G_LABEL_DEBUG(1, "MSDOSFS: %s: FAT32 volume not valid.",
			    pp->name);
			goto error;
		}
		G_LABEL_DEBUG(1, "MSDOSFS: %s: FAT32 volume detected.",
		    pp->name);

		/*
		 * If the volume label is not "NO NAME    " we're done.
		 */
		if (strncmp(pfat32_bsbpb->BS_VolLab, LABEL_NO_NAME,
		    sizeof(pfat32_bsbpb->BS_VolLab)) != 0) {
			strlcpy(label, pfat32_bsbpb->BS_VolLab,
			    MIN(size, sizeof(pfat32_bsbpb->BS_VolLab) + 1));
			goto endofchecks;
		}

		/*
		 * If the volume label "NO NAME    " is in the boot sector, the
		 * label of FAT32 volumes may be stored as a special entry in
		 * the root directory.
		 */
		fat_FirstDataSector =
		    UINT16BYTES(pfat32_bsbpb->BPB_RsvdSecCnt) +
		    (pfat32_bsbpb->BPB_NumFATs *
		     UINT32BYTES(pfat32_bsbpb->BPB_FATSz32));
		fat_BytesPerSector = UINT16BYTES(pfat32_bsbpb->BPB_BytsPerSec);

		G_LABEL_DEBUG(2,
		    "MSDOSFS: FAT_FirstDataSector=0x%x, FAT_BytesPerSector=%d",
		    fat_FirstDataSector, fat_BytesPerSector);

		for (offset = fat_BytesPerSector * fat_FirstDataSector;;
		    offset += fat_BytesPerSector) {
			sector = (uint8_t *)g_read_data(cp, offset,
			    fat_BytesPerSector, NULL);
			if (sector == NULL)
				goto error;

			pfat_entry = (FAT_DES *)sector;
			do {
				/* No more entries available. */
				if (pfat_entry->DIR_Name[0] == 0) {
					G_LABEL_DEBUG(1, "MSDOSFS: %s: "
					    "FAT32 volume has no name.",
					    pp->name);
					goto error;
				}

				/* Skip empty or long name entries. */
				if (pfat_entry->DIR_Name[0] == 0xe5 ||
				    (pfat_entry->DIR_Attr &
				     FAT_DES_ATTR_LONG_NAME) ==
				    FAT_DES_ATTR_LONG_NAME) {
					continue;
				}

				/*
				 * The name of the entry is the volume label if
				 * ATTR_VOLUME_ID is set.
				 */
				if (pfat_entry->DIR_Attr &
				    FAT_DES_ATTR_VOLUME_ID) {
					strlcpy(label, pfat_entry->DIR_Name,
					    MIN(size,
					    sizeof(pfat_entry->DIR_Name) + 1));
					goto endofchecks;
				}
			} while((uint8_t *)(++pfat_entry) <
			    (uint8_t *)(sector + fat_BytesPerSector));
			g_free(sector);
		}
	} else {
		G_LABEL_DEBUG(1, "MSDOSFS: %s: no FAT volume detected.",
		    pp->name);
		goto error;
	}

endofchecks:
	for (i = size - 1; i > 0; i--) {
		if (label[i] == '\0')
			continue;
		else if (label[i] == ' ')
			label[i] = '\0';
		else
			break;
	}

error:
	if (sector0 != NULL)
		g_free(sector0);
	if (sector != NULL)
		g_free(sector);
}

struct g_label_desc g_label_msdosfs = {
	.ld_taste = g_label_msdosfs_taste,
	.ld_dir = G_LABEL_MSDOSFS_DIR,
	.ld_enabled = 1
};

G_LABEL_INIT(msdosfs, g_label_msdosfs, "Create device nodes for MSDOSFS volumes");