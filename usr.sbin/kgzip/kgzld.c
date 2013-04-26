
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
#include <sys/endian.h>
#include <sys/wait.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aouthdr.h"
#include "elfhdr.h"
#include "kgzip.h"

#define align(x, y) (((x) + (y) - 1) & ~((y) - 1))

/*
 * Link KGZ file and loader.
 */
void
kgzld(struct kgz_hdr * kh, const char *f1, const char *f2)
{
    char addr[16];
    struct iodesc idi;
    pid_t pid;
    size_t n;
    int status;

    if (strcmp(kh->ident, "KGZ")) {
	if ((idi.fd = open(idi.fname = f1, O_RDONLY)) == -1)
	    err(1, "%s", idi.fname);
	if (!format) {
	    union {
		struct exec ex;
		Elf32_Ehdr ee;
	    } hdr;
	    n = xread(&idi, &hdr, sizeof(hdr), 0);
	    if (n >= sizeof(hdr.ee) && IS_ELF(hdr.ee))
		format = F_ELF;
	    else if (n >= sizeof(hdr.ex) &&
		     N_GETMAGIC(hdr.ex) == OMAGIC)
		format = F_AOUT;
	    if (!format)
		errx(1, "%s: Format not supported", idi.fname);
	}
	n = xread(&idi, kh, sizeof(*kh),
		  format == F_AOUT ? sizeof(struct kgz_aouthdr0)
				   : sizeof(struct kgz_elfhdr));
	xclose(&idi);
	if (n != sizeof(*kh) || strcmp(kh->ident, "KGZ"))
	    errx(1, "%s: Invalid format", idi.fname);
    }
    sprintf(addr, "%#x", align(kh->dload + kh->dsize, 0x1000));
    switch (pid = fork()) {
    case -1:
	err(1, NULL);
    case 0:
	if (format == F_AOUT)
	    execlp("ld", "ld", "-aout", "-Z", "-T", addr, "-o", f2,
		   loader, f1, (char *)NULL);
	else
	    execlp("ld", "ld", "-Ttext", addr, "-o", f2, loader, f1,
		   (char *)NULL);
	warn(NULL);
	_exit(1);
    default:
	if ((pid = waitpid(pid, &status, 0)) == -1)
	    err(1, NULL);
	if (WIFSIGNALED(status) || WEXITSTATUS(status))
	    exit(1);
    }
}