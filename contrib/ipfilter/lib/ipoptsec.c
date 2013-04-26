
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
/*	$FreeBSD$	*/
/*
 * Copyright (C) 2001-2002 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: ipoptsec.c,v 1.2.4.1 2006/06/16 17:21:04 darrenr Exp $
 */

#include "ipf.h"


struct	ipopt_names	secclass[] = {
	{ IPSO_CLASS_RES4,	0x01,	0, "reserv-4" },
	{ IPSO_CLASS_TOPS,	0x02,	0, "topsecret" },
	{ IPSO_CLASS_SECR,	0x04,	0, "secret" },
	{ IPSO_CLASS_RES3,	0x08,	0, "reserv-3" },
	{ IPSO_CLASS_CONF,	0x10,	0, "confid" },
	{ IPSO_CLASS_UNCL,	0x20,	0, "unclass" },
	{ IPSO_CLASS_RES2,	0x40,	0, "reserv-2" },
	{ IPSO_CLASS_RES1,	0x80,	0, "reserv-1" },
	{ 0, 0, 0, NULL }	/* must be last */
};


u_char seclevel(slevel)
char *slevel;
{
	struct ipopt_names *so;

	for (so = secclass; so->on_name; so++)
		if (!strcasecmp(slevel, so->on_name))
			break;

	if (!so->on_name) {
		fprintf(stderr, "no such security level: %s\n", slevel);
		return 0;
	}
	return (u_char)so->on_value;
}


u_char secbit(class)
int class;
{
	struct ipopt_names *so;

	for (so = secclass; so->on_name; so++)
		if (so->on_value == class)
			break;

	if (!so->on_name) {
		fprintf(stderr, "no such security class: %d\n", class);
		return 0;
	}
	return (u_char)so->on_bit;
}