
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

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/extattr.h>
#include <sys/param.h>
#include <sys/mount.h>

#include <ufs/ufs/extattr.h>

#include <errno.h>
#include <fcntl.h>
#include <libutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int initattr(int argc, char *argv[]);
int showattr(int argc, char *argv[]);
long num_inodes_by_path(char *path);
void usage(void);

void
usage(void)
{

	fprintf(stderr,
	    "usage:\n"
	    "  extattrctl start path\n"
	    "  extattrctl stop path\n"
	    "  extattrctl initattr [-f] [-p path] attrsize attrfile\n"
	    "  extattrctl showattr attrfile\n"
	    "  extattrctl enable path attrnamespace attrname attrfile\n"
	    "  extattrctl disable path attrnamespace attrname\n");
	exit(-1);
}

long
num_inodes_by_path(char *path)
{
	struct statfs	buf;
	int	error;

	error = statfs(path, &buf);
	if (error) {
		perror("statfs");
		return (-1);
	}

	return (buf.f_files);
}

static const char zero_buf[8192];

int
initattr(int argc, char *argv[])
{
	struct ufs_extattr_fileheader	uef;
	char	*fs_path = NULL;
	int	ch, i, error, flags;
	ssize_t	wlen;
	size_t	easize;

	flags = O_CREAT | O_WRONLY | O_TRUNC | O_EXCL;
	optind = 0;
	while ((ch = getopt(argc, argv, "fp:r:w:")) != -1)
		switch (ch) {
		case 'f':
			flags &= ~O_EXCL;
			break;
		case 'p':
			fs_path = optarg;
			break;
		case '?':
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	if (argc != 2)
		usage();

	error = 0;
	if ((i = open(argv[1], flags, 0600)) == -1) {
		/* unable to open file */
		perror(argv[1]);
		return (-1);
	}
	uef.uef_magic = UFS_EXTATTR_MAGIC;
	uef.uef_version = UFS_EXTATTR_VERSION;
	uef.uef_size = atoi(argv[0]);
	if (write(i, &uef, sizeof(uef)) == -1)
		error = -1;
	else if (fs_path != NULL) {
		easize = (sizeof uef + uef.uef_size) *
		    num_inodes_by_path(fs_path);
		while (easize > 0) {
			if (easize > sizeof zero_buf)
				wlen = write(i, zero_buf, sizeof zero_buf);
			else
				wlen = write(i, zero_buf, easize);
			if (wlen == -1) {
				error = -1;
				break;
			}
			easize -= wlen;
		}
	}
	if (error == -1) {
		perror(argv[1]);
		unlink(argv[1]);
		close(i);
		return (-1);
	}

	close(i);
	return (0);
}

int
showattr(int argc, char *argv[])
{
	struct ufs_extattr_fileheader	uef;
	int i, fd;

	if (argc != 1)
		usage();

	fd = open(argv[0], O_RDONLY);
	if (fd == -1) {
		perror(argv[0]);
		return (-1);
	}

	i = read(fd, &uef, sizeof(uef));
	if (i == -1) {
		perror(argv[0]);
		close(fd);
		return (-1);
	}
	if (i != sizeof(uef)) {
		fprintf(stderr, "%s: invalid file header\n", argv[0]);
		close(fd);
		return (-1);
	}

	if (uef.uef_magic != UFS_EXTATTR_MAGIC) {
		fprintf(stderr, "%s: bad magic\n", argv[0]);
		close(fd);
		return (-1);
	}

	printf("%s: version %d, size %d\n", argv[0], uef.uef_version,
	    uef.uef_size);

	close(fd);
	return (0);
}

int
main(int argc, char *argv[])
{
	int	error = 0, attrnamespace;

	if (argc < 2)
		usage();

	if (!strcmp(argv[1], "start")) {
		if (argc != 3)
			usage();
		error = extattrctl(argv[2], UFS_EXTATTR_CMD_START, NULL, 0,
		    NULL);
		if (error) {
			perror("extattrctl start");
			return (-1);
		}
	} else if (!strcmp(argv[1], "stop")) {
		if (argc != 3)
			usage();
		error = extattrctl(argv[2], UFS_EXTATTR_CMD_STOP, NULL, 0,
		   NULL);
		if (error) {
			perror("extattrctl stop");
			return (-1);
		}
	} else if (!strcmp(argv[1], "enable")) {
		if (argc != 6)
			usage();
		error = extattr_string_to_namespace(argv[3], &attrnamespace);
		if (error) {
			perror("extattrctl enable");
			return (-1);
		}
		error = extattrctl(argv[2], UFS_EXTATTR_CMD_ENABLE, argv[5],
		    attrnamespace, argv[4]);
		if (error) {
			perror("extattrctl enable");
			return (-1);
		}
	} else if (!strcmp(argv[1], "disable")) {
		if (argc != 5)
			usage();
		error = extattr_string_to_namespace(argv[3], &attrnamespace);
		if (error) {
			perror("extattrctl disable");
			return (-1);
		}
		error = extattrctl(argv[2], UFS_EXTATTR_CMD_DISABLE, NULL,
		    attrnamespace, argv[4]);
		if (error) {
			perror("extattrctl disable");
			return (-1);
		}
	} else if (!strcmp(argv[1], "initattr")) {
		argc -= 2;
		argv += 2;
		error = initattr(argc, argv);
		if (error)
			return (-1);
	} else if (!strcmp(argv[1], "showattr")) {
		argc -= 2;
		argv += 2;
		error = showattr(argc, argv);
		if (error)
			return (-1);
	} else
		usage();

	return (0);
}