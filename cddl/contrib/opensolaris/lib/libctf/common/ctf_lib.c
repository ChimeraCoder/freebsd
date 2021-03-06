
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/zmod.h>
#include <ctf_impl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#if defined(sun)
#include <dlfcn.h>
#else
#include <zlib.h>
#endif
#include <gelf.h>

#if defined(sun)
#ifdef _LP64
static const char *_libctf_zlib = "/usr/lib/64/libz.so";
#else
static const char *_libctf_zlib = "/usr/lib/libz.so";
#endif
#endif

static struct {
	int (*z_uncompress)(uchar_t *, ulong_t *, const uchar_t *, ulong_t);
	const char *(*z_error)(int);
	void *z_dlp;
} zlib;

static size_t _PAGESIZE;
static size_t _PAGEMASK;

#if defined(sun)
#pragma init(_libctf_init)
#else
void    _libctf_init(void) __attribute__ ((constructor));
#endif
void
_libctf_init(void)
{
#if defined(sun)
	const char *p = getenv("LIBCTF_DECOMPRESSOR");

	if (p != NULL)
		_libctf_zlib = p; /* use alternate decompression library */
#endif

	_libctf_debug = getenv("LIBCTF_DEBUG") != NULL;

	_PAGESIZE = getpagesize();
	_PAGEMASK = ~(_PAGESIZE - 1);
}

/*
 * Attempt to dlopen the decompression library and locate the symbols of
 * interest that we will need to call.  This information in cached so
 * that multiple calls to ctf_bufopen() do not need to reopen the library.
 */
void *
ctf_zopen(int *errp)
{
#if defined(sun)
	ctf_dprintf("decompressing CTF data using %s\n", _libctf_zlib);

	if (zlib.z_dlp != NULL)
		return (zlib.z_dlp); /* library is already loaded */

	if (access(_libctf_zlib, R_OK) == -1)
		return (ctf_set_open_errno(errp, ECTF_ZMISSING));

	if ((zlib.z_dlp = dlopen(_libctf_zlib, RTLD_LAZY | RTLD_LOCAL)) == NULL)
		return (ctf_set_open_errno(errp, ECTF_ZINIT));

	zlib.z_uncompress = (int (*)(uchar_t *, ulong_t *, const uchar_t *, ulong_t)) dlsym(zlib.z_dlp, "uncompress");
	zlib.z_error = (const char *(*)(int)) dlsym(zlib.z_dlp, "zError");

	if (zlib.z_uncompress == NULL || zlib.z_error == NULL) {
		(void) dlclose(zlib.z_dlp);
		bzero(&zlib, sizeof (zlib));
		return (ctf_set_open_errno(errp, ECTF_ZINIT));
	}
#else
	zlib.z_uncompress = uncompress;
	zlib.z_error = zError;

	/* Dummy return variable as 'no error' */
	zlib.z_dlp = (void *) (uintptr_t) 1;
#endif

	return (zlib.z_dlp);
}

/*
 * The ctf_bufopen() routine calls these subroutines, defined by <sys/zmod.h>,
 * which we then patch through to the functions in the decompression library.
 */
int
z_uncompress(void *dst, size_t *dstlen, const void *src, size_t srclen)
{
	return (zlib.z_uncompress(dst, (ulong_t *)dstlen, src, srclen));
}

const char *
z_strerror(int err)
{
	return (zlib.z_error(err));
}

/*
 * Convert a 32-bit ELF file header into GElf.
 */
static void
ehdr_to_gelf(const Elf32_Ehdr *src, GElf_Ehdr *dst)
{
	bcopy(src->e_ident, dst->e_ident, EI_NIDENT);
	dst->e_type = src->e_type;
	dst->e_machine = src->e_machine;
	dst->e_version = src->e_version;
	dst->e_entry = (Elf64_Addr)src->e_entry;
	dst->e_phoff = (Elf64_Off)src->e_phoff;
	dst->e_shoff = (Elf64_Off)src->e_shoff;
	dst->e_flags = src->e_flags;
	dst->e_ehsize = src->e_ehsize;
	dst->e_phentsize = src->e_phentsize;
	dst->e_phnum = src->e_phnum;
	dst->e_shentsize = src->e_shentsize;
	dst->e_shnum = src->e_shnum;
	dst->e_shstrndx = src->e_shstrndx;
}

