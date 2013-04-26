
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
#ifndef lint
__RCSID("$NetBSD: boot.c,v 1.11 2006/06/05 16:51:18 christos Exp ");
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "ext.h"
#include "fsutil.h"

int
readboot(int dosfs, struct bootblock *boot)
{
	u_char block[DOSBOOTBLOCKSIZE];
	u_char fsinfo[2 * DOSBOOTBLOCKSIZE];
	u_char backup[DOSBOOTBLOCKSIZE];
	int ret = FSOK;
	int i;
	
	if ((size_t)read(dosfs, block, sizeof block) != sizeof block) {
		perr("could not read boot block");
		return FSFATAL;
	}

	if (block[510] != 0x55 || block[511] != 0xaa) {
		pfatal("Invalid signature in boot block: %02x%02x",
		    block[511], block[510]);
		return FSFATAL;
	}

	memset(boot, 0, sizeof *boot);
	boot->ValidFat = -1;

	/* decode bios parameter block */
	boot->bpbBytesPerSec = block[11] + (block[12] << 8);
	boot->bpbSecPerClust = block[13];
	boot->bpbResSectors = block[14] + (block[15] << 8);
	boot->bpbFATs = block[16];
	boot->bpbRootDirEnts = block[17] + (block[18] << 8);
	boot->bpbSectors = block[19] + (block[20] << 8);
	boot->bpbMedia = block[21];
	boot->bpbFATsmall = block[22] + (block[23] << 8);
	boot->SecPerTrack = block[24] + (block[25] << 8);
	boot->bpbHeads = block[26] + (block[27] << 8);
	boot->bpbHiddenSecs = block[28] + (block[29] << 8) +
	    (block[30] << 16) + (block[31] << 24);
	boot->bpbHugeSectors = block[32] + (block[33] << 8) +
	    (block[34] << 16) + (block[35] << 24);

	boot->FATsecs = boot->bpbFATsmall;

	if (!boot->bpbRootDirEnts)
		boot->flags |= FAT32;
	if (boot->flags & FAT32) {
		boot->FATsecs = block[36] + (block[37] << 8)
				+ (block[38] << 16) + (block[39] << 24);
		if (block[40] & 0x80)
			boot->ValidFat = block[40] & 0x0f;

		/* check version number: */
		if (block[42] || block[43]) {
			/* Correct?				XXX */
			pfatal("Unknown file system version: %x.%x",
			       block[43], block[42]);
			return FSFATAL;
		}
		boot->bpbRootClust = block[44] + (block[45] << 8)
			       + (block[46] << 16) + (block[47] << 24);
		boot->bpbFSInfo = block[48] + (block[49] << 8);
		boot->bpbBackup = block[50] + (block[51] << 8);

		if (lseek(dosfs, boot->bpbFSInfo * boot->bpbBytesPerSec,
		    SEEK_SET) != boot->bpbFSInfo * boot->bpbBytesPerSec
		    || read(dosfs, fsinfo, sizeof fsinfo) != sizeof fsinfo) {
			perr("could not read fsinfo block");
			return FSFATAL;
		}
		if (memcmp(fsinfo, "RRaA", 4)
		    || memcmp(fsinfo + 0x1e4, "rrAa", 4)
		    || fsinfo[0x1fc]
		    || fsinfo[0x1fd]
		    || fsinfo[0x1fe] != 0x55
		    || fsinfo[0x1ff] != 0xaa
		    || fsinfo[0x3fc]
		    || fsinfo[0x3fd]
		    || fsinfo[0x3fe] != 0x55
		    || fsinfo[0x3ff] != 0xaa) {
			pwarn("Invalid signature in fsinfo block\n");
			if (ask(0, "Fix")) {
				memcpy(fsinfo, "RRaA", 4);
				memcpy(fsinfo + 0x1e4, "rrAa", 4);
				fsinfo[0x1fc] = fsinfo[0x1fd] = 0;
				fsinfo[0x1fe] = 0x55;
				fsinfo[0x1ff] = 0xaa;
				fsinfo[0x3fc] = fsinfo[0x3fd] = 0;
				fsinfo[0x3fe] = 0x55;
				fsinfo[0x3ff] = 0xaa;
				if (lseek(dosfs, boot->bpbFSInfo *
				    boot->bpbBytesPerSec, SEEK_SET)
				    != boot->bpbFSInfo * boot->bpbBytesPerSec
				    || write(dosfs, fsinfo, sizeof fsinfo)
				    != sizeof fsinfo) {
					perr("Unable to write bpbFSInfo");
					return FSFATAL;
				}
				ret = FSBOOTMOD;
			} else
				boot->bpbFSInfo = 0;
		}
		if (boot->bpbFSInfo) {
			boot->FSFree = fsinfo[0x1e8] + (fsinfo[0x1e9] << 8)
				       + (fsinfo[0x1ea] << 16)
				       + (fsinfo[0x1eb] << 24);
			boot->FSNext = fsinfo[0x1ec] + (fsinfo[0x1ed] << 8)
				       + (fsinfo[0x1ee] << 16)
				       + (fsinfo[0x1ef] << 24);
		}

		if (lseek(dosfs, boot->bpbBackup * boot->bpbBytesPerSec,
		    SEEK_SET)
		    != boot->bpbBackup * boot->bpbBytesPerSec
		    || read(dosfs, backup, sizeof backup) != sizeof  backup) {
			perr("could not read backup bootblock");
			return FSFATAL;
		}
		backup[65] = block[65];				/* XXX */
		if (memcmp(block + 11, backup + 11, 79)) {
			/*
			 * XXX We require a reference that explains
			 * that these bytes need to match, or should
			 * drop the check.  gdt@NetBSD has observed
			 * filesystems that work fine under Windows XP
			 * and NetBSD that do not match, so the
			 * requirement is suspect.  For now, just
			 * print out useful information and continue.
			 */
			pfatal("backup (block %d) mismatch with primary bootblock:\n",
			        boot->bpbBackup);
			for (i = 11; i < 11 + 90; i++) {
				if (block[i] != backup[i])
					pfatal("\ti=%d\tprimary 0x%02x\tbackup 0x%02x\n",
					       i, block[i], backup[i]);
			}
		}
		/* Check backup bpbFSInfo?					XXX */
	}

	boot->ClusterOffset = (boot->bpbRootDirEnts * 32 +
	    boot->bpbBytesPerSec - 1) / boot->bpbBytesPerSec +
	    boot->bpbResSectors + boot->bpbFATs * boot->FATsecs -
	    CLUST_FIRST * boot->bpbSecPerClust;

	if (boot->bpbBytesPerSec % DOSBOOTBLOCKSIZE != 0) {
		pfatal("Invalid sector size: %u", boot->bpbBytesPerSec);
		return FSFATAL;
	}
	if (boot->bpbSecPerClust == 0) {
		pfatal("Invalid cluster size: %u", boot->bpbSecPerClust);
		return FSFATAL;
	}
	if (boot->bpbSectors) {
		boot->bpbHugeSectors = 0;
		boot->NumSectors = boot->bpbSectors;
	} else
		boot->NumSectors = boot->bpbHugeSectors;
	boot->NumClusters = (boot->NumSectors - boot->ClusterOffset) /
	    boot->bpbSecPerClust;

	if (boot->flags&FAT32)
		boot->ClustMask = CLUST32_MASK;
	else if (boot->NumClusters < (CLUST_RSRVD&CLUST12_MASK))
		boot->ClustMask = CLUST12_MASK;
	else if (boot->NumClusters < (CLUST_RSRVD&CLUST16_MASK))
		boot->ClustMask = CLUST16_MASK;
	else {
		pfatal("Filesystem too big (%u clusters) for non-FAT32 partition",
		       boot->NumClusters);
		return FSFATAL;
	}

	switch (boot->ClustMask) {
	case CLUST32_MASK:
		boot->NumFatEntries = (boot->FATsecs * boot->bpbBytesPerSec) / 4;
		break;
	case CLUST16_MASK:
		boot->NumFatEntries = (boot->FATsecs * boot->bpbBytesPerSec) / 2;
		break;
	default:
		boot->NumFatEntries = (boot->FATsecs * boot->bpbBytesPerSec * 2) / 3;
		break;
	}

	if (boot->NumFatEntries < boot->NumClusters) {
		pfatal("FAT size too small, %u entries won't fit into %u sectors\n",
		       boot->NumClusters, boot->FATsecs);
		return FSFATAL;
	}
	boot->ClusterSize = boot->bpbBytesPerSec * boot->bpbSecPerClust;

	boot->NumFiles = 1;
	boot->NumFree = 0;

	return ret;
}

