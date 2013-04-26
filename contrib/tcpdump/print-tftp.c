
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
    "@(#) $Header: /tcpdump/master/tcpdump/print-tftp.c,v 1.39 2008-04-11 16:47:38 gianluca Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#ifdef SEGSIZE
#undef SEGSIZE					/* SINIX sucks */
#endif

#include <stdio.h>
#include <string.h>

#include "interface.h"
#include "addrtoname.h"
#include "extract.h"
#include "tftp.h"

/* op code to string mapping */
static struct tok op2str[] = {
	{ RRQ,		"RRQ" },	/* read request */
	{ WRQ,		"WRQ" },	/* write request */
	{ DATA,		"DATA" },	/* data packet */
	{ ACK,		"ACK" },	/* acknowledgement */
	{ TFTP_ERROR,	"ERROR" },	/* error code */
	{ OACK,		"OACK" },	/* option acknowledgement */
	{ 0,		NULL }
};

/* error code to string mapping */
static struct tok err2str[] = {
	{ EUNDEF,	"EUNDEF" },	/* not defined */
	{ ENOTFOUND,	"ENOTFOUND" },	/* file not found */
	{ EACCESS,	"EACCESS" },	/* access violation */
	{ ENOSPACE,	"ENOSPACE" },	/* disk full or allocation exceeded */
	{ EBADOP,	"EBADOP" },	/* illegal TFTP operation */
	{ EBADID,	"EBADID" },	/* unknown transfer ID */
	{ EEXISTS,	"EEXISTS" },	/* file already exists */
	{ ENOUSER,	"ENOUSER" },	/* no such user */
	{ 0,		NULL }
};

/*
 * Print trivial file transfer program requests
 */
void
tftp_print(register const u_char *bp, u_int length)
{
	register const struct tftphdr *tp;
	register const char *cp;
	register const u_char *p;
	register int opcode, i;
	static char tstr[] = " [|tftp]";

	tp = (const struct tftphdr *)bp;

	/* Print length */
	printf(" %d", length);

	/* Print tftp request type */
	TCHECK(tp->th_opcode);
	opcode = EXTRACT_16BITS(&tp->th_opcode);
	cp = tok2str(op2str, "tftp-#%d", opcode);
	printf(" %s", cp);
	/* Bail if bogus opcode */
	if (*cp == 't')
		return;

	switch (opcode) {

	case RRQ:
	case WRQ:
	case OACK:
		p = (u_char *)tp->th_stuff;
		putchar(' ');
		/* Print filename or first option */
		if (opcode != OACK)
			putchar('"');
		i = fn_print(p, snapend);
		if (opcode != OACK)
			putchar('"');

		/* Print the mode (RRQ and WRQ only) and any options */
		while ((p = (const u_char *)strchr((const char *)p, '\0')) != NULL) {
			if (length <= (u_int)(p - (const u_char *)&tp->th_block))
				break;
			p++;
			if (*p != '\0') {
				putchar(' ');
				fn_print(p, snapend);
			}
		}
		
		if (i)
			goto trunc;
		break;

	case ACK:
	case DATA:
		TCHECK(tp->th_block);
		printf(" block %d", EXTRACT_16BITS(&tp->th_block));
		break;

	case TFTP_ERROR:
		/* Print error code string */
		TCHECK(tp->th_code);
		printf(" %s \"", tok2str(err2str, "tftp-err-#%d \"",
				       EXTRACT_16BITS(&tp->th_code)));
		/* Print error message string */
		i = fn_print((const u_char *)tp->th_data, snapend);
		putchar('"');
		if (i)
			goto trunc;
		break;

	default:
		/* We shouldn't get here */
		printf("(unknown #%d)", opcode);
		break;
	}
	return;
trunc:
	fputs(tstr, stdout);
	return;
}