/*
 * Convert a 32-bit ELF section header into GElf.
 */
static void
shdr_to_gelf(const Elf32_Shdr *src, GElf_Shdr *dst)
{
	dst->sh_name = src->sh_name;
	dst->sh_type = src->sh_type;
	dst->sh_flags = src->sh_flags;
	dst->sh_addr = src->sh_addr;
	dst->sh_offset = src->sh_offset;
	dst->sh_size = src->sh_size;
	dst->sh_link = src->sh_link;
	dst->sh_info = src->sh_info;
	dst->sh_addralign = src->sh_addralign;
	dst->sh_entsize = src->sh_entsize;
}

/*
 * In order to mmap a section from the ELF file, we must round down sh_offset
 * to the previous page boundary, and mmap the surrounding page.  We store
 * the pointer to the start of the actual section data back into sp->cts_data.
 */
const void *
ctf_sect_mmap(ctf_sect_t *sp, int fd)
{
	size_t pageoff = sp->cts_offset & ~_PAGEMASK;

	caddr_t base = mmap64(NULL, sp->cts_size + pageoff, PROT_READ,
	    MAP_PRIVATE, fd, sp->cts_offset & _PAGEMASK);

	if (base != MAP_FAILED)
		sp->cts_data = base + pageoff;

	return (base);
}

/*
 * Since sp->cts_data has the adjusted offset, we have to again round down
 * to get the actual mmap address and round up to get the size.
 */
void
ctf_sect_munmap(const ctf_sect_t *sp)
{
	uintptr_t addr = (uintptr_t)sp->cts_data;
	uintptr_t pageoff = addr & ~_PAGEMASK;

	(void) munmap((void *)(addr - pageoff), sp->cts_size + pageoff);
}

/*
 * Open the specified file descriptor and return a pointer to a CTF container.
 * The file can be either an ELF file or raw CTF file.  The caller is
 * responsible for closing the file descriptor when it is no longer needed.
 */
