
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
/* ia64-opc-m.c -- IA-64 `M' opcode table.   Copyright 1998, 1999, 2000, 2002, 2005 Free Software Foundation, Inc.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

   This file is part of GDB, GAS, and the GNU binutils.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version
   2, or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "ia64-opc.h"

#define M0	IA64_TYPE_M, 0
#define M	IA64_TYPE_M, 1
#define M2	IA64_TYPE_M, 2

/* instruction bit fields: */
#define bM(x)		(((ia64_insn) ((x) & 0x1)) << 36)
#define bX(x)		(((ia64_insn) ((x) & 0x1)) << 27)
#define bX2(x)		(((ia64_insn) ((x) & 0x3)) << 31)
#define bX3(x)		(((ia64_insn) ((x) & 0x7)) << 33)
#define bX4(x)		(((ia64_insn) ((x) & 0xf)) << 27)
#define bX6a(x)		(((ia64_insn) ((x) & 0x3f)) << 30)
#define bX6b(x)		(((ia64_insn) ((x) & 0x3f)) << 27)
#define bX7(x)		(((ia64_insn) ((x) & 0x1)) << 36)	/* note: alias for bM() */
#define bY(x)		(((ia64_insn) ((x) & 0x1)) << 26)
#define bHint(x)	(((ia64_insn) ((x) & 0x3)) << 28)

#define mM	bM (-1)
#define mX	bX (-1)
#define mX2	bX2 (-1)
#define mX3	bX3 (-1)
#define mX4	bX4 (-1)
#define mX6a	bX6a (-1)
#define mX6b	bX6b (-1)
#define mX7	bX7 (-1)
#define mY	bY (-1)
#define mHint	bHint (-1)

#define OpX3(a,b) 		(bOp (a) | bX3 (b)), (mOp | mX3)
#define OpX3X6b(a,b,c) 		(bOp (a) | bX3 (b) | bX6b (c)), \
				(mOp | mX3 | mX6b)
#define OpX3X6bX7(a,b,c,d)	(bOp (a) | bX3 (b) | bX6b (c) | bX7 (d)), \
				(mOp | mX3 | mX6b | mX7)
#define OpX3X4(a,b,c)	 	(bOp (a) | bX3 (b) | bX4 (c)), \
				(mOp | mX3 | mX4)
#define OpX3X4X2(a,b,c,d) 	(bOp (a) | bX3 (b) | bX4 (c) | bX2 (d)), \
				(mOp | mX3 | mX4 | mX2)
#define OpX3X4X2Y(a,b,c,d,e) 	(bOp (a) | bX3 (b) | bX4 (c) | bX2 (d) | bY (e)), \
				(mOp | mX3 | mX4 | mX2 | mY)
#define OpX6aHint(a,b,c) 	(bOp (a) | bX6a (b) | bHint (c)), \
				(mOp | mX6a | mHint)
#define OpXX6aHint(a,b,c,d) 	(bOp (a) | bX (b) | bX6a (c) | bHint (d)), \
				(mOp | mX | mX6a | mHint)
#define OpMXX6a(a,b,c,d) \
	(bOp (a) | bM (b) | bX (c) | bX6a (d)), (mOp | mM | mX | mX6a)
#define OpMXX6aHint(a,b,c,d,e) \
	(bOp (a) | bM (b) | bX (c) | bX6a (d) | bHint (e)), \
	(mOp | mM | mX | mX6a | mHint)

/* Used to initialise unused fields in ia64_opcode struct,
   in order to stop gcc from complaining.  */
#define EMPTY 0,0,NULL

