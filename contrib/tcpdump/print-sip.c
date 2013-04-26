
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-sip.c,v 1.1 2004-07-27 17:04:20 hannes Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <stdlib.h>

#include "interface.h"
#include "extract.h"

#include "udp.h"

void
sip_print(register const u_char *pptr, register u_int len)
{
    u_int idx;

    printf("SIP, length: %u%s", len, vflag ? "\n\t" : "");

    /* in non-verbose mode just lets print the protocol and length */
    if (vflag < 1)
        return;

    for (idx = 0; idx < len; idx++) {
        TCHECK2(*(pptr+idx), 2);
        if (EXTRACT_16BITS(pptr+idx) != 0x0d0a) { /* linefeed ? */
            safeputchar(*(pptr+idx));
        } else {
            printf("\n\t");
            idx+=1;
        }
    }

    /* do we want to see an additionally hexdump ? */
    if (vflag> 1)
        print_unknown_data(pptr,"\n\t",len);

    return;

trunc:
    printf("[|sip]");
}