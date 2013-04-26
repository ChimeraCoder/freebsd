
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

#include <sys/types.h>

#include <machine/elf.h>
#include <machine/endian.h>

#include <libelf.h>
#include <osreldate.h>

#include "_libelf.h"

struct align {
	int a32;
	int a64;
};

#ifdef	__GNUC__
#define	MALIGN(N)	{					\
		.a32 = __alignof__(Elf32_##N),			\
		.a64 = __alignof__(Elf64_##N)			\
	}
#define	MALIGN64(V)	  {					\
		.a32 = 0,					\
		.a64 = __alignof__(Elf64_##V)			\
	}
#define	MALIGN_WORD()	{					\
		.a32 = __alignof__(int32_t),			\
		.a64 = __alignof__(int64_t)			\
	    }
#else
#error	Need the __alignof__ builtin.
#endif
#define	UNSUPPORTED()	{					\
		.a32 = 0,					\
		.a64 = 0					\
	}

static struct align malign[ELF_T_NUM] = {
	[ELF_T_ADDR]	= MALIGN(Addr),
	[ELF_T_BYTE]	= { .a32 = 1, .a64 = 1 },
#if	__FreeBSD_version >= 700025
	[ELF_T_CAP]	= MALIGN(Cap),
#endif
	[ELF_T_DYN]	= MALIGN(Dyn),
	[ELF_T_EHDR]	= MALIGN(Ehdr),
	[ELF_T_HALF]	= MALIGN(Half),
#if	__FreeBSD_version >= 700025
	[ELF_T_LWORD]	= MALIGN(Lword),
	[ELF_T_MOVE]	= MALIGN(Move),
#endif
	[ELF_T_MOVEP] 	= UNSUPPORTED(),
#if	__FreeBSD_version >= 700025
	[ELF_T_NOTE]	= MALIGN(Nhdr),
#endif
	[ELF_T_OFF]	= MALIGN(Off),
	[ELF_T_PHDR]	= MALIGN(Phdr),
	[ELF_T_REL]	= MALIGN(Rel),
	[ELF_T_RELA]	= MALIGN(Rela),
	[ELF_T_SHDR]	= MALIGN(Shdr),
	[ELF_T_SWORD]	= MALIGN(Sword),
	[ELF_T_SXWORD]	= MALIGN64(Sxword),
	[ELF_T_SYM]	= MALIGN(Sym),
#if	__FreeBSD_version >= 700025
	[ELF_T_SYMINFO]	= MALIGN(Syminfo),
#endif
#if	__FreeBSD_version >= 700009
	[ELF_T_VDEF]	= MALIGN(Verdef),
	[ELF_T_VNEED]	= MALIGN(Verneed),
#endif
	[ELF_T_WORD]	= MALIGN(Word),
	[ELF_T_XWORD]	= MALIGN64(Xword),
#if	__FreeBSD_version >= 800062
	[ELF_T_GNUHASH] = MALIGN_WORD()
#endif
};

int
_libelf_malign(Elf_Type t, int elfclass)
{
	if (t >= ELF_T_NUM || (int) t < 0)
		return (0);

	return (elfclass == ELFCLASS32 ? malign[t].a32 :
	    malign[t].a64);
}

#define	FALIGN(A32,A64)	{ .a32 = (A32), .a64 = (A64) }

static struct align falign[ELF_T_NUM] = {
	[ELF_T_ADDR]	= FALIGN(4,8),
	[ELF_T_BYTE]	= FALIGN(1,1),
#if	__FreeBSD_version >= 700025
	[ELF_T_CAP]	= FALIGN(4,8),
#endif
	[ELF_T_DYN]	= FALIGN(4,8),
	[ELF_T_EHDR]	= FALIGN(4,8),
	[ELF_T_HALF]	= FALIGN(2,2),
#if	__FreeBSD_version >= 700025
	[ELF_T_LWORD]	= FALIGN(8,8),
	[ELF_T_MOVE]	= FALIGN(8,8),
#endif
	[ELF_T_MOVEP] 	= UNSUPPORTED(),
#if	__FreeBSD_version >= 700025
	[ELF_T_NOTE]	= FALIGN(1,1),
#endif
	[ELF_T_OFF]	= FALIGN(4,8),
	[ELF_T_PHDR]	= FALIGN(4,8),
	[ELF_T_REL]	= FALIGN(4,8),
	[ELF_T_RELA]	= FALIGN(4,8),
	[ELF_T_SHDR]	= FALIGN(4,8),
	[ELF_T_SWORD]	= FALIGN(4,4),
	[ELF_T_SXWORD]	= FALIGN(0,8),
	[ELF_T_SYM]	= FALIGN(4,8),
#if	__FreeBSD_version >= 700025
	[ELF_T_SYMINFO]	= FALIGN(2,2),
#endif
#if	__FreeBSD_version >= 700009
	[ELF_T_VDEF]	= FALIGN(4,4),
	[ELF_T_VNEED]	= FALIGN(4,4),
#endif
	[ELF_T_WORD]	= FALIGN(4,4),
	[ELF_T_XWORD]	= FALIGN(0,8),
#if	__FreeBSD_version >= 800062
	[ELF_T_GNUHASH] = FALIGN(4,8)
#endif
};

int
_libelf_falign(Elf_Type t, int elfclass)
{
	if (t >= ELF_T_NUM || (int) t < 0)
		return (0);

	return (elfclass == ELFCLASS32 ? falign[t].a32 :
	    falign[t].a64);
}