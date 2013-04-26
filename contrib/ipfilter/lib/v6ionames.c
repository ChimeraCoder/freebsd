
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
 * Copyright (C) 2003-2005 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: v6ionames.c,v 1.1.4.3 2006/06/16 17:21:18 darrenr Exp $
 */
#include "ipf.h"


#ifdef	USE_INET6

struct	ipopt_names	v6ionames[] ={
	{ IPPROTO_HOPOPTS,	0x000001,	0,	"hopopts" },
	{ IPPROTO_IPV6,		0x000002,	0,	"ipv6" },
	{ IPPROTO_ROUTING,	0x000004,	0,	"routing" },
	{ IPPROTO_FRAGMENT,	0x000008,	0,	"frag" },	
	{ IPPROTO_ESP,		0x000010,	0,	"esp" },
	{ IPPROTO_AH,		0x000020,	0,	"ah" },
	{ IPPROTO_NONE,		0x000040,	0,	"none" },	
	{ IPPROTO_DSTOPTS,	0x000080,	0,	"dstopts" },
	{ IPPROTO_MOBILITY,	0x000100,	0,	"mobility" },
	{ 0, 			0,		0,	(char *)NULL }
};

#endif