struct ia64_opcode ia64_opcodes_m[] =
  {
    /* M-type instruction encodings (sorted according to major opcode).  */

    {"chk.a.nc",	M0, OpX3 (0, 4), {R1, TGT25c}, EMPTY},
    {"chk.a.clr",	M0, OpX3 (0, 5), {R1, TGT25c}, EMPTY},
    {"chk.a.nc",	M0, OpX3 (0, 6), {F1, TGT25c}, EMPTY},
    {"chk.a.clr",	M0, OpX3 (0, 7), {F1, TGT25c}, EMPTY},

    {"invala",		M0, OpX3X4X2 (0, 0, 0, 1), {}, EMPTY},
    {"fwb",		M0, OpX3X4X2 (0, 0, 0, 2), {}, EMPTY},
    {"mf",		M0, OpX3X4X2 (0, 0, 2, 2), {}, EMPTY},
    {"mf.a",		M0, OpX3X4X2 (0, 0, 3, 2), {}, EMPTY},
    {"srlz.d",		M0, OpX3X4X2 (0, 0, 0, 3), {}, EMPTY},
    {"srlz.i",		M0, OpX3X4X2 (0, 0, 1, 3), {}, EMPTY},
    {"sync.i",		M0, OpX3X4X2 (0, 0, 3, 3), {}, EMPTY},
    {"flushrs",		M0, OpX3X4X2 (0, 0, 0xc, 0), {}, FIRST | NO_PRED, 0, NULL},
    {"loadrs",		M0, OpX3X4X2 (0, 0, 0xa, 0), {}, FIRST | NO_PRED, 0, NULL},
    {"invala.e",	M0, OpX3X4X2 (0, 0, 2, 1), {R1}, EMPTY},
    {"invala.e",	M0, OpX3X4X2 (0, 0, 3, 1), {F1}, EMPTY},
    {"mov.m",		M, OpX3X4X2 (0, 0, 8, 2), {AR3, IMM8}, EMPTY},

    {"break.m",		M0, OpX3X4X2 (0, 0, 0, 0), {IMMU21}, EMPTY},
    {"nop.m",		M0, OpX3X4X2Y (0, 0, 1, 0, 0), {IMMU21}, EMPTY},
    {"hint.m",		M0, OpX3X4X2Y (0, 0, 1, 0, 1), {IMMU21}, EMPTY},

    {"sum",		M0, OpX3X4 (0, 0, 4), {IMMU24}, EMPTY},
    {"rum",		M0, OpX3X4 (0, 0, 5), {IMMU24}, EMPTY},
    {"ssm",		M0, OpX3X4 (0, 0, 6), {IMMU24}, PRIV, 0, NULL},
    {"rsm",		M0, OpX3X4 (0, 0, 7), {IMMU24}, PRIV, 0, NULL},

    {"mov.m",	M, OpX3X6b (1, 0, 0x2a), {AR3, R2}, EMPTY},
    {"mov.m",	M, OpX3X6b (1, 0, 0x22), {R1, AR3}, EMPTY},
    {"mov",	M, OpX3X6b (1, 0, 0x2c), {CR3, R2}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x24), {R1, CR3}, PRIV, 0, NULL},

    {"alloc",	M, OpX3 (1, 6), {R1, AR_PFS, SOF, SOL, SOR}, FIRST|NO_PRED|MOD_RRBS, 0, NULL},
    {"alloc",	M, OpX3 (1, 6), {R1, SOF, SOL, SOR}, PSEUDO|FIRST|NO_PRED|MOD_RRBS, 0, NULL},

    {"mov",	M, OpX3X6b (1, 0, 0x2d), {PSR_L, R2}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x29), {PSR_UM, R2}, EMPTY},
    {"mov",	M, OpX3X6b (1, 0, 0x25), {R1, PSR}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x21), {R1, PSR_UM}, EMPTY},
    {"probe.r",	M, OpX3X6b (1, 0, 0x38), {R1, R3, R2}, EMPTY},
    {"probe.w",	M, OpX3X6b (1, 0, 0x39), {R1, R3, R2}, EMPTY},
    {"probe.r", M, OpX3X6b (1, 0, 0x18), {R1, R3, IMMU2}, EMPTY},
    {"probe.w", M, OpX3X6b (1, 0, 0x19), {R1, R3, IMMU2}, EMPTY},
    {"probe.rw.fault", M0, OpX3X6b (1, 0, 0x31), {R3, IMMU2}, EMPTY},
    {"probe.r.fault",  M0, OpX3X6b (1, 0, 0x32), {R3, IMMU2}, EMPTY},
    {"probe.w.fault",  M0, OpX3X6b (1, 0, 0x33), {R3, IMMU2}, EMPTY},
    {"itc.d",	M0, OpX3X6b (1, 0, 0x2e), {R2}, LAST | PRIV, 0, NULL},
    {"itc.i",	M0, OpX3X6b (1, 0, 0x2f), {R2}, LAST | PRIV, 0, NULL},

    {"mov",	M, OpX3X6b (1, 0, 0x00), {RR_R3, R2}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x01), {DBR_R3, R2}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x02), {IBR_R3, R2}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x03), {PKR_R3, R2}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x04), {PMC_R3, R2}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x05), {PMD_R3, R2}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x06), {MSR_R3, R2}, PRIV, 0, NULL},
    {"itr.d",	M, OpX3X6b (1, 0, 0x0e), {DTR_R3, R2}, PRIV, 0, NULL},
    {"itr.i",	M, OpX3X6b (1, 0, 0x0f), {ITR_R3, R2}, PRIV, 0, NULL},

    {"mov",	M, OpX3X6b (1, 0, 0x10), {R1, RR_R3}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x11), {R1, DBR_R3}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x12), {R1, IBR_R3}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x13), {R1, PKR_R3}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x14), {R1, PMC_R3}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x15), {R1, PMD_R3}, EMPTY},
    {"mov",	M, OpX3X6b (1, 0, 0x16), {R1, MSR_R3}, PRIV, 0, NULL},
    {"mov",	M, OpX3X6b (1, 0, 0x17), {R1, CPUID_R3}, EMPTY},

    {"ptc.l",	M0, OpX3X6b (1, 0, 0x09), {R3, R2}, PRIV, 0, NULL},
    {"ptc.g",	M0, OpX3X6b (1, 0, 0x0a), {R3, R2}, LAST | PRIV, 0, NULL},
    {"ptc.ga",	M0, OpX3X6b (1, 0, 0x0b), {R3, R2}, LAST | PRIV, 0, NULL},
    {"ptr.d",	M0, OpX3X6b (1, 0, 0x0c), {R3, R2}, PRIV, 0, NULL},
    {"ptr.i",	M0, OpX3X6b (1, 0, 0x0d), {R3, R2}, PRIV, 0, NULL},

    {"thash",	M, OpX3X6b (1, 0, 0x1a), {R1, R3}, EMPTY},
    {"ttag",	M, OpX3X6b (1, 0, 0x1b), {R1, R3}, EMPTY},
    {"tpa",	M, OpX3X6b (1, 0, 0x1e), {R1, R3}, PRIV, 0, NULL},
    {"tak",	M, OpX3X6b (1, 0, 0x1f), {R1, R3}, PRIV, 0, NULL},

    {"chk.s.m",	M0, OpX3 (1, 1), {R2, TGT25b}, EMPTY},
    {"chk.s",	M0, OpX3 (1, 3), {F2, TGT25b}, EMPTY},

    {"fc",	M0, OpX3X6bX7 (1, 0, 0x30, 0), {R3}, EMPTY},
    {"fc.i",	M0, OpX3X6bX7 (1, 0, 0x30, 1), {R3}, EMPTY},
    {"ptc.e",	M0, OpX3X6b (1, 0, 0x34), {R3}, PRIV, 0, NULL},

    /* integer load */
    {"ld1",		M, OpMXX6aHint (4, 0, 0, 0x00, 0), {R1, MR3}, EMPTY},
    {"ld1.nt1",		M, OpMXX6aHint (4, 0, 0, 0x00, 1), {R1, MR3}, EMPTY},
    {"ld1.nta",		M, OpMXX6aHint (4, 0, 0, 0x00, 3), {R1, MR3}, EMPTY},
    {"ld2",		M, OpMXX6aHint (4, 0, 0, 0x01, 0), {R1, MR3}, EMPTY},
    {"ld2.nt1",		M, OpMXX6aHint (4, 0, 0, 0x01, 1), {R1, MR3}, EMPTY},
    {"ld2.nta",		M, OpMXX6aHint (4, 0, 0, 0x01, 3), {R1, MR3}, EMPTY},
    {"ld4",		M, OpMXX6aHint (4, 0, 0, 0x02, 0), {R1, MR3}, EMPTY},
    {"ld4.nt1",		M, OpMXX6aHint (4, 0, 0, 0x02, 1), {R1, MR3}, EMPTY},
    {"ld4.nta",		M, OpMXX6aHint (4, 0, 0, 0x02, 3), {R1, MR3}, EMPTY},
    {"ld8",		M, OpMXX6aHint (4, 0, 0, 0x03, 0), {R1, MR3}, EMPTY},
    {"ld8.nt1",		M, OpMXX6aHint (4, 0, 0, 0x03, 1), {R1, MR3}, EMPTY},
    {"ld8.nta",		M, OpMXX6aHint (4, 0, 0, 0x03, 3), {R1, MR3}, EMPTY},
    {"ld16",		M2, OpMXX6aHint (4, 0, 1, 0x28, 0), {R1, AR_CSD, MR3}, EMPTY},
    {"ld16",		M, OpMXX6aHint (4, 0, 1, 0x28, 0), {R1, MR3}, PSEUDO, 0, NULL},
    {"ld16.nt1",	M2, OpMXX6aHint (4, 0, 1, 0x28, 1), {R1, AR_CSD, MR3}, EMPTY},
    {"ld16.nt1",	M, OpMXX6aHint (4, 0, 1, 0x28, 1), {R1, MR3}, PSEUDO, 0, NULL},
    {"ld16.nta",	M2, OpMXX6aHint (4, 0, 1, 0x28, 3), {R1, AR_CSD, MR3}, EMPTY},
    {"ld16.nta",	M, OpMXX6aHint (4, 0, 1, 0x28, 3), {R1, MR3}, PSEUDO, 0, NULL},
    {"ld1.s",		M, OpMXX6aHint (4, 0, 0, 0x04, 0), {R1, MR3}, EMPTY},
    {"ld1.s.nt1",	M, OpMXX6aHint (4, 0, 0, 0x04, 1), {R1, MR3}, EMPTY},
    {"ld1.s.nta",	M, OpMXX6aHint (4, 0, 0, 0x04, 3), {R1, MR3}, EMPTY},
    {"ld2.s",		M, OpMXX6aHint (4, 0, 0, 0x05, 0), {R1, MR3}, EMPTY},
    {"ld2.s.nt1",	M, OpMXX6aHint (4, 0, 0, 0x05, 1), {R1, MR3}, EMPTY},
    {"ld2.s.nta",	M, OpMXX6aHint (4, 0, 0, 0x05, 3), {R1, MR3}, EMPTY},
    {"ld4.s",		M, OpMXX6aHint (4, 0, 0, 0x06, 0), {R1, MR3}, EMPTY},
    {"ld4.s.nt1",	M, OpMXX6aHint (4, 0, 0, 0x06, 1), {R1, MR3}, EMPTY},
    {"ld4.s.nta",	M, OpMXX6aHint (4, 0, 0, 0x06, 3), {R1, MR3}, EMPTY},
    {"ld8.s",		M, OpMXX6aHint (4, 0, 0, 0x07, 0), {R1, MR3}, EMPTY},
    {"ld8.s.nt1",	M, OpMXX6aHint (4, 0, 0, 0x07, 1), {R1, MR3}, EMPTY},
    {"ld8.s.nta",	M, OpMXX6aHint (4, 0, 0, 0x07, 3), {R1, MR3}, EMPTY},
    {"ld1.a",		M, OpMXX6aHint (4, 0, 0, 0x08, 0), {R1, MR3}, EMPTY},
    {"ld1.a.nt1",	M, OpMXX6aHint (4, 0, 0, 0x08, 1), {R1, MR3}, EMPTY},
    {"ld1.a.nta",	M, OpMXX6aHint (4, 0, 0, 0x08, 3), {R1, MR3}, EMPTY},
    {"ld2.a",		M, OpMXX6aHint (4, 0, 0, 0x09, 0), {R1, MR3}, EMPTY},
    {"ld2.a.nt1",	M, OpMXX6aHint (4, 0, 0, 0x09, 1), {R1, MR3}, EMPTY},
    {"ld2.a.nta",	M, OpMXX6aHint (4, 0, 0, 0x09, 3), {R1, MR3}, EMPTY},
    {"ld4.a",		M, OpMXX6aHint (4, 0, 0, 0x0a, 0), {R1, MR3}, EMPTY},
    {"ld4.a.nt1",	M, OpMXX6aHint (4, 0, 0, 0x0a, 1), {R1, MR3}, EMPTY},
    {"ld4.a.nta",	M, OpMXX6aHint (4, 0, 0, 0x0a, 3), {R1, MR3}, EMPTY},
    {"ld8.a",		M, OpMXX6aHint (4, 0, 0, 0x0b, 0), {R1, MR3}, EMPTY},
    {"ld8.a.nt1",	M, OpMXX6aHint (4, 0, 0, 0x0b, 1), {R1, MR3}, EMPTY},
    {"ld8.a.nta",	M, OpMXX6aHint (4, 0, 0, 0x0b, 3), {R1, MR3}, EMPTY},
    {"ld1.sa",		M, OpMXX6aHint (4, 0, 0, 0x0c, 0), {R1, MR3}, EMPTY},
    {"ld1.sa.nt1",	M, OpMXX6aHint (4, 0, 0, 0x0c, 1), {R1, MR3}, EMPTY},
    {"ld1.sa.nta",	M, OpMXX6aHint (4, 0, 0, 0x0c, 3), {R1, MR3}, EMPTY},
    {"ld2.sa",		M, OpMXX6aHint (4, 0, 0, 0x0d, 0), {R1, MR3}, EMPTY},
    {"ld2.sa.nt1",	M, OpMXX6aHint (4, 0, 0, 0x0d, 1), {R1, MR3}, EMPTY},
    {"ld2.sa.nta",	M, OpMXX6aHint (4, 0, 0, 0x0d, 3), {R1, MR3}, EMPTY},
    {"ld4.sa",		M, OpMXX6aHint (4, 0, 0, 0x0e, 0), {R1, MR3}, EMPTY},
    {"ld4.sa.nt1",	M, OpMXX6aHint (4, 0, 0, 0x0e, 1), {R1, MR3}, EMPTY},
    {"ld4.sa.nta",	M, OpMXX6aHint (4, 0, 0, 0x0e, 3), {R1, MR3}, EMPTY},
    {"ld8.sa",		M, OpMXX6aHint (4, 0, 0, 0x0f, 0), {R1, MR3}, EMPTY},
    {"ld8.sa.nt1",	M, OpMXX6aHint (4, 0, 0, 0x0f, 1), {R1, MR3}, EMPTY},
    {"ld8.sa.nta",	M, OpMXX6aHint (4, 0, 0, 0x0f, 3), {R1, MR3}, EMPTY},
    {"ld1.bias",	M, OpMXX6aHint (4, 0, 0, 0x10, 0), {R1, MR3}, EMPTY},
    {"ld1.bias.nt1",	M, OpMXX6aHint (4, 0, 0, 0x10, 1), {R1, MR3}, EMPTY},
    {"ld1.bias.nta",	M, OpMXX6aHint (4, 0, 0, 0x10, 3), {R1, MR3}, EMPTY},
    {"ld2.bias",	M, OpMXX6aHint (4, 0, 0, 0x11, 0), {R1, MR3}, EMPTY},
    {"ld2.bias.nt1",	M, OpMXX6aHint (4, 0, 0, 0x11, 1), {R1, MR3}, EMPTY},
    {"ld2.bias.nta",	M, OpMXX6aHint (4, 0, 0, 0x11, 3), {R1, MR3}, EMPTY},
    {"ld4.bias",	M, OpMXX6aHint (4, 0, 0, 0x12, 0), {R1, MR3}, EMPTY},
    {"ld4.bias.nt1",	M, OpMXX6aHint (4, 0, 0, 0x12, 1), {R1, MR3}, EMPTY},
    {"ld4.bias.nta",	M, OpMXX6aHint (4, 0, 0, 0x12, 3), {R1, MR3}, EMPTY},
    {"ld8.bias",	M, OpMXX6aHint (4, 0, 0, 0x13, 0), {R1, MR3}, EMPTY},
    {"ld8.bias.nt1",	M, OpMXX6aHint (4, 0, 0, 0x13, 1), {R1, MR3}, EMPTY},
    {"ld8.bias.nta",	M, OpMXX6aHint (4, 0, 0, 0x13, 3), {R1, MR3}, EMPTY},
    {"ld1.acq",		M, OpMXX6aHint (4, 0, 0, 0x14, 0), {R1, MR3}, EMPTY},
    {"ld1.acq.nt1",	M, OpMXX6aHint (4, 0, 0, 0x14, 1), {R1, MR3}, EMPTY},
    {"ld1.acq.nta",	M, OpMXX6aHint (4, 0, 0, 0x14, 3), {R1, MR3}, EMPTY},
    {"ld2.acq",		M, OpMXX6aHint (4, 0, 0, 0x15, 0), {R1, MR3}, EMPTY},
    {"ld2.acq.nt1",	M, OpMXX6aHint (4, 0, 0, 0x15, 1), {R1, MR3}, EMPTY},
    {"ld2.acq.nta",	M, OpMXX6aHint (4, 0, 0, 0x15, 3), {R1, MR3}, EMPTY},
    {"ld4.acq",		M, OpMXX6aHint (4, 0, 0, 0x16, 0), {R1, MR3}, EMPTY},
    {"ld4.acq.nt1",	M, OpMXX6aHint (4, 0, 0, 0x16, 1), {R1, MR3}, EMPTY},
    {"ld4.acq.nta",	M, OpMXX6aHint (4, 0, 0, 0x16, 3), {R1, MR3}, EMPTY},
    {"ld8.acq",		M, OpMXX6aHint (4, 0, 0, 0x17, 0), {R1, MR3}, EMPTY},
    {"ld8.acq.nt1",	M, OpMXX6aHint (4, 0, 0, 0x17, 1), {R1, MR3}, EMPTY},
    {"ld8.acq.nta",	M, OpMXX6aHint (4, 0, 0, 0x17, 3), {R1, MR3}, EMPTY},
    {"ld16.acq",	M2, OpMXX6aHint (4, 0, 1, 0x2c, 0), {R1, AR_CSD, MR3}, EMPTY},
    {"ld16.acq",	M, OpMXX6aHint (4, 0, 1, 0x2c, 0), {R1, MR3}, PSEUDO, 0, NULL},
    {"ld16.acq.nt1",	M2, OpMXX6aHint (4, 0, 1, 0x2c, 1), {R1, AR_CSD, MR3}, EMPTY},
    {"ld16.acq.nt1",	M, OpMXX6aHint (4, 0, 1, 0x2c, 1), {R1, MR3}, PSEUDO, 0, NULL},
    {"ld16.acq.nta",	M2, OpMXX6aHint (4, 0, 1, 0x2c, 3), {R1, AR_CSD, MR3}, EMPTY},
    {"ld16.acq.nta",	M, OpMXX6aHint (4, 0, 1, 0x2c, 3), {R1, MR3}, PSEUDO, 0, NULL},
    {"ld8.fill",	M, OpMXX6aHint (4, 0, 0, 0x1b, 0), {R1, MR3}, EMPTY},
    {"ld8.fill.nt1",	M, OpMXX6aHint (4, 0, 0, 0x1b, 1), {R1, MR3}, EMPTY},
    {"ld8.fill.nta",	M, OpMXX6aHint (4, 0, 0, 0x1b, 3), {R1, MR3}, EMPTY},
    {"ld1.c.clr",	M, OpMXX6aHint (4, 0, 0, 0x20, 0), {R1, MR3}, EMPTY},
    {"ld1.c.clr.nt1",	M, OpMXX6aHint (4, 0, 0, 0x20, 1), {R1, MR3}, EMPTY},
    {"ld1.c.clr.nta",	M, OpMXX6aHint (4, 0, 0, 0x20, 3), {R1, MR3}, EMPTY},
    {"ld2.c.clr",	M, OpMXX6aHint (4, 0, 0, 0x21, 0), {R1, MR3}, EMPTY},
    {"ld2.c.clr.nt1",	M, OpMXX6aHint (4, 0, 0, 0x21, 1), {R1, MR3}, EMPTY},
    {"ld2.c.clr.nta",	M, OpMXX6aHint (4, 0, 0, 0x21, 3), {R1, MR3}, EMPTY},
    {"ld4.c.clr",	M, OpMXX6aHint (4, 0, 0, 0x22, 0), {R1, MR3}, EMPTY},
    {"ld4.c.clr.nt1",	M, OpMXX6aHint (4, 0, 0, 0x22, 1), {R1, MR3}, EMPTY},
    {"ld4.c.clr.nta",	M, OpMXX6aHint (4, 0, 0, 0x22, 3), {R1, MR3}, EMPTY},
    {"ld8.c.clr",	M, OpMXX6aHint (4, 0, 0, 0x23, 0), {R1, MR3}, EMPTY},
    {"ld8.c.clr.nt1",	M, OpMXX6aHint (4, 0, 0, 0x23, 1), {R1, MR3}, EMPTY},
    {"ld8.c.clr.nta",	M, OpMXX6aHint (4, 0, 0, 0x23, 3), {R1, MR3}, EMPTY},
    {"ld1.c.nc",	M, OpMXX6aHint (4, 0, 0, 0x24, 0), {R1, MR3}, EMPTY},
    {"ld1.c.nc.nt1",	M, OpMXX6aHint (4, 0, 0, 0x24, 1), {R1, MR3}, EMPTY},
    {"ld1.c.nc.nta",	M, OpMXX6aHint (4, 0, 0, 0x24, 3), {R1, MR3}, EMPTY},
    {"ld2.c.nc",	M, OpMXX6aHint (4, 0, 0, 0x25, 0), {R1, MR3}, EMPTY},
    {"ld2.c.nc.nt1",	M, OpMXX6aHint (4, 0, 0, 0x25, 1), {R1, MR3}, EMPTY},
    {"ld2.c.nc.nta",	M, OpMXX6aHint (4, 0, 0, 0x25, 3), {R1, MR3}, EMPTY},
    {"ld4.c.nc",	M, OpMXX6aHint (4, 0, 0, 0x26, 0), {R1, MR3}, EMPTY},
    {"ld4.c.nc.nt1",	M, OpMXX6aHint (4, 0, 0, 0x26, 1), {R1, MR3}, EMPTY},
    {"ld4.c.nc.nta",	M, OpMXX6aHint (4, 0, 0, 0x26, 3), {R1, MR3}, EMPTY},
    {"ld8.c.nc",	M, OpMXX6aHint (4, 0, 0, 0x27, 0), {R1, MR3}, EMPTY},
    {"ld8.c.nc.nt1",	M, OpMXX6aHint (4, 0, 0, 0x27, 1), {R1, MR3}, EMPTY},
    {"ld8.c.nc.nta",	M, OpMXX6aHint (4, 0, 0, 0x27, 3), {R1, MR3}, EMPTY},
    {"ld1.c.clr.acq",	  M, OpMXX6aHint (4, 0, 0, 0x28, 0), {R1, MR3}, EMPTY},
    {"ld1.c.clr.acq.nt1", M, OpMXX6aHint (4, 0, 0, 0x28, 1), {R1, MR3}, EMPTY},
    {"ld1.c.clr.acq.nta", M, OpMXX6aHint (4, 0, 0, 0x28, 3), {R1, MR3}, EMPTY},
    {"ld2.c.clr.acq",	  M, OpMXX6aHint (4, 0, 0, 0x29, 0), {R1, MR3}, EMPTY},
    {"ld2.c.clr.acq.nt1", M, OpMXX6aHint (4, 0, 0, 0x29, 1), {R1, MR3}, EMPTY},
    {"ld2.c.clr.acq.nta", M, OpMXX6aHint (4, 0, 0, 0x29, 3), {R1, MR3}, EMPTY},
    {"ld4.c.clr.acq",	  M, OpMXX6aHint (4, 0, 0, 0x2a, 0), {R1, MR3}, EMPTY},
    {"ld4.c.clr.acq.nt1", M, OpMXX6aHint (4, 0, 0, 0x2a, 1), {R1, MR3}, EMPTY},
    {"ld4.c.clr.acq.nta", M, OpMXX6aHint (4, 0, 0, 0x2a, 3), {R1, MR3}, EMPTY},
    {"ld8.c.clr.acq",	  M, OpMXX6aHint (4, 0, 0, 0x2b, 0), {R1, MR3}, EMPTY},
    {"ld8.c.clr.acq.nt1", M, OpMXX6aHint (4, 0, 0, 0x2b, 1), {R1, MR3}, EMPTY},
    {"ld8.c.clr.acq.nta", M, OpMXX6aHint (4, 0, 0, 0x2b, 3), {R1, MR3}, EMPTY},

    /* Pseudo-op that generates ldxmov relocation.  */
    {"ld8.mov",		M, OpMXX6aHint (4, 0, 0, 0x03, 0),
     {R1, MR3, IA64_OPND_LDXMOV}, EMPTY},

    /* Integer load w/increment by register.  */
