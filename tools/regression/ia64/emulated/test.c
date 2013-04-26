
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

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

/* Supported long branch types */
#define	Call		1
#define	Cond		2

/* Supported predicates */
#define	False		1
#define	True		2

/* Supported variations */
#define	Backward	1
#define	Forward		2

#if TYPE == 0 || PRED == 0 || VAR == 0
#error Define TYPE, PRED and/or VAR
#endif

union bundle {
	unsigned char bytes[16];
	long double _align;
};

/*
 * Machine code of a bundle containing a long branch. The predicate of the
 * long branch is the result of the compare in the first slot.
 * The assembly of the bundle is:
 *	{	.mlx
 *		cmp.eq		p0,p15= <PREDICATE>,r0
 *	  (p15)	brl.few		<TARGET> ;;
 *	}
 * the predicate is written to bit 18:1
 * The branch target is written to bits 100:20, 48:39 and 123:1
 */
unsigned char mc_brl_cond[16] = {
	0x05, 0x00, 0x00, 0x00, 0x0f, 0x39,
	0x00, 0x00, 0x00, 0x00, 0x80, 0x07,
	0x00, 0x00, 0x00, 0xc0 
};

/*
 * Machine code of the epilogue of a typical function returning an integer.
 * The assembly of the epilogue is:
 *	{	.mib
 *		nop.m		0
 *		addl		r8 = <RETVAL>, r0
 *		br.ret.sptk.few b0 ;;
 *	}
 * The return value is written to bits 59:7, 73:9, 68:5, and 82:1.
 */
unsigned char mc_epilogue[16] = {
	0x11, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x80, 0x00, 0x00, 0x00, 0x48, 0x80,
	0x00, 0x00, 0x84, 0x00
};

void
mc_patch(union bundle *b, unsigned long val, int start, int len)
{
	unsigned long mask;
	int bit, byte, run;

	byte = start >> 3;
	bit = start & 7;
	while (len) {
		run = ((len > (8 - bit)) ? (8 - bit) : len);
		mask = (1UL << run) - 1UL;
		b->bytes[byte] |= (val & mask) << bit;
		val >>= run;
		len -= run;
		byte++;
		bit = 0;
	}
}

void
assemble_brl_cond(union bundle *b, int pred, unsigned long tgt)
{
	unsigned long iprel;

	iprel = tgt - (unsigned long)b;
	memcpy(b->bytes, mc_brl_cond, sizeof(mc_brl_cond));
	mc_patch(b, pred ? 1 : 0, 18, 1);
	mc_patch(b, iprel >> 4, 100, 20);
	mc_patch(b, iprel >> 24, 48, 39);
	mc_patch(b, iprel >> 63, 123, 1);
}

void
assemble_epilogue(union bundle *b, int retval)
{
	memcpy(b->bytes, mc_epilogue, sizeof(mc_epilogue));
	mc_patch(b, retval, 59, 7);
	mc_patch(b, retval >> 7, 73, 9);
	mc_patch(b, retval >> 16, 68, 5);
	mc_patch(b, retval >> 21, 82, 1);
}

int
doit(void *addr)
{
	asm("mov b6 = %0; br.sptk b6;;" :: "r"(addr));
	return 1;
}

int
test_cond(int pred, union bundle *src, union bundle *dst)
{
	assemble_epilogue(dst, pred ? 0 : 2);
	assemble_brl_cond(src, pred ? 1 : 0, (unsigned long)dst);
	assemble_epilogue(src + 1, !pred ? 0 : 2);
	return doit(src);
}

int
main()
{
	static union bundle blob_low[2];
	union bundle *blob_high;
	void *addr;

	addr = (void *)0x7FFFFFFF00000000L;
	blob_high = mmap(addr, 32, PROT_EXEC | PROT_READ | PROT_WRITE,
	    MAP_ANON, -1, 0L);
	if (blob_high != addr)
		printf("NOTICE: blob_high is at %p, not at %p\n", blob_high,
		    addr);

#if TYPE == Call
	return (test_call(blob_high, blob_low));
#elif TYPE == Cond
  #if VAR == Forward
	return (test_cond(PRED - 1, blob_low, blob_high));
  #elif VAR == Backward
	return (test_cond(PRED - 1, blob_high, blob_low));
  #else
	return (1);
  #endif
#else
	return (1);
#endif
}