int
writefsinfo(int dosfs, struct bootblock *boot)
{
	u_char fsinfo[2 * DOSBOOTBLOCKSIZE];

	if (lseek(dosfs, boot->bpbFSInfo * boot->bpbBytesPerSec, SEEK_SET)
	    != boot->bpbFSInfo * boot->bpbBytesPerSec
	    || read(dosfs, fsinfo, sizeof fsinfo) != sizeof fsinfo) {
		perr("could not read fsinfo block");
		return FSFATAL;
	}
	fsinfo[0x1e8] = (u_char)boot->FSFree;
	fsinfo[0x1e9] = (u_char)(boot->FSFree >> 8);
	fsinfo[0x1ea] = (u_char)(boot->FSFree >> 16);
	fsinfo[0x1eb] = (u_char)(boot->FSFree >> 24);
	fsinfo[0x1ec] = (u_char)boot->FSNext;
	fsinfo[0x1ed] = (u_char)(boot->FSNext >> 8);
	fsinfo[0x1ee] = (u_char)(boot->FSNext >> 16);
	fsinfo[0x1ef] = (u_char)(boot->FSNext >> 24);
	if (lseek(dosfs, boot->bpbFSInfo * boot->bpbBytesPerSec, SEEK_SET)
	    != boot->bpbFSInfo * boot->bpbBytesPerSec
	    || write(dosfs, fsinfo, sizeof fsinfo)
	    != sizeof fsinfo) {
		perr("Unable to write bpbFSInfo");
		return FSFATAL;
	}
	/*
	 * Technically, we should return FSBOOTMOD here.
	 *
	 * However, since Win95 OSR2 (the first M$ OS that has
	 * support for FAT32) doesn't maintain the FSINFO block
	 * correctly, it has to be fixed pretty often.
	 *
	 * Therefor, we handle the FSINFO block only informally,
	 * fixing it if necessary, but otherwise ignoring the
	 * fact that it was incorrect.
	 */
	return 0;
}