
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-syslog.c,v 1.1 2004-10-29 11:42:53 hannes Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <stdlib.h>

#include "interface.h"
#include "extract.h"
#include "addrtoname.h"

/* 
 * tokenlists and #defines taken from Ethereal - Network traffic analyzer
 * by Gerald Combs <gerald@ethereal.com>
 */

#define SYSLOG_SEVERITY_MASK 0x0007  /* 0000 0000 0000 0111 */
#define SYSLOG_FACILITY_MASK 0x03f8  /* 0000 0011 1111 1000 */
#define SYSLOG_MAX_DIGITS 3 /* The maximum number if priority digits to read in. */

static const struct tok syslog_severity_values[] = {
  { 0,      "emergency" },
  { 1,      "alert" },
  { 2,      "critical" },
  { 3,      "error" },
  { 4,      "warning" },
  { 5,      "notice" },
  { 6,      "info" },
  { 7,      "debug" },
  { 0, NULL },
};

static const struct tok syslog_facility_values[] = {
  { 0,     "kernel" },
  { 1,     "user" },
  { 2,     "mail" },
  { 3,     "daemon" },
  { 4,     "auth" },
  { 5,     "syslog" },
  { 6,     "lpr" },
  { 7,     "news" },
  { 8,     "uucp" },
  { 9,     "cron" },
  { 10,    "authpriv" },
  { 11,    "ftp" },
  { 12,    "ntp" },
  { 13,    "security" },
  { 14,    "console" },
  { 15,    "cron" },
  { 16,    "local0" },
  { 17,    "local1" },
  { 18,    "local2" },
  { 19,    "local3" },
  { 20,    "local4" },
  { 21,    "local5" },
  { 22,    "local6" },
  { 23,    "local7" },
  { 0, NULL },
};

void
syslog_print(register const u_char *pptr, register u_int len)
{
    u_int16_t msg_off = 0;
    u_int16_t pri = 0;
    u_int16_t facility,severity;

    /* extract decimal figures that are
     * encapsulated within < > tags
     * based on this decimal figure extract the
     * severity and facility values
     */

    if (!TTEST2(*pptr, 1))
        goto trunc;

    if (*(pptr+msg_off) == '<') {
        msg_off++;

        if (!TTEST2(*(pptr+msg_off), 1))
            goto trunc;

        while ( *(pptr+msg_off) >= '0' &&
                *(pptr+msg_off) <= '9' &&
                msg_off <= SYSLOG_MAX_DIGITS) {

            if (!TTEST2(*(pptr+msg_off), 1))
                goto trunc;

            pri = pri * 10 + (*(pptr+msg_off) - '0');
            msg_off++;

            if (!TTEST2(*(pptr+msg_off), 1))
                goto trunc;

        if (*(pptr+msg_off) == '>')
            msg_off++;
        }
    } else {
        printf("[|syslog]");
        return;
    }

    facility = (pri & SYSLOG_FACILITY_MASK) >> 3;
    severity = pri & SYSLOG_SEVERITY_MASK;

    
    if (vflag < 1 )
    {
        printf("SYSLOG %s.%s, length: %u",
               tok2str(syslog_facility_values, "unknown (%u)", facility),
               tok2str(syslog_severity_values, "unknown (%u)", severity),
               len);
        return;
    }
       
    printf("SYSLOG, length: %u\n\tFacility %s (%u), Severity %s (%u)\n\tMsg: ",
           len,
           tok2str(syslog_facility_values, "unknown (%u)", facility),
           facility,
           tok2str(syslog_severity_values, "unknown (%u)", severity),
           severity);

    /* print the syslog text in verbose mode */
    for (; msg_off < len; msg_off++) {
        if (!TTEST2(*(pptr+msg_off), 1))
            goto trunc;
        safeputchar(*(pptr+msg_off));        
    }

    if (vflag > 1) {
        if(!print_unknown_data(pptr,"\n\t",len))
            return;
    }
    
    return;

trunc:
        printf("[|syslog]");
}