#define LDINCREG(c,h) M, OpMXX6aHint (4, 1, 0, c, h), {R1, MR3, R2}, POSTINC, 0, NULL
    {"ld1",		LDINCREG (0x00, 0)},
    {"ld1.nt1",		LDINCREG (0x00, 1)},
    {"ld1.nta",		LDINCREG (0x00, 3)},
    {"ld2",		LDINCREG (0x01, 0)},
    {"ld2.nt1",		LDINCREG (0x01, 1)},
    {"ld2.nta",		LDINCREG (0x01, 3)},
    {"ld4",		LDINCREG (0x02, 0)},
    {"ld4.nt1",		LDINCREG (0x02, 1)},
    {"ld4.nta",		LDINCREG (0x02, 3)},
    {"ld8",		LDINCREG (0x03, 0)},
    {"ld8.nt1",		LDINCREG (0x03, 1)},
    {"ld8.nta",		LDINCREG (0x03, 3)},
    {"ld1.s",		LDINCREG (0x04, 0)},
    {"ld1.s.nt1",	LDINCREG (0x04, 1)},
    {"ld1.s.nta",	LDINCREG (0x04, 3)},
    {"ld2.s",		LDINCREG (0x05, 0)},
    {"ld2.s.nt1",	LDINCREG (0x05, 1)},
    {"ld2.s.nta",	LDINCREG (0x05, 3)},
    {"ld4.s",		LDINCREG (0x06, 0)},
    {"ld4.s.nt1",	LDINCREG (0x06, 1)},
    {"ld4.s.nta",	LDINCREG (0x06, 3)},
    {"ld8.s",		LDINCREG (0x07, 0)},
    {"ld8.s.nt1",	LDINCREG (0x07, 1)},
    {"ld8.s.nta",	LDINCREG (0x07, 3)},
    {"ld1.a",		LDINCREG (0x08, 0)},
    {"ld1.a.nt1",	LDINCREG (0x08, 1)},
    {"ld1.a.nta",	LDINCREG (0x08, 3)},
    {"ld2.a",		LDINCREG (0x09, 0)},
    {"ld2.a.nt1",	LDINCREG (0x09, 1)},
    {"ld2.a.nta",	LDINCREG (0x09, 3)},
    {"ld4.a",		LDINCREG (0x0a, 0)},
    {"ld4.a.nt1",	LDINCREG (0x0a, 1)},
    {"ld4.a.nta",	LDINCREG (0x0a, 3)},
    {"ld8.a",		LDINCREG (0x0b, 0)},
    {"ld8.a.nt1",	LDINCREG (0x0b, 1)},
    {"ld8.a.nta",	LDINCREG (0x0b, 3)},
    {"ld1.sa",		LDINCREG (0x0c, 0)},
    {"ld1.sa.nt1",	LDINCREG (0x0c, 1)},
    {"ld1.sa.nta",	LDINCREG (0x0c, 3)},
    {"ld2.sa",		LDINCREG (0x0d, 0)},
    {"ld2.sa.nt1",	LDINCREG (0x0d, 1)},
    {"ld2.sa.nta",	LDINCREG (0x0d, 3)},
    {"ld4.sa",		LDINCREG (0x0e, 0)},
    {"ld4.sa.nt1",	LDINCREG (0x0e, 1)},
    {"ld4.sa.nta",	LDINCREG (0x0e, 3)},
    {"ld8.sa",		LDINCREG (0x0f, 0)},
    {"ld8.sa.nt1",	LDINCREG (0x0f, 1)},
    {"ld8.sa.nta",	LDINCREG (0x0f, 3)},
    {"ld1.bias",	LDINCREG (0x10, 0)},
    {"ld1.bias.nt1",	LDINCREG (0x10, 1)},
    {"ld1.bias.nta",	LDINCREG (0x10, 3)},
    {"ld2.bias",	LDINCREG (0x11, 0)},
    {"ld2.bias.nt1",	LDINCREG (0x11, 1)},
    {"ld2.bias.nta",	LDINCREG (0x11, 3)},
    {"ld4.bias",	LDINCREG (0x12, 0)},
    {"ld4.bias.nt1",	LDINCREG (0x12, 1)},
    {"ld4.bias.nta",	LDINCREG (0x12, 3)},
    {"ld8.bias",	LDINCREG (0x13, 0)},
    {"ld8.bias.nt1",	LDINCREG (0x13, 1)},
    {"ld8.bias.nta",	LDINCREG (0x13, 3)},
    {"ld1.acq",		LDINCREG (0x14, 0)},
    {"ld1.acq.nt1",	LDINCREG (0x14, 1)},
    {"ld1.acq.nta",	LDINCREG (0x14, 3)},
    {"ld2.acq",		LDINCREG (0x15, 0)},
    {"ld2.acq.nt1",	LDINCREG (0x15, 1)},
    {"ld2.acq.nta",	LDINCREG (0x15, 3)},
    {"ld4.acq",		LDINCREG (0x16, 0)},
    {"ld4.acq.nt1",	LDINCREG (0x16, 1)},
    {"ld4.acq.nta",	LDINCREG (0x16, 3)},
    {"ld8.acq",		LDINCREG (0x17, 0)},
    {"ld8.acq.nt1",	LDINCREG (0x17, 1)},
    {"ld8.acq.nta",	LDINCREG (0x17, 3)},
    {"ld8.fill",	LDINCREG (0x1b, 0)},
    {"ld8.fill.nt1",	LDINCREG (0x1b, 1)},
    {"ld8.fill.nta",	LDINCREG (0x1b, 3)},
    {"ld1.c.clr",	LDINCREG (0x20, 0)},
    {"ld1.c.clr.nt1",	LDINCREG (0x20, 1)},
    {"ld1.c.clr.nta",	LDINCREG (0x20, 3)},
    {"ld2.c.clr",	LDINCREG (0x21, 0)},
    {"ld2.c.clr.nt1",	LDINCREG (0x21, 1)},
    {"ld2.c.clr.nta",	LDINCREG (0x21, 3)},
    {"ld4.c.clr",	LDINCREG (0x22, 0)},
    {"ld4.c.clr.nt1",	LDINCREG (0x22, 1)},
    {"ld4.c.clr.nta",	LDINCREG (0x22, 3)},
    {"ld8.c.clr",	LDINCREG (0x23, 0)},
    {"ld8.c.clr.nt1",	LDINCREG (0x23, 1)},
    {"ld8.c.clr.nta",	LDINCREG (0x23, 3)},
    {"ld1.c.nc",	LDINCREG (0x24, 0)},
    {"ld1.c.nc.nt1",	LDINCREG (0x24, 1)},
    {"ld1.c.nc.nta",	LDINCREG (0x24, 3)},
    {"ld2.c.nc",	LDINCREG (0x25, 0)},
    {"ld2.c.nc.nt1",	LDINCREG (0x25, 1)},
    {"ld2.c.nc.nta",	LDINCREG (0x25, 3)},
    {"ld4.c.nc",	LDINCREG (0x26, 0)},
    {"ld4.c.nc.nt1",	LDINCREG (0x26, 1)},
    {"ld4.c.nc.nta",	LDINCREG (0x26, 3)},
    {"ld8.c.nc",	LDINCREG (0x27, 0)},
    {"ld8.c.nc.nt1",	LDINCREG (0x27, 1)},
    {"ld8.c.nc.nta",	LDINCREG (0x27, 3)},
    {"ld1.c.clr.acq",	  LDINCREG (0x28, 0)},
    {"ld1.c.clr.acq.nt1", LDINCREG (0x28, 1)},
    {"ld1.c.clr.acq.nta", LDINCREG (0x28, 3)},
    {"ld2.c.clr.acq",	  LDINCREG (0x29, 0)},
    {"ld2.c.clr.acq.nt1", LDINCREG (0x29, 1)},
    {"ld2.c.clr.acq.nta", LDINCREG (0x29, 3)},
    {"ld4.c.clr.acq",	  LDINCREG (0x2a, 0)},
    {"ld4.c.clr.acq.nt1", LDINCREG (0x2a, 1)},
    {"ld4.c.clr.acq.nta", LDINCREG (0x2a, 3)},
    {"ld8.c.clr.acq",	  LDINCREG (0x2b, 0)},
    {"ld8.c.clr.acq.nt1", LDINCREG (0x2b, 1)},
    {"ld8.c.clr.acq.nta", LDINCREG (0x2b, 3)},
