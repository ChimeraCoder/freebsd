
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

/* $Id: xmmap.c,v 1.15 2009/02/16 04:21:40 djm Exp $ */

#include "includes.h"

#include <sys/types.h>
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

void *
xmmap(size_t size)
{
#ifdef HAVE_MMAP
	void *address;

# ifdef MAP_ANON
	address = mmap(NULL, size, PROT_WRITE|PROT_READ, MAP_ANON|MAP_SHARED,
	    -1, (off_t)0);
# else
	address = mmap(NULL, size, PROT_WRITE|PROT_READ, MAP_SHARED,
	    open("/dev/zero", O_RDWR), (off_t)0);
# endif

#define MM_SWAP_TEMPLATE "/var/run/sshd.mm.XXXXXXXX"
	if (address == (void *)MAP_FAILED) {
		char tmpname[sizeof(MM_SWAP_TEMPLATE)] = MM_SWAP_TEMPLATE;
		int tmpfd;
		mode_t old_umask;

		old_umask = umask(0177);
		tmpfd = mkstemp(tmpname);
		umask(old_umask);
		if (tmpfd == -1)
			fatal("mkstemp(\"%s\"): %s",
			    MM_SWAP_TEMPLATE, strerror(errno));
		unlink(tmpname);
		if (ftruncate(tmpfd, size) != 0)
			fatal("%s: ftruncate: %s", __func__, strerror(errno));
		address = mmap(NULL, size, PROT_WRITE|PROT_READ, MAP_SHARED,
		    tmpfd, (off_t)0);
		close(tmpfd);
	}

	return (address);
#else
	fatal("%s: UsePrivilegeSeparation=yes and Compression=yes not supported",
	    __func__);
#endif /* HAVE_MMAP */

}