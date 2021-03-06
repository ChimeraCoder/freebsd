
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/sysmacros.h>
#include <sys/modctl.h>
#include <sys/debug.h>
#include <sys/mman.h>
#include <sys/modctl.h>
#include <sys/kobj.h>
#include <ctf_impl.h>

int ctf_leave_compressed = 0;

static struct modlmisc modlmisc = {
	&mod_miscops, "Compact C Type Format routines"
};

static struct modlinkage modlinkage = {
	MODREV_1, &modlmisc, NULL
};

int
_init(void)
{
	return (mod_install(&modlinkage));
}

int
_info(struct modinfo *mip)
{
	return (mod_info(&modlinkage, mip));
}

int
_fini(void)
{
	return (mod_remove(&modlinkage));
}

/*ARGSUSED*/
void *
ctf_zopen(int *errp)
{
	return ((void *)1); /* zmod is always loaded because we depend on it */
}

/*ARGSUSED*/
const void *
ctf_sect_mmap(ctf_sect_t *sp, int fd)
{
	return (MAP_FAILED); /* we don't support this in the kernel */
}

/*ARGSUSED*/
void
ctf_sect_munmap(const ctf_sect_t *sp)
{
	/* we don't support this in the kernel */
}

/*ARGSUSED*/
ctf_file_t *
ctf_fdopen(int fd, int *errp)
{
	return (ctf_set_open_errno(errp, ENOTSUP));
}

/*ARGSUSED*/
ctf_file_t *
ctf_open(const char *filename, int *errp)
{
	return (ctf_set_open_errno(errp, ENOTSUP));
}

/*ARGSUSED*/
int
ctf_write(ctf_file_t *fp, int fd)
{
	return (ctf_set_errno(fp, ENOTSUP));
}

int
ctf_version(int version)
{
	ASSERT(version > 0 && version <= CTF_VERSION);

	if (version > 0)
		_libctf_version = MIN(CTF_VERSION, version);

	return (_libctf_version);
}

/*ARGSUSED*/
ctf_file_t *
ctf_modopen(struct module *mp, int *error)
{
	ctf_sect_t ctfsect, symsect, strsect;
	ctf_file_t *fp = NULL;
	int err;

	if (error == NULL)
		error = &err;

	ctfsect.cts_name = ".SUNW_ctf";
	ctfsect.cts_type = SHT_PROGBITS;
	ctfsect.cts_flags = SHF_ALLOC;
	ctfsect.cts_data = mp->ctfdata;
	ctfsect.cts_size = mp->ctfsize;
	ctfsect.cts_entsize = 1;
	ctfsect.cts_offset = 0;

	symsect.cts_name = ".symtab";
	symsect.cts_type = SHT_SYMTAB;
	symsect.cts_flags = 0;
	symsect.cts_data = mp->symtbl;
	symsect.cts_size = mp->symhdr->sh_size;
#ifdef _LP64
	symsect.cts_entsize = sizeof (Elf64_Sym);
#else
	symsect.cts_entsize = sizeof (Elf32_Sym);
#endif
	symsect.cts_offset = 0;

	strsect.cts_name = ".strtab";
	strsect.cts_type = SHT_STRTAB;
	strsect.cts_flags = 0;
	strsect.cts_data = mp->strings;
	strsect.cts_size = mp->strhdr->sh_size;
	strsect.cts_entsize = 1;
	strsect.cts_offset = 0;

	ASSERT(MUTEX_HELD(&mod_lock));

	if ((fp = ctf_bufopen(&ctfsect, &symsect, &strsect, error)) == NULL)
		return (NULL);

	if (!ctf_leave_compressed && (caddr_t)fp->ctf_base != mp->ctfdata) {
		/*
		 * We must have just uncompressed the CTF data.  To avoid
		 * others having to pay the (substantial) cost of decompressing
		 * the data, we're going to substitute the uncompressed version
		 * for the compressed version.  Note that this implies that the
		 * first CTF consumer will induce memory impact on the system
		 * (but in the name of performance of future CTF consumers).
		 */
		kobj_set_ctf(mp, (caddr_t)fp->ctf_base, fp->ctf_size);
		fp->ctf_data.cts_data = fp->ctf_base;
		fp->ctf_data.cts_size = fp->ctf_size;
	}

	return (fp);
}