#undef LDINCREG

    {"st1",		M, OpMXX6aHint (4, 0, 0, 0x30, 0), {MR3, R2}, EMPTY},
    {"st1.nta",		M, OpMXX6aHint (4, 0, 0, 0x30, 3), {MR3, R2}, EMPTY},
    {"st2",		M, OpMXX6aHint (4, 0, 0, 0x31, 0), {MR3, R2}, EMPTY},
    {"st2.nta",		M, OpMXX6aHint (4, 0, 0, 0x31, 3), {MR3, R2}, EMPTY},
    {"st4",		M, OpMXX6aHint (4, 0, 0, 0x32, 0), {MR3, R2}, EMPTY},
    {"st4.nta",		M, OpMXX6aHint (4, 0, 0, 0x32, 3), {MR3, R2}, EMPTY},
    {"st8",		M, OpMXX6aHint (4, 0, 0, 0x33, 0), {MR3, R2}, EMPTY},
    {"st8.nta",		M, OpMXX6aHint (4, 0, 0, 0x33, 3), {MR3, R2}, EMPTY},
    {"st16",		M, OpMXX6aHint (4, 0, 1, 0x30, 0), {MR3, R2, AR_CSD}, EMPTY},
    {"st16",		M, OpMXX6aHint (4, 0, 1, 0x30, 0), {MR3, R2}, PSEUDO, 0, NULL},
    {"st16.nta",	M, OpMXX6aHint (4, 0, 1, 0x30, 3), {MR3, R2, AR_CSD}, EMPTY},
    {"st16.nta",	M, OpMXX6aHint (4, 0, 1, 0x30, 3), {MR3, R2}, PSEUDO, 0, NULL},
    {"st1.rel",		M, OpMXX6aHint (4, 0, 0, 0x34, 0), {MR3, R2}, EMPTY},
    {"st1.rel.nta",	M, OpMXX6aHint (4, 0, 0, 0x34, 3), {MR3, R2}, EMPTY},
    {"st2.rel",		M, OpMXX6aHint (4, 0, 0, 0x35, 0), {MR3, R2}, EMPTY},
    {"st2.rel.nta",	M, OpMXX6aHint (4, 0, 0, 0x35, 3), {MR3, R2}, EMPTY},
    {"st4.rel",		M, OpMXX6aHint (4, 0, 0, 0x36, 0), {MR3, R2}, EMPTY},
    {"st4.rel.nta",	M, OpMXX6aHint (4, 0, 0, 0x36, 3), {MR3, R2}, EMPTY},
    {"st8.rel",		M, OpMXX6aHint (4, 0, 0, 0x37, 0), {MR3, R2}, EMPTY},
    {"st8.rel.nta",	M, OpMXX6aHint (4, 0, 0, 0x37, 3), {MR3, R2}, EMPTY},
    {"st16.rel",	M, OpMXX6aHint (4, 0, 1, 0x34, 0), {MR3, R2, AR_CSD}, EMPTY},
    {"st16.rel",	M, OpMXX6aHint (4, 0, 1, 0x34, 0), {MR3, R2}, PSEUDO, 0, NULL},
    {"st16.rel.nta",	M, OpMXX6aHint (4, 0, 1, 0x34, 3), {MR3, R2, AR_CSD}, EMPTY},
    {"st16.rel.nta",	M, OpMXX6aHint (4, 0, 1, 0x34, 3), {MR3, R2}, PSEUDO, 0, NULL},
    {"st8.spill",	M, OpMXX6aHint (4, 0, 0, 0x3b, 0), {MR3, R2}, EMPTY},
    {"st8.spill.nta",	M, OpMXX6aHint (4, 0, 0, 0x3b, 3), {MR3, R2}, EMPTY},