ctf_file_t *
ctf_fdopen(int fd, int *errp)
{
	ctf_sect_t ctfsect, symsect, strsect;
	ctf_file_t *fp = NULL;

	struct stat64 st;
	ssize_t nbytes;

	union {
		ctf_preamble_t ctf;
		Elf32_Ehdr e32;
		GElf_Ehdr e64;
	} hdr;

	bzero(&ctfsect, sizeof (ctf_sect_t));
	bzero(&symsect, sizeof (ctf_sect_t));
	bzero(&strsect, sizeof (ctf_sect_t));
	bzero(&hdr.ctf, sizeof (hdr));

	if (fstat64(fd, &st) == -1)
		return (ctf_set_open_errno(errp, errno));

	if ((nbytes = pread64(fd, &hdr.ctf, sizeof (hdr), 0)) <= 0)
		return (ctf_set_open_errno(errp, nbytes < 0? errno : ECTF_FMT));

	/*
	 * If we have read enough bytes to form a CTF header and the magic
	 * string matches, attempt to interpret the file as raw CTF.
	 */
	if (nbytes >= (ssize_t) sizeof (ctf_preamble_t) &&
	    hdr.ctf.ctp_magic == CTF_MAGIC) {
		if (hdr.ctf.ctp_version > CTF_VERSION)
			return (ctf_set_open_errno(errp, ECTF_CTFVERS));

		ctfsect.cts_data = mmap64(NULL, st.st_size, PROT_READ,
		    MAP_PRIVATE, fd, 0);

		if (ctfsect.cts_data == MAP_FAILED)
			return (ctf_set_open_errno(errp, errno));

		ctfsect.cts_name = _CTF_SECTION;
		ctfsect.cts_type = SHT_PROGBITS;
		ctfsect.cts_flags = SHF_ALLOC;
		ctfsect.cts_size = (size_t)st.st_size;
		ctfsect.cts_entsize = 1;
		ctfsect.cts_offset = 0;

		if ((fp = ctf_bufopen(&ctfsect, NULL, NULL, errp)) == NULL)
			ctf_sect_munmap(&ctfsect);

		return (fp);
	}

	/*
	 * If we have read enough bytes to form an ELF header and the magic
	 * string matches, attempt to interpret the file as an ELF file.  We
	 * do our own largefile ELF processing, and convert everything to
	 * GElf structures so that clients can operate on any data model.
	 */
	if (nbytes >= (ssize_t) sizeof (Elf32_Ehdr) &&
	    bcmp(&hdr.e32.e_ident[EI_MAG0], ELFMAG, SELFMAG) == 0) {
#ifdef	_BIG_ENDIAN
		uchar_t order = ELFDATA2MSB;
#else
		uchar_t order = ELFDATA2LSB;
#endif
		GElf_Half i, n;
		GElf_Shdr *sp;

		void *strs_map;
		size_t strs_mapsz;
		char *strs;

		if (hdr.e32.e_ident[EI_DATA] != order)
			return (ctf_set_open_errno(errp, ECTF_ENDIAN));
		if (hdr.e32.e_version != EV_CURRENT)
			return (ctf_set_open_errno(errp, ECTF_ELFVERS));

		if (hdr.e32.e_ident[EI_CLASS] == ELFCLASS64) {
			if (nbytes < (ssize_t) sizeof (GElf_Ehdr))
				return (ctf_set_open_errno(errp, ECTF_FMT));
		} else {
			Elf32_Ehdr e32 = hdr.e32;
			ehdr_to_gelf(&e32, &hdr.e64);
		}

		if (hdr.e64.e_shstrndx >= hdr.e64.e_shnum)
			return (ctf_set_open_errno(errp, ECTF_CORRUPT));

		n = hdr.e64.e_shnum;
		nbytes = sizeof (GElf_Shdr) * n;

		if ((sp = malloc(nbytes)) == NULL)
			return (ctf_set_open_errno(errp, errno));

		/*
		 * Read in and convert to GElf the array of Shdr structures
		 * from e_shoff so we can locate sections of interest.
		 */
		if (hdr.e32.e_ident[EI_CLASS] == ELFCLASS32) {
			Elf32_Shdr *sp32;

			nbytes = sizeof (Elf32_Shdr) * n;

			if ((sp32 = malloc(nbytes)) == NULL || pread64(fd,
			    sp32, nbytes, hdr.e64.e_shoff) != nbytes) {
				free(sp);
				return (ctf_set_open_errno(errp, errno));
			}

			for (i = 0; i < n; i++)
				shdr_to_gelf(&sp32[i], &sp[i]);

			free(sp32);

		} else if (pread64(fd, sp, nbytes, hdr.e64.e_shoff) != nbytes) {
			free(sp);
			return (ctf_set_open_errno(errp, errno));
		}

		/*
		 * Now mmap the section header strings section so that we can
		 * perform string comparison on the section names.
		 */
		strs_mapsz = sp[hdr.e64.e_shstrndx].sh_size +
		    (sp[hdr.e64.e_shstrndx].sh_offset & ~_PAGEMASK);

		strs_map = mmap64(NULL, strs_mapsz, PROT_READ, MAP_PRIVATE,
		    fd, sp[hdr.e64.e_shstrndx].sh_offset & _PAGEMASK);

		strs = (char *)strs_map +
		    (sp[hdr.e64.e_shstrndx].sh_offset & ~_PAGEMASK);

		if (strs_map == MAP_FAILED) {
			free(sp);
			return (ctf_set_open_errno(errp, ECTF_MMAP));
		}

		/*
		 * Iterate over the section header array looking for the CTF
		 * section and symbol table.  The strtab is linked to symtab.
		 */
		for (i = 0; i < n; i++) {
			const GElf_Shdr *shp = &sp[i];
			const GElf_Shdr *lhp = &sp[shp->sh_link];

			if (shp->sh_link >= hdr.e64.e_shnum)
				continue; /* corrupt sh_link field */

			if (shp->sh_name >= sp[hdr.e64.e_shstrndx].sh_size ||
			    lhp->sh_name >= sp[hdr.e64.e_shstrndx].sh_size)
				continue; /* corrupt sh_name field */

			if (shp->sh_type == SHT_PROGBITS &&
			    strcmp(strs + shp->sh_name, _CTF_SECTION) == 0) {
				ctfsect.cts_name = strs + shp->sh_name;
				ctfsect.cts_type = shp->sh_type;
				ctfsect.cts_flags = shp->sh_flags;
				ctfsect.cts_size = shp->sh_size;
				ctfsect.cts_entsize = shp->sh_entsize;
				ctfsect.cts_offset = (off64_t)shp->sh_offset;

			} else if (shp->sh_type == SHT_SYMTAB) {
				symsect.cts_name = strs + shp->sh_name;
				symsect.cts_type = shp->sh_type;
				symsect.cts_flags = shp->sh_flags;
				symsect.cts_size = shp->sh_size;
				symsect.cts_entsize = shp->sh_entsize;
				symsect.cts_offset = (off64_t)shp->sh_offset;

				strsect.cts_name = strs + lhp->sh_name;
				strsect.cts_type = lhp->sh_type;
				strsect.cts_flags = lhp->sh_flags;
				strsect.cts_size = lhp->sh_size;
				strsect.cts_entsize = lhp->sh_entsize;
				strsect.cts_offset = (off64_t)lhp->sh_offset;
			}
		}

		free(sp); /* free section header array */

		if (ctfsect.cts_type == SHT_NULL) {
			(void) munmap(strs_map, strs_mapsz);
			return (ctf_set_open_errno(errp, ECTF_NOCTFDATA));
		}

		/*
		 * Now mmap the CTF data, symtab, and strtab sections and
		 * call ctf_bufopen() to do the rest of the work.
		 */
		if (ctf_sect_mmap(&ctfsect, fd) == MAP_FAILED) {
			(void) munmap(strs_map, strs_mapsz);
			return (ctf_set_open_errno(errp, ECTF_MMAP));
		}

		if (symsect.cts_type != SHT_NULL &&
		    strsect.cts_type != SHT_NULL) {
			if (ctf_sect_mmap(&symsect, fd) == MAP_FAILED ||
			    ctf_sect_mmap(&strsect, fd) == MAP_FAILED) {
				(void) ctf_set_open_errno(errp, ECTF_MMAP);
				goto bad; /* unmap all and abort */
			}
			fp = ctf_bufopen(&ctfsect, &symsect, &strsect, errp);
		} else
			fp = ctf_bufopen(&ctfsect, NULL, NULL, errp);
bad:
		if (fp == NULL) {
			ctf_sect_munmap(&ctfsect);
			ctf_sect_munmap(&symsect);
			ctf_sect_munmap(&strsect);
		} else
			fp->ctf_flags |= LCTF_MMAP;

		(void) munmap(strs_map, strs_mapsz);
		return (fp);
	}

	return (ctf_set_open_errno(errp, ECTF_FMT));
}

