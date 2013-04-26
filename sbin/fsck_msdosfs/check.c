
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
__RCSID("$NetBSD: check.c,v 1.14 2006/06/05 16:51:18 christos Exp $");
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "ext.h"
#include "fsutil.h"

int
checkfilesys(const char *fname)
{
	int dosfs;
	struct bootblock boot;
	struct fatEntry *fat = NULL;
	int finish_dosdirsection=0;
	u_int i;
	int mod = 0;
	int ret = 8;

	rdonly = alwaysno;
	if (!preen)
		printf("** %s", fname);

	dosfs = open(fname, rdonly ? O_RDONLY : O_RDWR, 0);
	if (dosfs < 0 && !rdonly) {
		dosfs = open(fname, O_RDONLY, 0);
		if (dosfs >= 0)
			pwarn(" (NO WRITE)\n");
		else if (!preen)
			printf("\n");
		rdonly = 1;
	} else if (!preen)
		printf("\n");

	if (dosfs < 0) {
		perr("Can't open `%s'", fname);
		printf("\n");
		return 8;
	}

	if (readboot(dosfs, &boot) != FSOK) {
		close(dosfs);
		printf("\n");
		return 8;
	}

	if (skipclean && preen && checkdirty(dosfs, &boot)) {
		printf("%s: ", fname);
		printf("FILESYSTEM CLEAN; SKIPPING CHECKS\n");
		ret = 0;
		goto out;
	}

	if (!preen)  {
		if (boot.ValidFat < 0)
			printf("** Phase 1 - Read and Compare FATs\n");
		else
			printf("** Phase 1 - Read FAT\n");
	}

	mod |= readfat(dosfs, &boot, boot.ValidFat >= 0 ? boot.ValidFat : 0, &fat);
	if (mod & FSFATAL) {
		close(dosfs);
		return 8;
	}

	if (boot.ValidFat < 0)
		for (i = 1; i < boot.bpbFATs; i++) {
			struct fatEntry *currentFat;

			mod |= readfat(dosfs, &boot, i, &currentFat);

			if (mod & FSFATAL)
				goto out;

			mod |= comparefat(&boot, fat, currentFat, i);
			free(currentFat);
			if (mod & FSFATAL)
				goto out;
		}

	if (!preen)
		printf("** Phase 2 - Check Cluster Chains\n");

	mod |= checkfat(&boot, fat);
	if (mod & FSFATAL)
		goto out;
	/* delay writing FATs */

	if (!preen)
		printf("** Phase 3 - Checking Directories\n");

	mod |= resetDosDirSection(&boot, fat);
	finish_dosdirsection = 1;
	if (mod & FSFATAL)
		goto out;
	/* delay writing FATs */

	mod |= handleDirTree(dosfs, &boot, fat);
	if (mod & FSFATAL)
		goto out;

	if (!preen)
		printf("** Phase 4 - Checking for Lost Files\n");

	mod |= checklost(dosfs, &boot, fat);
	if (mod & FSFATAL)
		goto out;

	/* now write the FATs */
	if (mod & FSFATMOD) {
		if (ask(1, "Update FATs")) {
			mod |= writefat(dosfs, &boot, fat, mod & FSFIXFAT);
			if (mod & FSFATAL)
				goto out;
		} else
			mod |= FSERROR;
	}

	if (boot.NumBad)
		pwarn("%d files, %d free (%d clusters), %d bad (%d clusters)\n",
		      boot.NumFiles,
		      boot.NumFree * boot.ClusterSize / 1024, boot.NumFree,
		      boot.NumBad * boot.ClusterSize / 1024, boot.NumBad);
	else
		pwarn("%d files, %d free (%d clusters)\n",
		      boot.NumFiles,
		      boot.NumFree * boot.ClusterSize / 1024, boot.NumFree);

	if (mod && (mod & FSERROR) == 0) {
		if (mod & FSDIRTY) {
			if (ask(1, "MARK FILE SYSTEM CLEAN") == 0)
				mod &= ~FSDIRTY;

			if (mod & FSDIRTY) {
				pwarn("MARKING FILE SYSTEM CLEAN\n");
				mod |= writefat(dosfs, &boot, fat, 1);
			} else {
				pwarn("\n***** FILE SYSTEM IS LEFT MARKED AS DIRTY *****\n");
				mod |= FSERROR; /* file system not clean */
			}
		}
	}

	if (mod & (FSFATAL | FSERROR))
		goto out;

	ret = 0;

    out:
	if (finish_dosdirsection)
		finishDosDirSection();
	free(fat);
	close(dosfs);

	if (mod & (FSFATMOD|FSDIRMOD))
		pwarn("\n***** FILE SYSTEM WAS MODIFIED *****\n");

	return ret;
}