#define CMPXCHG(c,h)	M, OpMXX6aHint (4, 0, 1, c, h), {R1, MR3, R2, AR_CCV}, EMPTY
#define CMPXCHG_P(c,h)	M, OpMXX6aHint (4, 0, 1, c, h), {R1, MR3, R2}, PSEUDO, 0, NULL
#define CMPXCHG16(c,h)	M, OpMXX6aHint (4, 0, 1, c, h), {R1, MR3, R2, AR_CSD, AR_CCV}, EMPTY
#define CMPXCHG16_P(c,h) M, OpMXX6aHint (4, 0, 1, c, h), {R1, MR3, R2}, PSEUDO, 0, NULL
#define CMPXCHG_acq 0
#define CMPXCHG_rel 4
#define CMPXCHG_1 0
#define CMPXCHG_2 1
#define CMPXCHG_4 2
#define CMPXCHG_8 3
#define CMPXCHGn(n, s) \
    {"cmpxchg"#n"."#s,       CMPXCHG   (CMPXCHG_##n|CMPXCHG_##s, 0)}, \
    {"cmpxchg"#n"."#s,       CMPXCHG_P (CMPXCHG_##n|CMPXCHG_##s, 0)}, \
    {"cmpxchg"#n"."#s".nt1", CMPXCHG   (CMPXCHG_##n|CMPXCHG_##s, 1)}, \
    {"cmpxchg"#n"."#s".nt1", CMPXCHG_P (CMPXCHG_##n|CMPXCHG_##s, 1)}, \
    {"cmpxchg"#n"."#s".nta", CMPXCHG   (CMPXCHG_##n|CMPXCHG_##s, 3)}, \
    {"cmpxchg"#n"."#s".nta", CMPXCHG_P (CMPXCHG_##n|CMPXCHG_##s, 3)}
#define CMP8XCHG16(s) \
    {"cmp8xchg16."#s,       CMPXCHG16   (0x20|CMPXCHG_##s, 0)}, \
    {"cmp8xchg16."#s,       CMPXCHG16_P (0x20|CMPXCHG_##s, 0)}, \
    {"cmp8xchg16."#s".nt1", CMPXCHG16   (0x20|CMPXCHG_##s, 1)}, \
    {"cmp8xchg16."#s".nt1", CMPXCHG16_P (0x20|CMPXCHG_##s, 1)}, \
    {"cmp8xchg16."#s".nta", CMPXCHG16   (0x20|CMPXCHG_##s, 3)}, \
    {"cmp8xchg16."#s".nta", CMPXCHG16_P (0x20|CMPXCHG_##s, 3)}
#define CMPXCHG_ALL(s) CMPXCHGn(1, s), \
                       CMPXCHGn(2, s), \
                       CMPXCHGn(4, s), \
                       CMPXCHGn(8, s), \
                       CMP8XCHG16(s)
    CMPXCHG_ALL(acq),
    CMPXCHG_ALL(rel),
#undef CMPXCHG
#undef CMPXCHG_P
#undef CMPXCHG16
#undef CMPXCHG16_P
#undef CMPXCHG_acq
#undef CMPXCHG_rel
#undef CMPXCHG_1
#undef CMPXCHG_2
#undef CMPXCHG_4
#undef CMPXCHG_8
#undef CMPXCHGn
#undef CMPXCHG16
#undef CMPXCHG_ALL
    {"xchg1",		  M, OpMXX6aHint (4, 0, 1, 0x08, 0), {R1, MR3, R2}, EMPTY},
    {"xchg1.nt1",	  M, OpMXX6aHint (4, 0, 1, 0x08, 1), {R1, MR3, R2}, EMPTY},
    {"xchg1.nta",	  M, OpMXX6aHint (4, 0, 1, 0x08, 3), {R1, MR3, R2}, EMPTY},
    {"xchg2",		  M, OpMXX6aHint (4, 0, 1, 0x09, 0), {R1, MR3, R2}, EMPTY},
    {"xchg2.nt1",	  M, OpMXX6aHint (4, 0, 1, 0x09, 1), {R1, MR3, R2}, EMPTY},
    {"xchg2.nta",	  M, OpMXX6aHint (4, 0, 1, 0x09, 3), {R1, MR3, R2}, EMPTY},
    {"xchg4",		  M, OpMXX6aHint (4, 0, 1, 0x0a, 0), {R1, MR3, R2}, EMPTY},
    {"xchg4.nt1",	  M, OpMXX6aHint (4, 0, 1, 0x0a, 1), {R1, MR3, R2}, EMPTY},
    {"xchg4.nta",	  M, OpMXX6aHint (4, 0, 1, 0x0a, 3), {R1, MR3, R2}, EMPTY},
    {"xchg8",		  M, OpMXX6aHint (4, 0, 1, 0x0b, 0), {R1, MR3, R2}, EMPTY},
    {"xchg8.nt1",	  M, OpMXX6aHint (4, 0, 1, 0x0b, 1), {R1, MR3, R2}, EMPTY},
    {"xchg8.nta",	  M, OpMXX6aHint (4, 0, 1, 0x0b, 3), {R1, MR3, R2}, EMPTY},

    {"fetchadd4.acq",	  M, OpMXX6aHint (4, 0, 1, 0x12, 0), {R1, MR3, INC3}, EMPTY},
    {"fetchadd4.acq.nt1", M, OpMXX6aHint (4, 0, 1, 0x12, 1), {R1, MR3, INC3}, EMPTY},
    {"fetchadd4.acq.nta", M, OpMXX6aHint (4, 0, 1, 0x12, 3), {R1, MR3, INC3}, EMPTY},
    {"fetchadd8.acq",	  M, OpMXX6aHint (4, 0, 1, 0x13, 0), {R1, MR3, INC3}, EMPTY},
    {"fetchadd8.acq.nt1", M, OpMXX6aHint (4, 0, 1, 0x13, 1), {R1, MR3, INC3}, EMPTY},
    {"fetchadd8.acq.nta", M, OpMXX6aHint (4, 0, 1, 0x13, 3), {R1, MR3, INC3}, EMPTY},
    {"fetchadd4.rel",	  M, OpMXX6aHint (4, 0, 1, 0x16, 0), {R1, MR3, INC3}, EMPTY},
    {"fetchadd4.rel.nt1", M, OpMXX6aHint (4, 0, 1, 0x16, 1), {R1, MR3, INC3}, EMPTY},
    {"fetchadd4.rel.nta", M, OpMXX6aHint (4, 0, 1, 0x16, 3), {R1, MR3, INC3}, EMPTY},
    {"fetchadd8.rel",	  M, OpMXX6aHint (4, 0, 1, 0x17, 0), {R1, MR3, INC3}, EMPTY},
    {"fetchadd8.rel.nt1", M, OpMXX6aHint (4, 0, 1, 0x17, 1), {R1, MR3, INC3}, EMPTY},
    {"fetchadd8.rel.nta", M, OpMXX6aHint (4, 0, 1, 0x17, 3), {R1, MR3, INC3}, EMPTY},

    {"getf.sig",	  M, OpMXX6a (4, 0, 1, 0x1c), {R1, F2}, EMPTY},
    {"getf.exp",	  M, OpMXX6a (4, 0, 1, 0x1d), {R1, F2}, EMPTY},
    {"getf.s",		  M, OpMXX6a (4, 0, 1, 0x1e), {R1, F2}, EMPTY},
    {"getf.d",		  M, OpMXX6a (4, 0, 1, 0x1f), {R1, F2}, EMPTY},

    /* Integer load w/increment by immediate.  */
#define LDINCIMMED(c,h) M, OpX6aHint (5, c, h), {R1, MR3, IMM9b}, POSTINC, 0, NULL
    {"ld1",		LDINCIMMED (0x00, 0)},
    {"ld1.nt1",		LDINCIMMED (0x00, 1)},
    {"ld1.nta",		LDINCIMMED (0x00, 3)},
    {"ld2",		LDINCIMMED (0x01, 0)},
    {"ld2.nt1",		LDINCIMMED (0x01, 1)},
    {"ld2.nta",		LDINCIMMED (0x01, 3)},
    {"ld4",		LDINCIMMED (0x02, 0)},
    {"ld4.nt1",		LDINCIMMED (0x02, 1)},
    {"ld4.nta",		LDINCIMMED (0x02, 3)},
    {"ld8",		LDINCIMMED (0x03, 0)},
    {"ld8.nt1",		LDINCIMMED (0x03, 1)},
    {"ld8.nta",		LDINCIMMED (0x03, 3)},
    {"ld1.s",		LDINCIMMED (0x04, 0)},
    {"ld1.s.nt1",	LDINCIMMED (0x04, 1)},
    {"ld1.s.nta",	LDINCIMMED (0x04, 3)},
    {"ld2.s",		LDINCIMMED (0x05, 0)},
    {"ld2.s.nt1",	LDINCIMMED (0x05, 1)},
    {"ld2.s.nta",	LDINCIMMED (0x05, 3)},
    {"ld4.s",		LDINCIMMED (0x06, 0)},
    {"ld4.s.nt1",	LDINCIMMED (0x06, 1)},
    {"ld4.s.nta",	LDINCIMMED (0x06, 3)},
    {"ld8.s",		LDINCIMMED (0x07, 0)},
    {"ld8.s.nt1",	LDINCIMMED (0x07, 1)},
    {"ld8.s.nta",	LDINCIMMED (0x07, 3)},
    {"ld1.a",		LDINCIMMED (0x08, 0)},
    {"ld1.a.nt1",	LDINCIMMED (0x08, 1)},
    {"ld1.a.nta",	LDINCIMMED (0x08, 3)},
    {"ld2.a",		LDINCIMMED (0x09, 0)},
    {"ld2.a.nt1",	LDINCIMMED (0x09, 1)},
    {"ld2.a.nta",	LDINCIMMED (0x09, 3)},
    {"ld4.a",		LDINCIMMED (0x0a, 0)},
    {"ld4.a.nt1",	LDINCIMMED (0x0a, 1)},
    {"ld4.a.nta",	LDINCIMMED (0x0a, 3)},
    {"ld8.a",		LDINCIMMED (0x0b, 0)},
    {"ld8.a.nt1",	LDINCIMMED (0x0b, 1)},
    {"ld8.a.nta",	LDINCIMMED (0x0b, 3)},
    {"ld1.sa",		LDINCIMMED (0x0c, 0)},
    {"ld1.sa.nt1",	LDINCIMMED (0x0c, 1)},
    {"ld1.sa.nta",	LDINCIMMED (0x0c, 3)},
    {"ld2.sa",		LDINCIMMED (0x0d, 0)},
    {"ld2.sa.nt1",	LDINCIMMED (0x0d, 1)},
    {"ld2.sa.nta",	LDINCIMMED (0x0d, 3)},
    {"ld4.sa",		LDINCIMMED (0x0e, 0)},
    {"ld4.sa.nt1",	LDINCIMMED (0x0e, 1)},
    {"ld4.sa.nta",	LDINCIMMED (0x0e, 3)},
    {"ld8.sa",		LDINCIMMED (0x0f, 0)},
    {"ld8.sa.nt1",	LDINCIMMED (0x0f, 1)},
    {"ld8.sa.nta",	LDINCIMMED (0x0f, 3)},
    {"ld1.bias",	LDINCIMMED (0x10, 0)},
    {"ld1.bias.nt1",	LDINCIMMED (0x10, 1)},
    {"ld1.bias.nta",	LDINCIMMED (0x10, 3)},
    {"ld2.bias",	LDINCIMMED (0x11, 0)},
    {"ld2.bias.nt1",	LDINCIMMED (0x11, 1)},
    {"ld2.bias.nta",	LDINCIMMED (0x11, 3)},
    {"ld4.bias",	LDINCIMMED (0x12, 0)},
    {"ld4.bias.nt1",	LDINCIMMED (0x12, 1)},
    {"ld4.bias.nta",	LDINCIMMED (0x12, 3)},
    {"ld8.bias",	LDINCIMMED (0x13, 0)},
    {"ld8.bias.nt1",	LDINCIMMED (0x13, 1)},
    {"ld8.bias.nta",	LDINCIMMED (0x13, 3)},
    {"ld1.acq",		LDINCIMMED (0x14, 0)},
    {"ld1.acq.nt1",	LDINCIMMED (0x14, 1)},
    {"ld1.acq.nta",	LDINCIMMED (0x14, 3)},
    {"ld2.acq",		LDINCIMMED (0x15, 0)},
    {"ld2.acq.nt1",	LDINCIMMED (0x15, 1)},
    {"ld2.acq.nta",	LDINCIMMED (0x15, 3)},
    {"ld4.acq",		LDINCIMMED (0x16, 0)},
    {"ld4.acq.nt1",	LDINCIMMED (0x16, 1)},
    {"ld4.acq.nta",	LDINCIMMED (0x16, 3)},
    {"ld8.acq",		LDINCIMMED (0x17, 0)},
    {"ld8.acq.nt1",	LDINCIMMED (0x17, 1)},
    {"ld8.acq.nta",	LDINCIMMED (0x17, 3)},
    {"ld8.fill",	LDINCIMMED (0x1b, 0)},
    {"ld8.fill.nt1",	LDINCIMMED (0x1b, 1)},
    {"ld8.fill.nta",	LDINCIMMED (0x1b, 3)},
    {"ld1.c.clr",	LDINCIMMED (0x20, 0)},
    {"ld1.c.clr.nt1",	LDINCIMMED (0x20, 1)},
    {"ld1.c.clr.nta",	LDINCIMMED (0x20, 3)},
    {"ld2.c.clr",	LDINCIMMED (0x21, 0)},
    {"ld2.c.clr.nt1",	LDINCIMMED (0x21, 1)},
    {"ld2.c.clr.nta",	LDINCIMMED (0x21, 3)},
    {"ld4.c.clr",	LDINCIMMED (0x22, 0)},
    {"ld4.c.clr.nt1",	LDINCIMMED (0x22, 1)},
    {"ld4.c.clr.nta",	LDINCIMMED (0x22, 3)},
    {"ld8.c.clr",	LDINCIMMED (0x23, 0)},
    {"ld8.c.clr.nt1",	LDINCIMMED (0x23, 1)},
    {"ld8.c.clr.nta",	LDINCIMMED (0x23, 3)},
    {"ld1.c.nc",	LDINCIMMED (0x24, 0)},
    {"ld1.c.nc.nt1",	LDINCIMMED (0x24, 1)},
    {"ld1.c.nc.nta",	LDINCIMMED (0x24, 3)},
    {"ld2.c.nc",	LDINCIMMED (0x25, 0)},
    {"ld2.c.nc.nt1",	LDINCIMMED (0x25, 1)},
    {"ld2.c.nc.nta",	LDINCIMMED (0x25, 3)},
    {"ld4.c.nc",	LDINCIMMED (0x26, 0)},
    {"ld4.c.nc.nt1",	LDINCIMMED (0x26, 1)},
    {"ld4.c.nc.nta",	LDINCIMMED (0x26, 3)},
    {"ld8.c.nc",	LDINCIMMED (0x27, 0)},
    {"ld8.c.nc.nt1",	LDINCIMMED (0x27, 1)},
    {"ld8.c.nc.nta",	LDINCIMMED (0x27, 3)},
    {"ld1.c.clr.acq",	  LDINCIMMED (0x28, 0)},
    {"ld1.c.clr.acq.nt1", LDINCIMMED (0x28, 1)},
    {"ld1.c.clr.acq.nta", LDINCIMMED (0x28, 3)},
    {"ld2.c.clr.acq",	  LDINCIMMED (0x29, 0)},
    {"ld2.c.clr.acq.nt1", LDINCIMMED (0x29, 1)},
    {"ld2.c.clr.acq.nta", LDINCIMMED (0x29, 3)},
    {"ld4.c.clr.acq",	  LDINCIMMED (0x2a, 0)},
    {"ld4.c.clr.acq.nt1", LDINCIMMED (0x2a, 1)},
    {"ld4.c.clr.acq.nta", LDINCIMMED (0x2a, 3)},
    {"ld8.c.clr.acq",	  LDINCIMMED (0x2b, 0)},
    {"ld8.c.clr.acq.nt1", LDINCIMMED (0x2b, 1)},
    {"ld8.c.clr.acq.nta", LDINCIMMED (0x2b, 3)},
#undef LDINCIMMED

    /* Store w/increment by immediate.  */
#define STINCIMMED(c,h) M, OpX6aHint (5, c, h), {MR3, R2, IMM9a}, POSTINC, 0, NULL
    {"st1",		STINCIMMED (0x30, 0)},
    {"st1.nta",		STINCIMMED (0x30, 3)},
    {"st2",		STINCIMMED (0x31, 0)},
    {"st2.nta",		STINCIMMED (0x31, 3)},
    {"st4",		STINCIMMED (0x32, 0)},
    {"st4.nta",		STINCIMMED (0x32, 3)},
    {"st8",		STINCIMMED (0x33, 0)},
    {"st8.nta",		STINCIMMED (0x33, 3)},
    {"st1.rel",		STINCIMMED (0x34, 0)},
    {"st1.rel.nta",	STINCIMMED (0x34, 3)},
    {"st2.rel",		STINCIMMED (0x35, 0)},
    {"st2.rel.nta",	STINCIMMED (0x35, 3)},
    {"st4.rel",		STINCIMMED (0x36, 0)},
    {"st4.rel.nta",	STINCIMMED (0x36, 3)},
    {"st8.rel",		STINCIMMED (0x37, 0)},
    {"st8.rel.nta",	STINCIMMED (0x37, 3)},
    {"st8.spill",	STINCIMMED (0x3b, 0)},
    {"st8.spill.nta",	STINCIMMED (0x3b, 3)},
#undef STINCIMMED

    /* Floating-point load.  */
    {"ldfs",		M, OpMXX6aHint (6, 0, 0, 0x02, 0), {F1, MR3}, EMPTY},
    {"ldfs.nt1",	M, OpMXX6aHint (6, 0, 0, 0x02, 1), {F1, MR3}, EMPTY},
    {"ldfs.nta",	M, OpMXX6aHint (6, 0, 0, 0x02, 3), {F1, MR3}, EMPTY},
    {"ldfd",		M, OpMXX6aHint (6, 0, 0, 0x03, 0), {F1, MR3}, EMPTY},
    {"ldfd.nt1",	M, OpMXX6aHint (6, 0, 0, 0x03, 1), {F1, MR3}, EMPTY},
    {"ldfd.nta",	M, OpMXX6aHint (6, 0, 0, 0x03, 3), {F1, MR3}, EMPTY},
    {"ldf8",		M, OpMXX6aHint (6, 0, 0, 0x01, 0), {F1, MR3}, EMPTY},
    {"ldf8.nt1",	M, OpMXX6aHint (6, 0, 0, 0x01, 1), {F1, MR3}, EMPTY},
    {"ldf8.nta",	M, OpMXX6aHint (6, 0, 0, 0x01, 3), {F1, MR3}, EMPTY},
    {"ldfe",		M, OpMXX6aHint (6, 0, 0, 0x00, 0), {F1, MR3}, EMPTY},
    {"ldfe.nt1",	M, OpMXX6aHint (6, 0, 0, 0x00, 1), {F1, MR3}, EMPTY},
    {"ldfe.nta",	M, OpMXX6aHint (6, 0, 0, 0x00, 3), {F1, MR3}, EMPTY},
    {"ldfs.s",		M, OpMXX6aHint (6, 0, 0, 0x06, 0), {F1, MR3}, EMPTY},
    {"ldfs.s.nt1",	M, OpMXX6aHint (6, 0, 0, 0x06, 1), {F1, MR3}, EMPTY},
    {"ldfs.s.nta",	M, OpMXX6aHint (6, 0, 0, 0x06, 3), {F1, MR3}, EMPTY},
    {"ldfd.s",		M, OpMXX6aHint (6, 0, 0, 0x07, 0), {F1, MR3}, EMPTY},
    {"ldfd.s.nt1",	M, OpMXX6aHint (6, 0, 0, 0x07, 1), {F1, MR3}, EMPTY},
    {"ldfd.s.nta",	M, OpMXX6aHint (6, 0, 0, 0x07, 3), {F1, MR3}, EMPTY},
    {"ldf8.s",		M, OpMXX6aHint (6, 0, 0, 0x05, 0), {F1, MR3}, EMPTY},
    {"ldf8.s.nt1",	M, OpMXX6aHint (6, 0, 0, 0x05, 1), {F1, MR3}, EMPTY},
    {"ldf8.s.nta",	M, OpMXX6aHint (6, 0, 0, 0x05, 3), {F1, MR3}, EMPTY},
    {"ldfe.s",		M, OpMXX6aHint (6, 0, 0, 0x04, 0), {F1, MR3}, EMPTY},
    {"ldfe.s.nt1",	M, OpMXX6aHint (6, 0, 0, 0x04, 1), {F1, MR3}, EMPTY},
    {"ldfe.s.nta",	M, OpMXX6aHint (6, 0, 0, 0x04, 3), {F1, MR3}, EMPTY},
    {"ldfs.a",		M, OpMXX6aHint (6, 0, 0, 0x0a, 0), {F1, MR3}, EMPTY},
    {"ldfs.a.nt1",	M, OpMXX6aHint (6, 0, 0, 0x0a, 1), {F1, MR3}, EMPTY},
    {"ldfs.a.nta",	M, OpMXX6aHint (6, 0, 0, 0x0a, 3), {F1, MR3}, EMPTY},
    {"ldfd.a",		M, OpMXX6aHint (6, 0, 0, 0x0b, 0), {F1, MR3}, EMPTY},
    {"ldfd.a.nt1",	M, OpMXX6aHint (6, 0, 0, 0x0b, 1), {F1, MR3}, EMPTY},
    {"ldfd.a.nta",	M, OpMXX6aHint (6, 0, 0, 0x0b, 3), {F1, MR3}, EMPTY},
    {"ldf8.a",		M, OpMXX6aHint (6, 0, 0, 0x09, 0), {F1, MR3}, EMPTY},
    {"ldf8.a.nt1",	M, OpMXX6aHint (6, 0, 0, 0x09, 1), {F1, MR3}, EMPTY},
    {"ldf8.a.nta",	M, OpMXX6aHint (6, 0, 0, 0x09, 3), {F1, MR3}, EMPTY},
    {"ldfe.a",		M, OpMXX6aHint (6, 0, 0, 0x08, 0), {F1, MR3}, EMPTY},
    {"ldfe.a.nt1",	M, OpMXX6aHint (6, 0, 0, 0x08, 1), {F1, MR3}, EMPTY},
    {"ldfe.a.nta",	M, OpMXX6aHint (6, 0, 0, 0x08, 3), {F1, MR3}, EMPTY},
    {"ldfs.sa",		M, OpMXX6aHint (6, 0, 0, 0x0e, 0), {F1, MR3}, EMPTY},
    {"ldfs.sa.nt1",	M, OpMXX6aHint (6, 0, 0, 0x0e, 1), {F1, MR3}, EMPTY},
    {"ldfs.sa.nta",	M, OpMXX6aHint (6, 0, 0, 0x0e, 3), {F1, MR3}, EMPTY},
    {"ldfd.sa",		M, OpMXX6aHint (6, 0, 0, 0x0f, 0), {F1, MR3}, EMPTY},
    {"ldfd.sa.nt1",	M, OpMXX6aHint (6, 0, 0, 0x0f, 1), {F1, MR3}, EMPTY},
    {"ldfd.sa.nta",	M, OpMXX6aHint (6, 0, 0, 0x0f, 3), {F1, MR3}, EMPTY},
    {"ldf8.sa",		M, OpMXX6aHint (6, 0, 0, 0x0d, 0), {F1, MR3}, EMPTY},
    {"ldf8.sa.nt1",	M, OpMXX6aHint (6, 0, 0, 0x0d, 1), {F1, MR3}, EMPTY},
    {"ldf8.sa.nta",	M, OpMXX6aHint (6, 0, 0, 0x0d, 3), {F1, MR3}, EMPTY},
    {"ldfe.sa",		M, OpMXX6aHint (6, 0, 0, 0x0c, 0), {F1, MR3}, EMPTY},
    {"ldfe.sa.nt1",	M, OpMXX6aHint (6, 0, 0, 0x0c, 1), {F1, MR3}, EMPTY},
    {"ldfe.sa.nta",	M, OpMXX6aHint (6, 0, 0, 0x0c, 3), {F1, MR3}, EMPTY},
    {"ldf.fill",	M, OpMXX6aHint (6, 0, 0, 0x1b, 0), {F1, MR3}, EMPTY},
    {"ldf.fill.nt1",	M, OpMXX6aHint (6, 0, 0, 0x1b, 1), {F1, MR3}, EMPTY},
    {"ldf.fill.nta",	M, OpMXX6aHint (6, 0, 0, 0x1b, 3), {F1, MR3}, EMPTY},
    {"ldfs.c.clr",	M, OpMXX6aHint (6, 0, 0, 0x22, 0), {F1, MR3}, EMPTY},
    {"ldfs.c.clr.nt1",	M, OpMXX6aHint (6, 0, 0, 0x22, 1), {F1, MR3}, EMPTY},
    {"ldfs.c.clr.nta",	M, OpMXX6aHint (6, 0, 0, 0x22, 3), {F1, MR3}, EMPTY},
    {"ldfd.c.clr",	M, OpMXX6aHint (6, 0, 0, 0x23, 0), {F1, MR3}, EMPTY},
    {"ldfd.c.clr.nt1",	M, OpMXX6aHint (6, 0, 0, 0x23, 1), {F1, MR3}, EMPTY},
    {"ldfd.c.clr.nta",	M, OpMXX6aHint (6, 0, 0, 0x23, 3), {F1, MR3}, EMPTY},
    {"ldf8.c.clr",	M, OpMXX6aHint (6, 0, 0, 0x21, 0), {F1, MR3}, EMPTY},
    {"ldf8.c.clr.nt1",	M, OpMXX6aHint (6, 0, 0, 0x21, 1), {F1, MR3}, EMPTY},
    {"ldf8.c.clr.nta",	M, OpMXX6aHint (6, 0, 0, 0x21, 3), {F1, MR3}, EMPTY},
    {"ldfe.c.clr",	M, OpMXX6aHint (6, 0, 0, 0x20, 0), {F1, MR3}, EMPTY},
    {"ldfe.c.clr.nt1",	M, OpMXX6aHint (6, 0, 0, 0x20, 1), {F1, MR3}, EMPTY},
    {"ldfe.c.clr.nta",	M, OpMXX6aHint (6, 0, 0, 0x20, 3), {F1, MR3}, EMPTY},
    {"ldfs.c.nc",	M, OpMXX6aHint (6, 0, 0, 0x26, 0), {F1, MR3}, EMPTY},
    {"ldfs.c.nc.nt1",	M, OpMXX6aHint (6, 0, 0, 0x26, 1), {F1, MR3}, EMPTY},
    {"ldfs.c.nc.nta",	M, OpMXX6aHint (6, 0, 0, 0x26, 3), {F1, MR3}, EMPTY},
    {"ldfd.c.nc",	M, OpMXX6aHint (6, 0, 0, 0x27, 0), {F1, MR3}, EMPTY},
    {"ldfd.c.nc.nt1",	M, OpMXX6aHint (6, 0, 0, 0x27, 1), {F1, MR3}, EMPTY},
    {"ldfd.c.nc.nta",	M, OpMXX6aHint (6, 0, 0, 0x27, 3), {F1, MR3}, EMPTY},
    {"ldf8.c.nc",	M, OpMXX6aHint (6, 0, 0, 0x25, 0), {F1, MR3}, EMPTY},
    {"ldf8.c.nc.nt1",	M, OpMXX6aHint (6, 0, 0, 0x25, 1), {F1, MR3}, EMPTY},
    {"ldf8.c.nc.nta",	M, OpMXX6aHint (6, 0, 0, 0x25, 3), {F1, MR3}, EMPTY},
    {"ldfe.c.nc",	M, OpMXX6aHint (6, 0, 0, 0x24, 0), {F1, MR3}, EMPTY},
    {"ldfe.c.nc.nt1",	M, OpMXX6aHint (6, 0, 0, 0x24, 1), {F1, MR3}, EMPTY},
    {"ldfe.c.nc.nta",	M, OpMXX6aHint (6, 0, 0, 0x24, 3), {F1, MR3}, EMPTY},

    /* Floating-point load w/increment by register.  */
#define FLDINCREG(c,h) M, OpMXX6aHint (6, 1, 0, c, h), {F1, MR3, R2}, POSTINC, 0, NULL
    {"ldfs",		FLDINCREG (0x02, 0)},
    {"ldfs.nt1",	FLDINCREG (0x02, 1)},
    {"ldfs.nta",	FLDINCREG (0x02, 3)},
    {"ldfd",		FLDINCREG (0x03, 0)},
    {"ldfd.nt1",	FLDINCREG (0x03, 1)},
    {"ldfd.nta",	FLDINCREG (0x03, 3)},
    {"ldf8",		FLDINCREG (0x01, 0)},
    {"ldf8.nt1",	FLDINCREG (0x01, 1)},
    {"ldf8.nta",	FLDINCREG (0x01, 3)},
    {"ldfe",		FLDINCREG (0x00, 0)},
    {"ldfe.nt1",	FLDINCREG (0x00, 1)},
    {"ldfe.nta",	FLDINCREG (0x00, 3)},
    {"ldfs.s",		FLDINCREG (0x06, 0)},
    {"ldfs.s.nt1",	FLDINCREG (0x06, 1)},
    {"ldfs.s.nta",	FLDINCREG (0x06, 3)},
    {"ldfd.s",		FLDINCREG (0x07, 0)},
    {"ldfd.s.nt1",	FLDINCREG (0x07, 1)},
    {"ldfd.s.nta",	FLDINCREG (0x07, 3)},
    {"ldf8.s",		FLDINCREG (0x05, 0)},
    {"ldf8.s.nt1",	FLDINCREG (0x05, 1)},
    {"ldf8.s.nta",	FLDINCREG (0x05, 3)},
    {"ldfe.s",		FLDINCREG (0x04, 0)},
    {"ldfe.s.nt1",	FLDINCREG (0x04, 1)},
    {"ldfe.s.nta",	FLDINCREG (0x04, 3)},
    {"ldfs.a",		FLDINCREG (0x0a, 0)},
    {"ldfs.a.nt1",	FLDINCREG (0x0a, 1)},
    {"ldfs.a.nta",	FLDINCREG (0x0a, 3)},
    {"ldfd.a",		FLDINCREG (0x0b, 0)},
    {"ldfd.a.nt1",	FLDINCREG (0x0b, 1)},
    {"ldfd.a.nta",	FLDINCREG (0x0b, 3)},
    {"ldf8.a",		FLDINCREG (0x09, 0)},
    {"ldf8.a.nt1",	FLDINCREG (0x09, 1)},
    {"ldf8.a.nta",	FLDINCREG (0x09, 3)},
    {"ldfe.a",		FLDINCREG (0x08, 0)},
    {"ldfe.a.nt1",	FLDINCREG (0x08, 1)},
    {"ldfe.a.nta",	FLDINCREG (0x08, 3)},
    {"ldfs.sa",		FLDINCREG (0x0e, 0)},
    {"ldfs.sa.nt1",	FLDINCREG (0x0e, 1)},
    {"ldfs.sa.nta",	FLDINCREG (0x0e, 3)},
    {"ldfd.sa",		FLDINCREG (0x0f, 0)},
    {"ldfd.sa.nt1",	FLDINCREG (0x0f, 1)},
    {"ldfd.sa.nta",	FLDINCREG (0x0f, 3)},
    {"ldf8.sa",		FLDINCREG (0x0d, 0)},
    {"ldf8.sa.nt1",	FLDINCREG (0x0d, 1)},
    {"ldf8.sa.nta",	FLDINCREG (0x0d, 3)},
    {"ldfe.sa",		FLDINCREG (0x0c, 0)},
    {"ldfe.sa.nt1",	FLDINCREG (0x0c, 1)},
    {"ldfe.sa.nta",	FLDINCREG (0x0c, 3)},
    {"ldf.fill",	FLDINCREG (0x1b, 0)},
    {"ldf.fill.nt1",	FLDINCREG (0x1b, 1)},
    {"ldf.fill.nta",	FLDINCREG (0x1b, 3)},
    {"ldfs.c.clr",	FLDINCREG (0x22, 0)},
    {"ldfs.c.clr.nt1",	FLDINCREG (0x22, 1)},
    {"ldfs.c.clr.nta",	FLDINCREG (0x22, 3)},
    {"ldfd.c.clr",	FLDINCREG (0x23, 0)},
    {"ldfd.c.clr.nt1",	FLDINCREG (0x23, 1)},
    {"ldfd.c.clr.nta",	FLDINCREG (0x23, 3)},
    {"ldf8.c.clr",	FLDINCREG (0x21, 0)},
    {"ldf8.c.clr.nt1",	FLDINCREG (0x21, 1)},
    {"ldf8.c.clr.nta",	FLDINCREG (0x21, 3)},
    {"ldfe.c.clr",	FLDINCREG (0x20, 0)},
    {"ldfe.c.clr.nt1",	FLDINCREG (0x20, 1)},
    {"ldfe.c.clr.nta",	FLDINCREG (0x20, 3)},
    {"ldfs.c.nc",	FLDINCREG (0x26, 0)},
    {"ldfs.c.nc.nt1",	FLDINCREG (0x26, 1)},
    {"ldfs.c.nc.nta",	FLDINCREG (0x26, 3)},
    {"ldfd.c.nc",	FLDINCREG (0x27, 0)},
    {"ldfd.c.nc.nt1",	FLDINCREG (0x27, 1)},
    {"ldfd.c.nc.nta",	FLDINCREG (0x27, 3)},
    {"ldf8.c.nc",	FLDINCREG (0x25, 0)},
    {"ldf8.c.nc.nt1",	FLDINCREG (0x25, 1)},
    {"ldf8.c.nc.nta",	FLDINCREG (0x25, 3)},
    {"ldfe.c.nc",	FLDINCREG (0x24, 0)},
    {"ldfe.c.nc.nt1",	FLDINCREG (0x24, 1)},
    {"ldfe.c.nc.nta",	FLDINCREG (0x24, 3)},
#undef FLDINCREG

    /* Floating-point store.  */
    {"stfs",		M, OpMXX6aHint (6, 0, 0, 0x32, 0), {MR3, F2}, EMPTY},
    {"stfs.nta",	M, OpMXX6aHint (6, 0, 0, 0x32, 3), {MR3, F2}, EMPTY},
    {"stfd",		M, OpMXX6aHint (6, 0, 0, 0x33, 0), {MR3, F2}, EMPTY},
    {"stfd.nta",	M, OpMXX6aHint (6, 0, 0, 0x33, 3), {MR3, F2}, EMPTY},
    {"stf8",		M, OpMXX6aHint (6, 0, 0, 0x31, 0), {MR3, F2}, EMPTY},
    {"stf8.nta",	M, OpMXX6aHint (6, 0, 0, 0x31, 3), {MR3, F2}, EMPTY},
    {"stfe",		M, OpMXX6aHint (6, 0, 0, 0x30, 0), {MR3, F2}, EMPTY},
    {"stfe.nta",	M, OpMXX6aHint (6, 0, 0, 0x30, 3), {MR3, F2}, EMPTY},
    {"stf.spill",	M, OpMXX6aHint (6, 0, 0, 0x3b, 0), {MR3, F2}, EMPTY},
    {"stf.spill.nta",	M, OpMXX6aHint (6, 0, 0, 0x3b, 3), {MR3, F2}, EMPTY},

    /* Floating-point load pair.  */
    {"ldfps",		M2, OpMXX6aHint (6, 0, 1, 0x02, 0), {F1, F2, MR3}, EMPTY},
    {"ldfps.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x02, 1), {F1, F2, MR3}, EMPTY},
    {"ldfps.nta",	M2, OpMXX6aHint (6, 0, 1, 0x02, 3), {F1, F2, MR3}, EMPTY},
    {"ldfpd",		M2, OpMXX6aHint (6, 0, 1, 0x03, 0), {F1, F2, MR3}, EMPTY},
    {"ldfpd.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x03, 1), {F1, F2, MR3}, EMPTY},
    {"ldfpd.nta",	M2, OpMXX6aHint (6, 0, 1, 0x03, 3), {F1, F2, MR3}, EMPTY},
    {"ldfp8",		M2, OpMXX6aHint (6, 0, 1, 0x01, 0), {F1, F2, MR3}, EMPTY},
    {"ldfp8.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x01, 1), {F1, F2, MR3}, EMPTY},
    {"ldfp8.nta",	M2, OpMXX6aHint (6, 0, 1, 0x01, 3), {F1, F2, MR3}, EMPTY},
    {"ldfps.s",		M2, OpMXX6aHint (6, 0, 1, 0x06, 0), {F1, F2, MR3}, EMPTY},
    {"ldfps.s.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x06, 1), {F1, F2, MR3}, EMPTY},
    {"ldfps.s.nta",	M2, OpMXX6aHint (6, 0, 1, 0x06, 3), {F1, F2, MR3}, EMPTY},
    {"ldfpd.s",		M2, OpMXX6aHint (6, 0, 1, 0x07, 0), {F1, F2, MR3}, EMPTY},
    {"ldfpd.s.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x07, 1), {F1, F2, MR3}, EMPTY},
    {"ldfpd.s.nta",	M2, OpMXX6aHint (6, 0, 1, 0x07, 3), {F1, F2, MR3}, EMPTY},
    {"ldfp8.s",		M2, OpMXX6aHint (6, 0, 1, 0x05, 0), {F1, F2, MR3}, EMPTY},
    {"ldfp8.s.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x05, 1), {F1, F2, MR3}, EMPTY},
    {"ldfp8.s.nta",	M2, OpMXX6aHint (6, 0, 1, 0x05, 3), {F1, F2, MR3}, EMPTY},
    {"ldfps.a",		M2, OpMXX6aHint (6, 0, 1, 0x0a, 0), {F1, F2, MR3}, EMPTY},
    {"ldfps.a.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x0a, 1), {F1, F2, MR3}, EMPTY},
    {"ldfps.a.nta",	M2, OpMXX6aHint (6, 0, 1, 0x0a, 3), {F1, F2, MR3}, EMPTY},
    {"ldfpd.a",		M2, OpMXX6aHint (6, 0, 1, 0x0b, 0), {F1, F2, MR3}, EMPTY},
    {"ldfpd.a.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x0b, 1), {F1, F2, MR3}, EMPTY},
    {"ldfpd.a.nta",	M2, OpMXX6aHint (6, 0, 1, 0x0b, 3), {F1, F2, MR3}, EMPTY},
    {"ldfp8.a",		M2, OpMXX6aHint (6, 0, 1, 0x09, 0), {F1, F2, MR3}, EMPTY},
    {"ldfp8.a.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x09, 1), {F1, F2, MR3}, EMPTY},
    {"ldfp8.a.nta",	M2, OpMXX6aHint (6, 0, 1, 0x09, 3), {F1, F2, MR3}, EMPTY},
    {"ldfps.sa",	M2, OpMXX6aHint (6, 0, 1, 0x0e, 0), {F1, F2, MR3}, EMPTY},
    {"ldfps.sa.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x0e, 1), {F1, F2, MR3}, EMPTY},
    {"ldfps.sa.nta",	M2, OpMXX6aHint (6, 0, 1, 0x0e, 3), {F1, F2, MR3}, EMPTY},
    {"ldfpd.sa",	M2, OpMXX6aHint (6, 0, 1, 0x0f, 0), {F1, F2, MR3}, EMPTY},
    {"ldfpd.sa.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x0f, 1), {F1, F2, MR3}, EMPTY},
    {"ldfpd.sa.nta",	M2, OpMXX6aHint (6, 0, 1, 0x0f, 3), {F1, F2, MR3}, EMPTY},
    {"ldfp8.sa",	M2, OpMXX6aHint (6, 0, 1, 0x0d, 0), {F1, F2, MR3}, EMPTY},
    {"ldfp8.sa.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x0d, 1), {F1, F2, MR3}, EMPTY},
    {"ldfp8.sa.nta",	M2, OpMXX6aHint (6, 0, 1, 0x0d, 3), {F1, F2, MR3}, EMPTY},
    {"ldfps.c.clr",	M2, OpMXX6aHint (6, 0, 1, 0x22, 0), {F1, F2, MR3}, EMPTY},
    {"ldfps.c.clr.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x22, 1), {F1, F2, MR3}, EMPTY},
    {"ldfps.c.clr.nta",	M2, OpMXX6aHint (6, 0, 1, 0x22, 3), {F1, F2, MR3}, EMPTY},
    {"ldfpd.c.clr",	M2, OpMXX6aHint (6, 0, 1, 0x23, 0), {F1, F2, MR3}, EMPTY},
    {"ldfpd.c.clr.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x23, 1), {F1, F2, MR3}, EMPTY},
    {"ldfpd.c.clr.nta",	M2, OpMXX6aHint (6, 0, 1, 0x23, 3), {F1, F2, MR3}, EMPTY},
    {"ldfp8.c.clr",	M2, OpMXX6aHint (6, 0, 1, 0x21, 0), {F1, F2, MR3}, EMPTY},
    {"ldfp8.c.clr.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x21, 1), {F1, F2, MR3}, EMPTY},
    {"ldfp8.c.clr.nta",	M2, OpMXX6aHint (6, 0, 1, 0x21, 3), {F1, F2, MR3}, EMPTY},
    {"ldfps.c.nc",	M2, OpMXX6aHint (6, 0, 1, 0x26, 0), {F1, F2, MR3}, EMPTY},
    {"ldfps.c.nc.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x26, 1), {F1, F2, MR3}, EMPTY},
    {"ldfps.c.nc.nta",	M2, OpMXX6aHint (6, 0, 1, 0x26, 3), {F1, F2, MR3}, EMPTY},
    {"ldfpd.c.nc",	M2, OpMXX6aHint (6, 0, 1, 0x27, 0), {F1, F2, MR3}, EMPTY},
    {"ldfpd.c.nc.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x27, 1), {F1, F2, MR3}, EMPTY},
    {"ldfpd.c.nc.nta",	M2, OpMXX6aHint (6, 0, 1, 0x27, 3), {F1, F2, MR3}, EMPTY},
    {"ldfp8.c.nc",	M2, OpMXX6aHint (6, 0, 1, 0x25, 0), {F1, F2, MR3}, EMPTY},
    {"ldfp8.c.nc.nt1",	M2, OpMXX6aHint (6, 0, 1, 0x25, 1), {F1, F2, MR3}, EMPTY},
    {"ldfp8.c.nc.nta",	M2, OpMXX6aHint (6, 0, 1, 0x25, 3), {F1, F2, MR3}, EMPTY},

    /* Floating-point load pair w/increment by immediate.  */
#define LD(a,b,c) M2, OpMXX6aHint (6, 1, 1, a, b), {F1, F2, MR3, c}, POSTINC, 0, NULL
    {"ldfps",		LD (0x02, 0, C8)},
    {"ldfps.nt1",	LD (0x02, 1, C8)},
    {"ldfps.nta",	LD (0x02, 3, C8)},
    {"ldfpd",		LD (0x03, 0, C16)},
    {"ldfpd.nt1",	LD (0x03, 1, C16)},
    {"ldfpd.nta",	LD (0x03, 3, C16)},
    {"ldfp8",		LD (0x01, 0, C16)},
    {"ldfp8.nt1",	LD (0x01, 1, C16)},
    {"ldfp8.nta",	LD (0x01, 3, C16)},
    {"ldfps.s",		LD (0x06, 0, C8)},
    {"ldfps.s.nt1",	LD (0x06, 1, C8)},
    {"ldfps.s.nta",	LD (0x06, 3, C8)},
    {"ldfpd.s",		LD (0x07, 0, C16)},
    {"ldfpd.s.nt1",	LD (0x07, 1, C16)},
    {"ldfpd.s.nta",	LD (0x07, 3, C16)},
    {"ldfp8.s",		LD (0x05, 0, C16)},
    {"ldfp8.s.nt1",	LD (0x05, 1, C16)},
    {"ldfp8.s.nta",	LD (0x05, 3, C16)},
    {"ldfps.a",		LD (0x0a, 0, C8)},
    {"ldfps.a.nt1",	LD (0x0a, 1, C8)},
    {"ldfps.a.nta",	LD (0x0a, 3, C8)},
    {"ldfpd.a",		LD (0x0b, 0, C16)},
    {"ldfpd.a.nt1",	LD (0x0b, 1, C16)},
    {"ldfpd.a.nta",	LD (0x0b, 3, C16)},
    {"ldfp8.a",		LD (0x09, 0, C16)},
    {"ldfp8.a.nt1",	LD (0x09, 1, C16)},
    {"ldfp8.a.nta",	LD (0x09, 3, C16)},
    {"ldfps.sa",	LD (0x0e, 0, C8)},
    {"ldfps.sa.nt1",	LD (0x0e, 1, C8)},
    {"ldfps.sa.nta",	LD (0x0e, 3, C8)},
    {"ldfpd.sa",	LD (0x0f, 0, C16)},
    {"ldfpd.sa.nt1",	LD (0x0f, 1, C16)},
    {"ldfpd.sa.nta",	LD (0x0f, 3, C16)},
    {"ldfp8.sa",	LD (0x0d, 0, C16)},
    {"ldfp8.sa.nt1",	LD (0x0d, 1, C16)},
    {"ldfp8.sa.nta",	LD (0x0d, 3, C16)},
    {"ldfps.c.clr",	LD (0x22, 0, C8)},
    {"ldfps.c.clr.nt1",	LD (0x22, 1, C8)},
    {"ldfps.c.clr.nta",	LD (0x22, 3, C8)},
    {"ldfpd.c.clr",	LD (0x23, 0, C16)},
    {"ldfpd.c.clr.nt1",	LD (0x23, 1, C16)},
    {"ldfpd.c.clr.nta",	LD (0x23, 3, C16)},
    {"ldfp8.c.clr",	LD (0x21, 0, C16)},
    {"ldfp8.c.clr.nt1",	LD (0x21, 1, C16)},
    {"ldfp8.c.clr.nta",	LD (0x21, 3, C16)},
    {"ldfps.c.nc",	LD (0x26, 0, C8)},
    {"ldfps.c.nc.nt1",	LD (0x26, 1, C8)},
    {"ldfps.c.nc.nta",	LD (0x26, 3, C8)},
    {"ldfpd.c.nc",	LD (0x27, 0, C16)},
    {"ldfpd.c.nc.nt1",	LD (0x27, 1, C16)},
    {"ldfpd.c.nc.nta",	LD (0x27, 3, C16)},
    {"ldfp8.c.nc",	LD (0x25, 0, C16)},
    {"ldfp8.c.nc.nt1",	LD (0x25, 1, C16)},
    {"ldfp8.c.nc.nta",	LD (0x25, 3, C16)},
#undef LD

    /* Line prefetch.  */
    {"lfetch",			M0, OpMXX6aHint (6, 0, 0, 0x2c, 0), {MR3}, EMPTY},
    {"lfetch.nt1",		M0, OpMXX6aHint (6, 0, 0, 0x2c, 1), {MR3}, EMPTY},
    {"lfetch.nt2",		M0, OpMXX6aHint (6, 0, 0, 0x2c, 2), {MR3}, EMPTY},
    {"lfetch.nta",		M0, OpMXX6aHint (6, 0, 0, 0x2c, 3), {MR3}, EMPTY},
    {"lfetch.excl",		M0, OpMXX6aHint (6, 0, 0, 0x2d, 0), {MR3}, EMPTY},
    {"lfetch.excl.nt1",		M0, OpMXX6aHint (6, 0, 0, 0x2d, 1), {MR3}, EMPTY},
    {"lfetch.excl.nt2",		M0, OpMXX6aHint (6, 0, 0, 0x2d, 2), {MR3}, EMPTY},
    {"lfetch.excl.nta",		M0, OpMXX6aHint (6, 0, 0, 0x2d, 3), {MR3}, EMPTY},
    {"lfetch.fault",		M0, OpMXX6aHint (6, 0, 0, 0x2e, 0), {MR3}, EMPTY},
    {"lfetch.fault.nt1",	M0, OpMXX6aHint (6, 0, 0, 0x2e, 1), {MR3}, EMPTY},
    {"lfetch.fault.nt2",	M0, OpMXX6aHint (6, 0, 0, 0x2e, 2), {MR3}, EMPTY},
    {"lfetch.fault.nta",	M0, OpMXX6aHint (6, 0, 0, 0x2e, 3), {MR3}, EMPTY},
    {"lfetch.fault.excl",	M0, OpMXX6aHint (6, 0, 0, 0x2f, 0), {MR3}, EMPTY},
    {"lfetch.fault.excl.nt1",	M0, OpMXX6aHint (6, 0, 0, 0x2f, 1), {MR3}, EMPTY},
    {"lfetch.fault.excl.nt2",	M0, OpMXX6aHint (6, 0, 0, 0x2f, 2), {MR3}, EMPTY},
    {"lfetch.fault.excl.nta",	M0, OpMXX6aHint (6, 0, 0, 0x2f, 3), {MR3}, EMPTY},

    /* Line prefetch w/increment by register.  */
#define LFETCHINCREG(c,h) M0, OpMXX6aHint (6, 1, 0, c, h), {MR3, R2}, POSTINC, 0, NULL
    {"lfetch",			LFETCHINCREG (0x2c, 0)},
    {"lfetch.nt1",		LFETCHINCREG (0x2c, 1)},
    {"lfetch.nt2",		LFETCHINCREG (0x2c, 2)},
    {"lfetch.nta",		LFETCHINCREG (0x2c, 3)},
    {"lfetch.excl",		LFETCHINCREG (0x2d, 0)},
    {"lfetch.excl.nt1",		LFETCHINCREG (0x2d, 1)},
    {"lfetch.excl.nt2",		LFETCHINCREG (0x2d, 2)},
    {"lfetch.excl.nta",		LFETCHINCREG (0x2d, 3)},
    {"lfetch.fault",		LFETCHINCREG (0x2e, 0)},
    {"lfetch.fault.nt1",	LFETCHINCREG (0x2e, 1)},
    {"lfetch.fault.nt2",	LFETCHINCREG (0x2e, 2)},
    {"lfetch.fault.nta",	LFETCHINCREG (0x2e, 3)},
    {"lfetch.fault.excl",	LFETCHINCREG (0x2f, 0)},
    {"lfetch.fault.excl.nt1",	LFETCHINCREG (0x2f, 1)},
    {"lfetch.fault.excl.nt2",	LFETCHINCREG (0x2f, 2)},
    {"lfetch.fault.excl.nta",	LFETCHINCREG (0x2f, 3)},
#undef LFETCHINCREG

    /* Semaphore operations.  */
    {"setf.sig",	M, OpMXX6a (6, 0, 1, 0x1c), {F1, R2}, EMPTY},
    {"setf.exp",	M, OpMXX6a (6, 0, 1, 0x1d), {F1, R2}, EMPTY},
    {"setf.s",		M, OpMXX6a (6, 0, 1, 0x1e), {F1, R2}, EMPTY},
    {"setf.d",		M, OpMXX6a (6, 0, 1, 0x1f), {F1, R2}, EMPTY},

    /* Floating-point load w/increment by immediate.  */
#define FLDINCIMMED(c,h) M, OpX6aHint (7, c, h), {F1, MR3, IMM9b}, POSTINC, 0, NULL
    {"ldfs",		FLDINCIMMED (0x02, 0)},
    {"ldfs.nt1",	FLDINCIMMED (0x02, 1)},
    {"ldfs.nta",	FLDINCIMMED (0x02, 3)},
    {"ldfd",		FLDINCIMMED (0x03, 0)},
    {"ldfd.nt1",	FLDINCIMMED (0x03, 1)},
    {"ldfd.nta",	FLDINCIMMED (0x03, 3)},
    {"ldf8",		FLDINCIMMED (0x01, 0)},
    {"ldf8.nt1",	FLDINCIMMED (0x01, 1)},
    {"ldf8.nta",	FLDINCIMMED (0x01, 3)},
    {"ldfe",		FLDINCIMMED (0x00, 0)},
    {"ldfe.nt1",	FLDINCIMMED (0x00, 1)},
    {"ldfe.nta",	FLDINCIMMED (0x00, 3)},
    {"ldfs.s",		FLDINCIMMED (0x06, 0)},
    {"ldfs.s.nt1",	FLDINCIMMED (0x06, 1)},
    {"ldfs.s.nta",	FLDINCIMMED (0x06, 3)},
    {"ldfd.s",		FLDINCIMMED (0x07, 0)},
    {"ldfd.s.nt1",	FLDINCIMMED (0x07, 1)},
    {"ldfd.s.nta",	FLDINCIMMED (0x07, 3)},
    {"ldf8.s",		FLDINCIMMED (0x05, 0)},
    {"ldf8.s.nt1",	FLDINCIMMED (0x05, 1)},
    {"ldf8.s.nta",	FLDINCIMMED (0x05, 3)},
    {"ldfe.s",		FLDINCIMMED (0x04, 0)},
    {"ldfe.s.nt1",	FLDINCIMMED (0x04, 1)},
    {"ldfe.s.nta",	FLDINCIMMED (0x04, 3)},
    {"ldfs.a",		FLDINCIMMED (0x0a, 0)},
    {"ldfs.a.nt1",	FLDINCIMMED (0x0a, 1)},
    {"ldfs.a.nta",	FLDINCIMMED (0x0a, 3)},
    {"ldfd.a",		FLDINCIMMED (0x0b, 0)},
    {"ldfd.a.nt1",	FLDINCIMMED (0x0b, 1)},
    {"ldfd.a.nta",	FLDINCIMMED (0x0b, 3)},
    {"ldf8.a",		FLDINCIMMED (0x09, 0)},
    {"ldf8.a.nt1",	FLDINCIMMED (0x09, 1)},
    {"ldf8.a.nta",	FLDINCIMMED (0x09, 3)},
    {"ldfe.a",		FLDINCIMMED (0x08, 0)},
    {"ldfe.a.nt1",	FLDINCIMMED (0x08, 1)},
    {"ldfe.a.nta",	FLDINCIMMED (0x08, 3)},
    {"ldfs.sa",		FLDINCIMMED (0x0e, 0)},
    {"ldfs.sa.nt1",	FLDINCIMMED (0x0e, 1)},
    {"ldfs.sa.nta",	FLDINCIMMED (0x0e, 3)},
    {"ldfd.sa",		FLDINCIMMED (0x0f, 0)},
    {"ldfd.sa.nt1",	FLDINCIMMED (0x0f, 1)},
    {"ldfd.sa.nta",	FLDINCIMMED (0x0f, 3)},
    {"ldf8.sa",		FLDINCIMMED (0x0d, 0)},
    {"ldf8.sa.nt1",	FLDINCIMMED (0x0d, 1)},
    {"ldf8.sa.nta",	FLDINCIMMED (0x0d, 3)},
    {"ldfe.sa",		FLDINCIMMED (0x0c, 0)},
    {"ldfe.sa.nt1",	FLDINCIMMED (0x0c, 1)},
    {"ldfe.sa.nta",	FLDINCIMMED (0x0c, 3)},
    {"ldf.fill",	FLDINCIMMED (0x1b, 0)},
    {"ldf.fill.nt1",	FLDINCIMMED (0x1b, 1)},
    {"ldf.fill.nta",	FLDINCIMMED (0x1b, 3)},
    {"ldfs.c.clr",	FLDINCIMMED (0x22, 0)},
    {"ldfs.c.clr.nt1",	FLDINCIMMED (0x22, 1)},
    {"ldfs.c.clr.nta",	FLDINCIMMED (0x22, 3)},
    {"ldfd.c.clr",	FLDINCIMMED (0x23, 0)},
    {"ldfd.c.clr.nt1",	FLDINCIMMED (0x23, 1)},
    {"ldfd.c.clr.nta",	FLDINCIMMED (0x23, 3)},
    {"ldf8.c.clr",	FLDINCIMMED (0x21, 0)},
    {"ldf8.c.clr.nt1",	FLDINCIMMED (0x21, 1)},
    {"ldf8.c.clr.nta",	FLDINCIMMED (0x21, 3)},
    {"ldfe.c.clr",	FLDINCIMMED (0x20, 0)},
    {"ldfe.c.clr.nt1",	FLDINCIMMED (0x20, 1)},
    {"ldfe.c.clr.nta",	FLDINCIMMED (0x20, 3)},
    {"ldfs.c.nc",	FLDINCIMMED (0x26, 0)},
    {"ldfs.c.nc.nt1",	FLDINCIMMED (0x26, 1)},
    {"ldfs.c.nc.nta",	FLDINCIMMED (0x26, 3)},
    {"ldfd.c.nc",	FLDINCIMMED (0x27, 0)},
    {"ldfd.c.nc.nt1",	FLDINCIMMED (0x27, 1)},
    {"ldfd.c.nc.nta",	FLDINCIMMED (0x27, 3)},
    {"ldf8.c.nc",	FLDINCIMMED (0x25, 0)},
    {"ldf8.c.nc.nt1",	FLDINCIMMED (0x25, 1)},
    {"ldf8.c.nc.nta",	FLDINCIMMED (0x25, 3)},
    {"ldfe.c.nc",	FLDINCIMMED (0x24, 0)},
    {"ldfe.c.nc.nt1",	FLDINCIMMED (0x24, 1)},
    {"ldfe.c.nc.nta",	FLDINCIMMED (0x24, 3)},
#undef FLDINCIMMED

    /* Floating-point store w/increment by immediate.  */
#define FSTINCIMMED(c,h) M, OpX6aHint (7, c, h), {MR3, F2, IMM9a}, POSTINC, 0, NULL
    {"stfs",		FSTINCIMMED (0x32, 0)},
    {"stfs.nta",	FSTINCIMMED (0x32, 3)},
    {"stfd",		FSTINCIMMED (0x33, 0)},
    {"stfd.nta",	FSTINCIMMED (0x33, 3)},
    {"stf8",		FSTINCIMMED (0x31, 0)},
    {"stf8.nta",	FSTINCIMMED (0x31, 3)},
    {"stfe",		FSTINCIMMED (0x30, 0)},
    {"stfe.nta",	FSTINCIMMED (0x30, 3)},
    {"stf.spill",	FSTINCIMMED (0x3b, 0)},
    {"stf.spill.nta",	FSTINCIMMED (0x3b, 3)},
#undef FSTINCIMMED

    /* Line prefetch w/increment by immediate.  */
#define LFETCHINCIMMED(c,h) M0, OpX6aHint (7, c, h), {MR3, IMM9b}, POSTINC, 0, NULL
    {"lfetch",			LFETCHINCIMMED (0x2c, 0)},
    {"lfetch.nt1",		LFETCHINCIMMED (0x2c, 1)},
    {"lfetch.nt2",		LFETCHINCIMMED (0x2c, 2)},
    {"lfetch.nta",		LFETCHINCIMMED (0x2c, 3)},
    {"lfetch.excl",		LFETCHINCIMMED (0x2d, 0)},
    {"lfetch.excl.nt1",		LFETCHINCIMMED (0x2d, 1)},
    {"lfetch.excl.nt2",		LFETCHINCIMMED (0x2d, 2)},
    {"lfetch.excl.nta",		LFETCHINCIMMED (0x2d, 3)},
    {"lfetch.fault",		LFETCHINCIMMED (0x2e, 0)},
    {"lfetch.fault.nt1",	LFETCHINCIMMED (0x2e, 1)},
    {"lfetch.fault.nt2",	LFETCHINCIMMED (0x2e, 2)},
    {"lfetch.fault.nta",	LFETCHINCIMMED (0x2e, 3)},
    {"lfetch.fault.excl",	LFETCHINCIMMED (0x2f, 0)},
    {"lfetch.fault.excl.nt1",	LFETCHINCIMMED (0x2f, 1)},
    {"lfetch.fault.excl.nt2",	LFETCHINCIMMED (0x2f, 2)},
    {"lfetch.fault.excl.nta",	LFETCHINCIMMED (0x2f, 3)},
#undef LFETCHINCIMMED

    {NULL, 0, 0, 0, 0, {0}, 0, 0, NULL}
  };

#undef M0
#undef M
#undef M2
#undef bM
#undef bX
#undef bX2
#undef bX3
#undef bX4
#undef bX6a
#undef bX6b
#undef bHint
#undef mM
#undef mX
#undef mX2
#undef mX3
#undef mX4
#undef mX6a
#undef mX6b
#undef mHint
#undef OpX3
#undef OpX3X6b
#undef OpX3X4
#undef OpX3X4X2
#undef OpX6aHint
#undef OpXX6aHint
#undef OpMXX6a
#undef OpMXX6aHint
#undef EMPTY