/*
 * Open the specified file and return a pointer to a CTF container.  The file
 * can be either an ELF file or raw CTF file.  This is just a convenient
 * wrapper around ctf_fdopen() for callers.
 */
ctf_file_t *
ctf_open(const char *filename, int *errp)
{
	ctf_file_t *fp;
	int fd;

	if ((fd = open64(filename, O_RDONLY)) == -1) {
		if (errp != NULL)
			*errp = errno;
		return (NULL);
	}

	fp = ctf_fdopen(fd, errp);
	(void) close(fd);
	return (fp);
}

/*
 * Write the uncompressed CTF data stream to the specified file descriptor.
 * This is useful for saving the results of dynamic CTF containers.
 */
int
ctf_write(ctf_file_t *fp, int fd)
{
	const uchar_t *buf = fp->ctf_base;
	ssize_t resid = fp->ctf_size;
	ssize_t len;

	while (resid != 0) {
		if ((len = write(fd, buf, resid)) <= 0)
			return (ctf_set_errno(fp, errno));
		resid -= len;
		buf += len;
	}

	return (0);
}

/*
 * Set the CTF library client version to the specified version.  If version is
 * zero, we just return the default library version number.
 */
int
ctf_version(int version)
{
	if (version < 0) {
		errno = EINVAL;
		return (-1);
	}

	if (version > 0) {
		if (version > CTF_VERSION) {
			errno = ENOTSUP;
			return (-1);
		}
		ctf_dprintf("ctf_version: client using version %d\n", version);
		_libctf_version = version;
	}

	return (_libctf_version);
}