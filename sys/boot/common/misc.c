
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

#include <string.h>
#include <stand.h>
#include <bootstrap.h>

/*
 * Concatenate the (argc) elements of (argv) into a single string, and return
 * a copy of same.
 */
char *
unargv(int argc, char *argv[])
{
    size_t	hlong;
    int		i;
    char	*cp;

    for (i = 0, hlong = 0; i < argc; i++)
	hlong += strlen(argv[i]) + 2;

    if(hlong == 0)
	return(NULL);

    cp = malloc(hlong);
    cp[0] = 0;
    for (i = 0; i < argc; i++) {
	strcat(cp, argv[i]);
	if (i < (argc - 1))
	  strcat(cp, " ");
    }
	  
    return(cp);
}

/*
 * Get the length of a string in kernel space
 */
size_t
strlenout(vm_offset_t src)
{
    char	c;
    size_t	len;
    
    for (len = 0; ; len++) {
	archsw.arch_copyout(src++, &c, 1);
	if (c == 0)
	    break;
    }
    return(len);
}

/*
 * Make a duplicate copy of a string in kernel space
 */
char *
strdupout(vm_offset_t str)
{
    char	*result, *cp;
    
    result = malloc(strlenout(str) + 1);
    for (cp = result; ;cp++) {
	archsw.arch_copyout(str++, cp, 1);
	if (*cp == 0)
	    break;
    }
    return(result);
}

/* Zero a region in kernel space. */
void
kern_bzero(vm_offset_t dest, size_t len)
{
	char buf[256];
	size_t chunk, resid;

	bzero(buf, sizeof(buf));
	resid = len;
	while (resid > 0) {
		chunk = min(sizeof(buf), resid);
		archsw.arch_copyin(buf, dest, chunk);
		resid -= chunk;
		dest += chunk;
	}
}

/*
 * Read the specified part of a file to kernel space.  Unlike regular
 * pread, the file pointer is advanced to the end of the read data,
 * and it just returns 0 if successful.
 */
int
kern_pread(int fd, vm_offset_t dest, size_t len, off_t off)
{
	ssize_t nread;

	if (lseek(fd, off, SEEK_SET) == -1) {
		printf("\nlseek failed\n");
		return (-1);
	}
	nread = archsw.arch_readin(fd, dest, len);
	if (nread != len) {
		printf("\nreadin failed\n");
		return (-1);
	}
	return (0);
}

/*
 * Read the specified part of a file to a malloced buffer.  The file
 * pointer is advanced to the end of the read data.
 */
void *
alloc_pread(int fd, off_t off, size_t len)
{
	void *buf;
	ssize_t nread;

	buf = malloc(len);
	if (buf == NULL) {
		printf("\nmalloc(%d) failed\n", (int)len);
		return (NULL);
	}
	if (lseek(fd, off, SEEK_SET) == -1) {
		printf("\nlseek failed\n");
		free(buf);
		return (NULL);
	}
	nread = read(fd, buf, len);
	if (nread != len) {
		printf("\nread failed\n");
		free(buf);
		return (NULL);
	}
	return (buf);
}

/*
 * Display a region in traditional hexdump format.
 */
void
hexdump(caddr_t region, size_t len)
{
    caddr_t	line;
    int		x, c;
    char	lbuf[80];
#define emit(fmt, args...)	{sprintf(lbuf, fmt , ## args); pager_output(lbuf);}

    pager_open();
    for (line = region; line < (region + len); line += 16) {
	emit("%08lx  ", (long) line);
	
	for (x = 0; x < 16; x++) {
	    if ((line + x) < (region + len)) {
		emit("%02x ", *(u_int8_t *)(line + x));
	    } else {
		emit("-- ");
	    }
	    if (x == 7)
		emit(" ");
	}
	emit(" |");
	for (x = 0; x < 16; x++) {
	    if ((line + x) < (region + len)) {
		c = *(u_int8_t *)(line + x);
		if ((c < ' ') || (c > '~'))	/* !isprint(c) */
		    c = '.';
		emit("%c", c);
	    } else {
		emit(" ");
	    }
	}
	emit("|\n");
    }
    pager_close();
}

void
dev_cleanup(void)
{
    int		i;

    /* Call cleanup routines */
    for (i = 0; devsw[i] != NULL; ++i)
	if (devsw[i]->dv_cleanup != NULL)
	    (devsw[i]->dv_cleanup)();
}