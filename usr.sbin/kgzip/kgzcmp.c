
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

#define	_KERNEL
#include <sys/param.h>
#undef _KERNEL
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <a.out.h>

#include "aouthdr.h"
#include "elfhdr.h"
#include "kgzip.h"

static void mk_data(const struct iodesc *i, const struct iodesc *,
		    struct kgz_hdr *, size_t);
static int ld_elf(const struct iodesc *, const struct iodesc *,
		  struct kgz_hdr *, const Elf32_Ehdr *);
static int ld_aout(const struct iodesc *, const struct iodesc *,
		   struct kgz_hdr *, const struct exec *);

/*
 * Compress executable and output it in relocatable object format.
 */
void
kgzcmp(struct kgz_hdr *kh, const char *f1, const char *f2)
{
    struct iodesc idi, ido;
    struct kgz_hdr khle;

    if ((idi.fd = open(idi.fname = f1, O_RDONLY)) == -1)
	err(1, "%s", idi.fname);
    if ((ido.fd = open(ido.fname = f2, O_CREAT | O_TRUNC | O_WRONLY,
		       0666)) == -1)
	err(1, "%s", ido.fname);
    kh->ident[0] = KGZ_ID0;
    kh->ident[1] = KGZ_ID1;
    kh->ident[2] = KGZ_ID2;
    kh->ident[3] = KGZ_ID3;
    mk_data(&idi, &ido, kh,
	    (format == F_AOUT ? sizeof(struct kgz_aouthdr0) :
				sizeof(struct kgz_elfhdr)) +
	     sizeof(struct kgz_hdr));
    kh->dload &= 0xffffff;
    kh->entry &= 0xffffff;
    if (format == F_AOUT) {
	struct kgz_aouthdr0 ahdr0 = aouthdr0;
	struct kgz_aouthdr1 ahdr1 = aouthdr1;
	unsigned x = (sizeof(struct kgz_hdr) + kh->nsize) & (16 - 1);
	if (x) {
	    x = 16 - x;
	    xzero(&ido, x);
	}
	xwrite(&ido, &ahdr1, sizeof(ahdr1));
	ahdr0.a.a_data += kh->nsize + x;
	xseek(&ido, 0);
	xwrite(&ido, &ahdr0, sizeof(ahdr0));
    } else {
	struct kgz_elfhdr ehdr = elfhdr;
	ehdr.st[KGZ_ST_KGZ_NDATA].st_size = htole32(kh->nsize);
	ehdr.sh[KGZ_SH_DATA].sh_size =
	    htole32(le32toh(ehdr.sh[KGZ_SH_DATA].sh_size) + kh->nsize);
	xseek(&ido, 0);
	xwrite(&ido, &ehdr, sizeof(ehdr));
    }
    khle = *kh;
    khle.dload = htole32(khle.dload);
    khle.dsize = htole32(khle.dsize);
    khle.isize = htole32(khle.isize);
    khle.entry = htole32(khle.entry);
    khle.nsize = htole32(khle.nsize);
    xwrite(&ido, &khle, sizeof(khle));
    xclose(&ido);
    xclose(&idi);
}

/*
 * Make encoded (compressed) data.
 */
static void
mk_data(const struct iodesc * idi, const struct iodesc * ido,
	struct kgz_hdr * kh, size_t off)
{
    union {
	struct exec ex;
	Elf32_Ehdr ee;
    } hdr;
    struct stat sb;
    struct iodesc idp;
    int fd[2];
    pid_t pid;
    size_t n;
    int fmt, status, e;

    n = xread(idi, &hdr, sizeof(hdr), 0);
    fmt = 0;
    if (n >= sizeof(hdr.ee) && IS_ELF(hdr.ee))
	fmt = F_ELF;
    else if (n >= sizeof(hdr.ex) && N_GETMAGIC(hdr.ex) == ZMAGIC)
	fmt = F_AOUT;
    if (!fmt)
	errx(1, "%s: Format not supported", idi->fname);
    xseek(ido, off);
    if (pipe(fd))
	err(1, NULL);
    switch (pid = fork()) {
    case -1:
	err(1, NULL);
    case 0:
	close(fd[1]);
	dup2(fd[0], STDIN_FILENO);
	close(fd[0]);
	close(idi->fd);
	dup2(ido->fd, STDOUT_FILENO);
	close(ido->fd);
	execlp("gzip", "gzip", "-9n", (char *)NULL);
	warn(NULL);
	_exit(1);
    default:
	close(fd[0]);
	idp.fname = "(pipe)";
	idp.fd = fd[1];
	e = fmt == F_ELF  ? ld_elf(idi, &idp, kh, &hdr.ee) :
	    fmt == F_AOUT ? ld_aout(idi, &idp, kh, &hdr.ex) : -1;
	close(fd[1]);
	if ((pid = waitpid(pid, &status, 0)) == -1)
	    err(1, NULL);
	if (WIFSIGNALED(status) || WEXITSTATUS(status))
	    exit(1);
    }
    if (e)
	errx(1, "%s: Invalid format", idi->fname);
    if (fstat(ido->fd, &sb))
	err(1, "%s", ido->fname);
    kh->nsize = sb.st_size - off;
}

/*
 * "Load" an ELF-format executable.
 */
static int
ld_elf(const struct iodesc * idi, const struct iodesc * ido,
       struct kgz_hdr * kh, const Elf32_Ehdr * e)
{
    Elf32_Phdr p;
    size_t load, addr, n;
    unsigned x, i;

    load = addr = n = 0;
    for (x = i = 0; i < e->e_phnum; i++) {
	if (xread(idi, &p, sizeof(p),
		  e->e_phoff + i * e->e_phentsize) != e->e_phentsize)
	    return -1;
	if (p.p_type != PT_LOAD)
	    continue;
	if (!x)
	    load = addr = p.p_vaddr;
	else {
	    if (p.p_vaddr < addr)
		return -1;
	    n = p.p_vaddr - addr;
	    if (n) {
		xzero(ido, n);
		addr += n;
	    }
	}
	if (p.p_memsz < p.p_filesz)
	    return -1;
	n = p.p_memsz - p.p_filesz;
	xcopy(idi, ido, p.p_filesz, p.p_offset);
	addr += p.p_filesz;
	x++;
    }
    if (!x)
	return -1;
    kh->dload = load;
    kh->dsize = addr - load;
    kh->isize = kh->dsize + n;
    kh->entry = e->e_entry;
    return 0;
}

/*
 * "Load" an a.out-format executable.
 */
static int
ld_aout(const struct iodesc * idi, const struct iodesc * ido,
	struct kgz_hdr * kh, const struct exec * a)
{
    size_t load, addr;

    load = addr = N_TXTADDR(*a);
    xcopy(idi, ido, le32toh(a->a_text), N_TXTOFF(*a));
    addr += le32toh(a->a_text);
    if (N_DATADDR(*a) != addr)
	return -1;
    xcopy(idi, ido, le32toh(a->a_data), N_DATOFF(*a));
    addr += le32toh(a->a_data);
    kh->dload = load;
    kh->dsize = addr - load;
    kh->isize = kh->dsize + le32toh(a->a_bss);
    kh->entry = le32toh(a->a_entry);
    return 0;
}