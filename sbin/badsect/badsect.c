
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
"@(#) Copyright (c) 1981, 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static const char sccsid[] = "@(#)badsect.c	8.1 (Berkeley) 6/5/93";
#endif
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * badsect
 *
 * Badsect takes a list of file-system relative sector numbers
 * and makes files containing the blocks of which these sectors are a part.
 * It can be used to contain sectors which have problems if these sectors
 * are not part of the bad file for the pack (see bad144).  For instance,
 * this program can be used if the driver for the file system in question
 * does not support bad block forwarding.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/disklabel.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <err.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <libufs.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define sblock	disk.d_fs
#define	acg	disk.d_cg
static struct	uufsd disk;
static struct	fs *fs = &sblock;
static int	errs;

int	chkuse(daddr_t, int);

static void
usage(void)
{
	fprintf(stderr, "usage: badsect bbdir blkno ...\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	daddr_t diskbn;
	daddr_t number;
	struct stat stbuf, devstat;
	struct dirent *dp;
	DIR *dirp;
	char name[2 * MAXPATHLEN];
	char *name_dir_end;

	if (argc < 3)
		usage();
	if (chdir(argv[1]) < 0 || stat(".", &stbuf) < 0)
		err(2, "%s", argv[1]);
	strcpy(name, _PATH_DEV);
	if ((dirp = opendir(name)) == NULL)
		err(3, "%s", name);
	name_dir_end = name + strlen(name);
	while ((dp = readdir(dirp)) != NULL) {
		strcpy(name_dir_end, dp->d_name);
		if (lstat(name, &devstat) < 0)
			err(4, "%s", name);
		if (stbuf.st_dev == devstat.st_rdev &&
		    (devstat.st_mode & IFMT) == IFCHR)
			break;
	}
	closedir(dirp);
	if (dp == NULL) {
		printf("Cannot find dev 0%lo corresponding to %s\n",
		    (u_long)stbuf.st_rdev, argv[1]);
		exit(5);
	}
	if (ufs_disk_fillout(&disk, name) == -1) {
		if (disk.d_error != NULL)
			errx(6, "%s: %s", name, disk.d_error);
		else
			err(7, "%s", name);
	}
	for (argc -= 2, argv += 2; argc > 0; argc--, argv++) {
		number = strtol(*argv, NULL, 0);
		if (errno == EINVAL || errno == ERANGE)
			err(8, "%s", *argv);
		if (chkuse(number, 1))
			continue;
		/*
		 * Print a warning if converting the block number to a dev_t
		 * will truncate it.  badsect was not very useful in versions
		 * of BSD before 4.4 because dev_t was 16 bits and another
		 * bit was lost by bogus sign extensions.
		 */
		diskbn = dbtofsb(fs, number);
		if ((dev_t)diskbn != diskbn) {
			printf("sector %ld cannot be represented as a dev_t\n",
			    (long)number);
			errs++;
		}
		else if (mknod(*argv, IFMT|0600, (dev_t)diskbn) < 0) {
			warn("%s", *argv);
			errs++;
		}
	}
	ufs_disk_close(&disk);
	printf("Don't forget to run ``fsck %s''\n", name);
	exit(errs);
}

int
chkuse(daddr_t blkno, int cnt)
{
	int cg;
	daddr_t fsbn, bn;

	fsbn = dbtofsb(fs, blkno);
	if ((unsigned)(fsbn+cnt) > fs->fs_size) {
		printf("block %ld out of range of file system\n", (long)blkno);
		return (1);
	}
	cg = dtog(fs, fsbn);
	if (fsbn < cgdmin(fs, cg)) {
		if (cg == 0 || (fsbn+cnt) > cgsblock(fs, cg)) {
			printf("block %ld in non-data area: cannot attach\n",
			    (long)blkno);
			return (1);
		}
	} else {
		if ((fsbn+cnt) > cgbase(fs, cg+1)) {
			printf("block %ld in non-data area: cannot attach\n",
			    (long)blkno);
			return (1);
		}
	}
	if (cgread1(&disk, cg) != 1) {
		fprintf(stderr, "cg %d: could not be read\n", cg);
		errs++;
		return (1);
	}
	if (!cg_chkmagic(&acg)) {
		fprintf(stderr, "cg %d: bad magic number\n", cg);
		errs++;
		return (1);
	}
	bn = dtogd(fs, fsbn);
	if (isclr(cg_blksfree(&acg), bn))
		printf("Warning: sector %ld is in use\n", (long)blkno);
	return (0);
}