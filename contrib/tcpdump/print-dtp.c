
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <string.h>

#include "interface.h"
#include "addrtoname.h"
#include "extract.h"		
#include "nlpid.h"

#define DTP_HEADER_LEN			1
#define DTP_DOMAIN_TLV			0x0001
#define DTP_STATUS_TLV			0x0002
#define DTP_DTP_TYPE_TLV		0x0003
#define DTP_NEIGHBOR_TLV		0x0004

static struct tok dtp_tlv_values[] = {
    { DTP_DOMAIN_TLV, "Domain TLV"},
    { DTP_STATUS_TLV, "Status TLV"},
    { DTP_DTP_TYPE_TLV, "DTP type TLV"},
    { DTP_NEIGHBOR_TLV, "Neighbor TLV"},
    { 0, NULL}
};

void
dtp_print (const u_char *pptr, u_int length)
{
    int type, len;
    const u_char *tptr;

    if (length < DTP_HEADER_LEN)
        goto trunc;

    tptr = pptr; 

    if (!TTEST2(*tptr, DTP_HEADER_LEN))	
	goto trunc;

    printf("DTPv%u, length %u", 
           (*tptr),
           length);

    /*
     * In non-verbose mode, just print version.
     */
    if (vflag < 1) {
	return;
    }

    tptr += DTP_HEADER_LEN;

    while (tptr < (pptr+length)) {

        if (!TTEST2(*tptr, 4)) 
            goto trunc;

	type = EXTRACT_16BITS(tptr);
        len  = EXTRACT_16BITS(tptr+2); 

        /* infinite loop check */
        if (type == 0 || len == 0) {
            return;
        }

        printf("\n\t%s (0x%04x) TLV, length %u",
               tok2str(dtp_tlv_values, "Unknown", type),
               type, len);

        switch (type) {
	case DTP_DOMAIN_TLV:
		printf(", %s", tptr+4);
		break;

	case DTP_STATUS_TLV:            
	case DTP_DTP_TYPE_TLV:
                printf(", 0x%x", *(tptr+4));
                break;

	case DTP_NEIGHBOR_TLV:
                printf(", %s", etheraddr_string(tptr+4));
                break;

        default:
            break;
        }	
        tptr += len;
    }

    return;

 trunc:
    printf("[|dtp]");
}

/*
 * Local Variables:
 * c-style: whitesmith
 * c-basic-offset: 4
 * End:
 */