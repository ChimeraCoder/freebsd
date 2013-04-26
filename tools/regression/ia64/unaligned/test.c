
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

#include <machine/float.h>
#include <string.h>

/* Memory accesses. */
#define	Load			0x01
#define	Store			0x02

/* Data type. */
#define	Integer			0x11
#define	FloatingPoint		0x12

/* Data size. */
#define	Small			0x21
#define	Medium			0x22
#define	Large			0x23

/* Post increment. */
#define	NoPostInc		0x31
#define	MinConstPostInc		0x32
#define	PlusConstPostInc	0x33
#define	ScratchRegPostInc	0x34
#define	PreservedRegPostInc	0x35

#if ACCESS == 0 || TYPE == 0 || SIZE == 0 || POSTINC == 0
#error define ACCESS, TYPE, SIZE and/or POSTINC
#endif

#if TYPE == Integer
#  define	REG		"r8"
#  if SIZE == Small
#    define	DATA_TYPE	short
#    define	DATA_VALUE	0x1234
#    define	LD		"ld2"
#    define	ST		"st2"
#  elif SIZE == Medium
#    define	DATA_TYPE	int
#    define	DATA_VALUE	0x12345678
#    define	LD		"ld4"
#    define	ST		"st4"
#  elif SIZE == Large
#    define	DATA_TYPE	long
#    define	DATA_VALUE	0x1234567890ABCDEF
#    define	LD		"ld8"
#    define	ST		"st8"
#  endif
#elif TYPE == FloatingPoint
#  define	REG		"f6"
#  if SIZE == Small
#    define	DATA_TYPE	float
#    define	DATA_VALUE	FLT_MIN
#    define	LD		"ldfs"
#    define	ST		"stfs"
#  elif SIZE == Medium
#    define	DATA_TYPE	double
#    define	DATA_VALUE	DBL_MIN
#    define	LD		"ldfd"
#    define	ST		"stfd"
#  elif SIZE == Large
#    define	DATA_TYPE	long double
#    define	DATA_VALUE	LDBL_MIN
#    define	LD		"ldfe"
#    define	ST		"stfe"
#  endif
#endif

struct {
	DATA_TYPE aligned;
	char _;
	char misaligned[sizeof(DATA_TYPE)];
} data;

DATA_TYPE *aligned = &data.aligned;
DATA_TYPE *misaligned = (DATA_TYPE *)data.misaligned;
DATA_TYPE value = DATA_VALUE;

void
block_copy(void *dst, void *src, size_t sz)
{

	memcpy(dst, src, sz);
}

int
main()
{

	/* Set PSR.ac. */
	asm volatile("sum 8");

#if ACCESS == Load
	/*
	 * LOAD
	 */
	block_copy(misaligned, &value, sizeof(DATA_TYPE));

#  if POSTINC == NoPostInc
	/* Misaligned load. */
	*aligned = *misaligned;
#  elif POSTINC == MinConstPostInc
	asm volatile(
		"ld8 r2=%0;;"
		LD " " REG "=[r2],%2;;"
		"st8 %0=r2;" ST " %1=" REG ";;"
	    : "=m"(misaligned), "=m"(*aligned)
	    : "i"(-sizeof(DATA_TYPE))
	    : REG, "r2", "memory");
#  elif POSTINC == PlusConstPostInc
	asm volatile(
		"ld8 r2=%0;;"
		LD " " REG "=[r2],%2;;"
		"st8 %0=r2;" ST " %1=" REG ";;"
	    : "=m"(misaligned), "=m"(*aligned)
	    : "i"(sizeof(DATA_TYPE))
	    : REG, "r2", "memory");
#  elif POSTINC == ScratchRegPostInc
	asm volatile(
		"ld8 r2=%0; mov r3=%2;;"
		LD " " REG "=[r2],r3;;"
		"st8 %0=r2;" ST " %1=" REG ";;"
	    : "=m"(misaligned), "=m"(*aligned)
	    : "i"(sizeof(DATA_TYPE))
	    : REG, "r2", "r3", "memory");
#  elif POSTINC == PreservedRegPostInc
	asm volatile(
		"ld8 r2=%0; mov r4=%2;;"
		LD " " REG "=[r2],r4;;"
		"st8 %0=r2;" ST " %1=" REG ";;"
	    : "=m"(misaligned), "=m"(*aligned)
	    : "i"(sizeof(DATA_TYPE))
	    : REG, "r2", "r4", "memory");
#  endif

#elif ACCESS == Store
	/*
	 * STORE
	 */

#  if POSTINC == NoPostInc
	/* Misaligned store. */
	*misaligned = value;
#  elif POSTINC == MinConstPostInc
	asm volatile(
		"ld8 r2=%0;" LD " " REG "=%1;;"
		ST " [r2]=" REG ",%2;;"
		"st8 %0=r2;;"
	    : "=m"(misaligned)
	    : "m"(value), "i"(-sizeof(DATA_TYPE))
	    : REG, "r2", "memory");
#  elif POSTINC == PlusConstPostInc
	asm volatile(
		"ld8 r2=%0;" LD " " REG "=%1;;"
		ST " [r2]=" REG ",%2;;"
		"st8 %0=r2;;"
	    : "=m"(misaligned)
	    : "m"(value), "i"(sizeof(DATA_TYPE))
	    : REG, "r2", "memory");
#  elif POSTINC == ScratchRegPostInc || POSTINC == PreservedRegPostInc
	return (1);
#  endif

	block_copy(aligned, data.misaligned, sizeof(DATA_TYPE));
#endif

	if (*aligned != value)
		return (2);

#if POSTINC == NoPostInc
	return (0);
#elif POSTINC == MinConstPostInc
	return (((char *)misaligned == data.misaligned - sizeof(DATA_TYPE))
	    ? 0 : 4);
#else
	return (((char *)misaligned == data.misaligned + sizeof(DATA_TYPE))
	    ? 0 : 4);
#endif
}