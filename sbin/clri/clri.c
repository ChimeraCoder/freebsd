
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

#if 0
#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1990, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)clri.c	8.2 (Berkeley) 9/23/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/disklabel.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*
 * Possible superblock locations ordered from most to least likely.
 */
static int sblock_try[] = SBLOCKSEARCH;

static void
usage(void)
{
	(void)fprintf(stderr, "usage: clri special_device inode_number ...\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct fs *sbp;
	struct ufs1_dinode *dp1;
	struct ufs2_dinode *dp2;
	char *ibuf[MAXBSIZE];
	long generation, bsize;
	off_t offset;
	int i, fd, inonum;
	char *fs, sblock[SBLOCKSIZE];
	void *v = ibuf;

	if (argc < 3)
		usage();

	fs = *++argv;
	sbp = NULL;

	/* get the superblock. */
	if ((fd = open(fs, O_RDWR, 0)) < 0)
		err(1, "%s", fs);
	for (i = 0; sblock_try[i] != -1; i++) {
		if (lseek(fd, (off_t)(sblock_try[i]), SEEK_SET) < 0)
			err(1, "%s", fs);
		if (read(fd, sblock, sizeof(sblock)) != sizeof(sblock))
			errx(1, "%s: can't read superblock", fs);
		sbp = (struct fs *)sblock;
		if ((sbp->fs_magic == FS_UFS1_MAGIC ||
		     (sbp->fs_magic == FS_UFS2_MAGIC &&
		      sbp->fs_sblockloc == sblock_try[i])) &&
		    sbp->fs_bsize <= MAXBSIZE &&
		    sbp->fs_bsize >= (int)sizeof(struct fs))
			break;
	}
	if (sblock_try[i] == -1)
		errx(2, "cannot find file system superblock");
	bsize = sbp->fs_bsize;

	/* remaining arguments are inode numbers. */
	while (*++argv) {
		/* get the inode number. */
		if ((inonum = atoi(*argv)) <= 0)
			errx(1, "%s is not a valid inode number", *argv);
		(void)printf("clearing %d\n", inonum);

		/* read in the appropriate block. */
		offset = ino_to_fsba(sbp, inonum);	/* inode to fs blk */
		offset = fsbtodb(sbp, offset);		/* fs blk disk blk */
		offset *= DEV_BSIZE;			/* disk blk to bytes */

		/* seek and read the block */
		if (lseek(fd, offset, SEEK_SET) < 0)
			err(1, "%s", fs);
		if (read(fd, ibuf, bsize) != bsize)
			err(1, "%s", fs);

		if (sbp->fs_magic == FS_UFS2_MAGIC) {
			/* get the inode within the block. */
			dp2 = &(((struct ufs2_dinode *)v)
			    [ino_to_fsbo(sbp, inonum)]);

			/* clear the inode, and bump the generation count. */
			generation = dp2->di_gen + 1;
			memset(dp2, 0, sizeof(*dp2));
			dp2->di_gen = generation;
		} else {
			/* get the inode within the block. */
			dp1 = &(((struct ufs1_dinode *)v)
			    [ino_to_fsbo(sbp, inonum)]);

			/* clear the inode, and bump the generation count. */
			generation = dp1->di_gen + 1;
			memset(dp1, 0, sizeof(*dp1));
			dp1->di_gen = generation;
		}

		/* backup and write the block */
		if (lseek(fd, (off_t)-bsize, SEEK_CUR) < 0)
			err(1, "%s", fs);
		if (write(fd, ibuf, bsize) != bsize)
			err(1, "%s", fs);
		(void)fsync(fd);
	}
	(void)close(fd);
	exit(0);
}