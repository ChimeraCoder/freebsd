
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
/*	$OpenBSD: print-carp.c,v 1.6 2009/10/27 23:59:55 deraadt Exp $	*/
/*
 * Copyright (c) 2000 William C. Fenner.
 *                All rights reserved.
 *
 * Kevin Steves <ks@hp.se> July 2000
 * Modified to:
 * - print version, type string and packet length
 * - print IP address count if > 1 (-v)
 * - verify checksum (-v)
 * - print authentication string (-v)
 *
 * Copyright (c) 2011 Advanced Computing Technologies
 * George V. Neille-Neil
 *
 * Modified to:
 * - work correctly with CARP
 * - compile into the latest tcpdump
 * - print out the counter
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code
 * distributions retain the above copyright notice and this paragraph
 * in its entirety, and (2) distributions including binary code include
 * the above copyright notice and this paragraph in its entirety in
 * the documentation or other materials provided with the distribution.
 * The name of William C. Fenner may not be used to endorse or
 * promote products derived from this software without specific prior
 * written permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>

#include "interface.h"
#include "extract.h"
#include "addrtoname.h"

void
carp_print(register const u_char *bp, register u_int len, int ttl)
{
	int version, type;
	const char *type_s;

	TCHECK(bp[0]);
	version = (bp[0] & 0xf0) >> 4;
	type = bp[0] & 0x0f;
	if (type == 1)
		type_s = "advertise";
	else
		type_s = "unknown";
	printf("CARPv%d-%s %d: ", version, type_s, len);
	if (ttl != 255)
		printf("[ttl=%d!] ", ttl);
	if (version != 2 || type != 1)
		return;
	TCHECK(bp[2]);
	TCHECK(bp[5]);
	printf("vhid=%d advbase=%d advskew=%d authlen=%d ",
	    bp[1], bp[5], bp[2], bp[3]);
	if (vflag) {
		struct cksum_vec vec[1];
		vec[0].ptr = (const u_int8_t *)bp;
		vec[0].len = len;
		if (TTEST2(bp[0], len) && in_cksum(vec, 1))
			printf(" (bad carp cksum %x!)",
				EXTRACT_16BITS(&bp[6]));
	}
	printf("counter=%" PRIu64, EXTRACT_64BITS(&bp[8]));

	return;
trunc:
	printf("[|carp]");
}