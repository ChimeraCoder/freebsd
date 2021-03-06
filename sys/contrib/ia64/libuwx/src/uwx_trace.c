
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
/*Copyright (c) 2003-2006 Hewlett-Packard Development Company, L.P.
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "uwx_env.h"
#include "uwx_utable.h"
#include "uwx_uinfo.h"
#include "uwx_scoreboard.h"
#include "uwx_trace.h"

#ifdef UWX_TRACE_ENABLE

void uwx_trace_init(struct uwx_env *env)
{
    char *tstr;

    tstr = getenv("UWX_TRACE");
    if (tstr != NULL) {
	while (*tstr != '\0') {
	    switch (*tstr) {
		case 'i': env->trace |= UWX_TRACE_UINFO; break;
		case 't': env->trace |= UWX_TRACE_UTABLE; break;
		case 'b': env->trace |= UWX_TRACE_SB; break;
		case 'r': env->trace |= UWX_TRACE_RSTATE; break;
		case 's': env->trace |= UWX_TRACE_STEP; break;
		case 'c': env->trace |= UWX_TRACE_CONTEXT; break;
		case 'C': env->trace |= UWX_TRACE_COPYIN; break;
		case 'L': env->trace |= UWX_TRACE_LOOKUPIP; break;
		case '?':
#ifdef _KERNEL
		    fprintf(stderr, "UWX_TRACE flag `%c' unknown.\n", *tstr);
#else
		    fprintf(stderr, "UWX_TRACE flags:\n");
		    fprintf(stderr, "  i: unwind info\n");
		    fprintf(stderr, "  t: unwind table searching\n");
		    fprintf(stderr, "  b: scoreboard management\n");
		    fprintf(stderr, "  r: register state vector\n");
		    fprintf(stderr, "  s: step\n");
		    fprintf(stderr, "  c: context\n");
		    fprintf(stderr, "  C: copyin callback\n");
		    fprintf(stderr, "  L: lookup ip callback\n");
		    exit(1);
#endif
	    }
	    tstr++;
	}
    }
}

char *uwx_sb_rnames[] = {
    "RP", "PSP", "PFS",
    "PREDS", "UNAT", "PRIUNAT", "RNAT", "LC", "FPSR",
    "GR4", "GR5", "GR6", "GR7",
    "BR1", "BR2", "BR3", "BR4", "BR5",
    "FR2", "FR3", "FR4", "FR5",
    "FR16", "FR17", "FR18", "FR19",
    "FR20", "FR21", "FR22", "FR23",
    "FR24", "FR25", "FR26", "FR27",
    "FR28", "FR29", "FR30", "FR31",
};

void uwx_dump_rstate(int regid, uint64_t rstate)
{
    int reg;

    if (rstate == UWX_DISP_NONE)
	return;
    fprintf(stderr, "    %-7s", uwx_sb_rnames[regid]);
    switch (UWX_GET_DISP_CODE(rstate)) {
	case UWX_DISP_NONE:
	    fprintf(stderr, "    unchanged\n");
	    break;
	case UWX_DISP_SPPLUS(0):
	    fprintf(stderr, "    SP + %d\n", (int)rstate & ~0x07);
	    break;
	case UWX_DISP_SPREL(0):
	    fprintf(stderr, "    [SP + %d]\n", (int)rstate & ~0x07);
	    break;
	case UWX_DISP_PSPREL(0):
	    fprintf(stderr, "    [PSP + 16 - %d]\n", (int)rstate & ~0x07);
	    break;
	case UWX_DISP_REG(0):
	    reg = UWX_GET_DISP_REGID(rstate);
	    if (reg == UWX_REG_AR_PFS)
		fprintf(stderr, "    AR.PFS\n");
	    else if (reg == UWX_REG_AR_UNAT)
		fprintf(stderr, "    AR.UNAT\n");
	    else if (reg >= UWX_REG_GR(0) && reg < UWX_REG_GR(128))
		fprintf(stderr, "    GR%d\n", reg - UWX_REG_GR(0));
	    else if (reg >= UWX_REG_FR(0) && reg < UWX_REG_FR(128))
		fprintf(stderr, "    FR%d\n", reg - UWX_REG_FR(0));
	    else if (reg >= UWX_REG_BR(0) && reg < UWX_REG_BR(8))
		fprintf(stderr, "    BR%d\n", reg - UWX_REG_BR(0));
	    else
		fprintf(stderr, "    <reg %d>\n", reg);
	    break;
	default:
	    fprintf(stderr, "    <%08lx>\n", (long)rstate);
	    break;
    }
}

void uwx_dump_scoreboard(
    struct uwx_scoreboard *scoreboard,
    int nsbreg,
    struct uwx_rhdr *rhdr,
    int cur_slot,
    int ip_slot)
{
    int i;

    if (rhdr->is_prologue)
	fprintf(stderr, "  Prologue region (start = %d, length = %d)\n",
		    (int)cur_slot, (int)rhdr->rlen);
    else
	fprintf(stderr, "  Body region (start = %d, length = %d, ecount = %d)\n",
		    cur_slot, (int)rhdr->rlen, rhdr->ecount);
    if (ip_slot < rhdr->rlen)
	fprintf(stderr, "    IP is in this region (offset = %d)\n", ip_slot);
    for (i = 0; i < nsbreg; i++)
	uwx_dump_rstate(i, scoreboard->rstate[i]);
}

void uwx_dump_uinfo_block(
	struct uwx_utable_entry *uentry,
	unsigned int ulen)
{
    int i;
    uint32_t *uinfo = (uint32_t *)(intptr_t)uentry->unwind_info;

    ulen += DWORDSZ;		/* Include unwind info header */
    if (uentry->unwind_flags & UNWIND_TBL_32BIT) /* and personality routine */
	ulen += WORDSZ;
    else
	ulen += DWORDSZ;
    while (ulen >= WORDSZ) {
	fprintf(stderr, "  %08lx: ", (unsigned long)uinfo);
	for (i = 0; i < 4 * WORDSZ && ulen >= WORDSZ; i += WORDSZ) {
	    fprintf(stderr, " %04x", *uinfo++);
	    ulen -= WORDSZ;
	}
	fprintf(stderr, "\n");
    }
}

#endif /* UWX_TRACE_ENABLE */