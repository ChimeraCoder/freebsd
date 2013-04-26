
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
 * Copyright (C) 2002 by Darren Reed.
 * 
 * See the IPFILTER.LICENCE file for details on licencing.  
 *   
 * $Id: nametokva.c,v 1.1.4.1 2006/06/16 17:21:07 darrenr Exp $ 
 */     

#include "ipf.h"

#include <sys/ioctl.h>
#include <fcntl.h>

ipfunc_t nametokva(name, iocfunc)
char *name;
ioctlfunc_t iocfunc;
{
	ipfunc_resolve_t res;
	int fd;

	strncpy(res.ipfu_name, name, sizeof(res.ipfu_name));
	res.ipfu_addr = NULL;
	fd = -1;

	if ((opts & OPT_DONOTHING) == 0) {
		fd = open(IPL_NAME, O_RDONLY);
		if (fd == -1)
			return NULL;
	}
	(void) (*iocfunc)(fd, SIOCFUNCL, &res);
	if (fd >= 0)
		close(fd);
	if (res.ipfu_addr == NULL)
		res.ipfu_addr = (ipfunc_t)-1;
	return res.ipfu_addr;
}