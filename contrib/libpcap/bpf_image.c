
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

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/libpcap/bpf_image.c,v 1.28 2008-01-02 04:16:46 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <pcap-stdinc.h>
#else /* WIN32 */
#if HAVE_INTTYPES_H
#include <inttypes.h>
#elif HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_SYS_BITYPES_H
#include <sys/bitypes.h>
#endif
#include <sys/types.h>
#endif /* WIN32 */

#include <stdio.h>
#include <string.h>

#include "pcap-int.h"

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

char *
bpf_image(p, n)
	struct bpf_insn *p;
	int n;
{
	int v;
	const char *fmt, *op;
	static char image[256];
	char operand[64];

	v = p->k;
	switch (p->code) {

	default:
		op = "unimp";
		fmt = "0x%x";
		v = p->code;
		break;

	case BPF_RET|BPF_K:
		op = "ret";
		fmt = "#%d";
		break;

	case BPF_RET|BPF_A:
		op = "ret";
		fmt = "";
		break;

	case BPF_LD|BPF_W|BPF_ABS:
		op = "ld";
		fmt = "[%d]";
		break;

	case BPF_LD|BPF_H|BPF_ABS:
		op = "ldh";
		fmt = "[%d]";
		break;

	case BPF_LD|BPF_B|BPF_ABS:
		op = "ldb";
		fmt = "[%d]";
		break;

	case BPF_LD|BPF_W|BPF_LEN:
		op = "ld";
		fmt = "#pktlen";
		break;

	case BPF_LD|BPF_W|BPF_IND:
		op = "ld";
		fmt = "[x + %d]";
		break;

	case BPF_LD|BPF_H|BPF_IND:
		op = "ldh";
		fmt = "[x + %d]";
		break;

	case BPF_LD|BPF_B|BPF_IND:
		op = "ldb";
		fmt = "[x + %d]";
		break;

	case BPF_LD|BPF_IMM:
		op = "ld";
		fmt = "#0x%x";
		break;

	case BPF_LDX|BPF_IMM:
		op = "ldx";
		fmt = "#0x%x";
		break;

	case BPF_LDX|BPF_MSH|BPF_B:
		op = "ldxb";
		fmt = "4*([%d]&0xf)";
		break;

	case BPF_LD|BPF_MEM:
		op = "ld";
		fmt = "M[%d]";
		break;

	case BPF_LDX|BPF_MEM:
		op = "ldx";
		fmt = "M[%d]";
		break;

	case BPF_ST:
		op = "st";
		fmt = "M[%d]";
		break;

	case BPF_STX:
		op = "stx";
		fmt = "M[%d]";
		break;

	case BPF_JMP|BPF_JA:
		op = "ja";
		fmt = "%d";
		v = n + 1 + p->k;
		break;

	case BPF_JMP|BPF_JGT|BPF_K:
		op = "jgt";
		fmt = "#0x%x";
		break;

	case BPF_JMP|BPF_JGE|BPF_K:
		op = "jge";
		fmt = "#0x%x";
		break;

	case BPF_JMP|BPF_JEQ|BPF_K:
		op = "jeq";
		fmt = "#0x%x";
		break;

	case BPF_JMP|BPF_JSET|BPF_K:
		op = "jset";
		fmt = "#0x%x";
		break;

	case BPF_JMP|BPF_JGT|BPF_X:
		op = "jgt";
		fmt = "x";
		break;

	case BPF_JMP|BPF_JGE|BPF_X:
		op = "jge";
		fmt = "x";
		break;

	case BPF_JMP|BPF_JEQ|BPF_X:
		op = "jeq";
		fmt = "x";
		break;

	case BPF_JMP|BPF_JSET|BPF_X:
		op = "jset";
		fmt = "x";
		break;

	case BPF_ALU|BPF_ADD|BPF_X:
		op = "add";
		fmt = "x";
		break;

	case BPF_ALU|BPF_SUB|BPF_X:
		op = "sub";
		fmt = "x";
		break;

	case BPF_ALU|BPF_MUL|BPF_X:
		op = "mul";
		fmt = "x";
		break;

	case BPF_ALU|BPF_DIV|BPF_X:
		op = "div";
		fmt = "x";
		break;

	case BPF_ALU|BPF_AND|BPF_X:
		op = "and";
		fmt = "x";
		break;

	case BPF_ALU|BPF_OR|BPF_X:
		op = "or";
		fmt = "x";
		break;

	case BPF_ALU|BPF_LSH|BPF_X:
		op = "lsh";
		fmt = "x";
		break;

	case BPF_ALU|BPF_RSH|BPF_X:
		op = "rsh";
		fmt = "x";
		break;

	case BPF_ALU|BPF_ADD|BPF_K:
		op = "add";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_SUB|BPF_K:
		op = "sub";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_MUL|BPF_K:
		op = "mul";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_DIV|BPF_K:
		op = "div";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_AND|BPF_K:
		op = "and";
		fmt = "#0x%x";
		break;

	case BPF_ALU|BPF_OR|BPF_K:
		op = "or";
		fmt = "#0x%x";
		break;

	case BPF_ALU|BPF_LSH|BPF_K:
		op = "lsh";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_RSH|BPF_K:
		op = "rsh";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_NEG:
		op = "neg";
		fmt = "";
		break;

	case BPF_MISC|BPF_TAX:
		op = "tax";
		fmt = "";
		break;

	case BPF_MISC|BPF_TXA:
		op = "txa";
		fmt = "";
		break;
	}
	(void)snprintf(operand, sizeof operand, fmt, v);
	(void)snprintf(image, sizeof image,
		      (BPF_CLASS(p->code) == BPF_JMP &&
		       BPF_OP(p->code) != BPF_JA) ?
		      "(%03d) %-8s %-16s jt %d\tjf %d"
		      : "(%03d) %-8s %s",
		      n, op, operand, n + 1 + p->jt, n + 1 + p->jf);
	return image;
}