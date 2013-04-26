
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/* 
 * Alias_dummy is just an empty skeleton used to demostrate how to write
 * a module for libalias, that will run unalterated in userland or in
 * kernel land.
 */

#ifdef _KERNEL
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#endif

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#ifdef _KERNEL
#include <netinet/libalias/alias_local.h>
#include <netinet/libalias/alias_mod.h>
#else
#include "alias_local.h"
#include "alias_mod.h"
#endif

static void
AliasHandleDummy(struct libalias *la, struct ip *ip, struct alias_data *ah);

static int 
fingerprint(struct libalias *la, struct alias_data *ah)
{

	/* 
	 * Check here all the data that will be used later, if any field 
	 * is empy/NULL, return a -1 value.
	 */
	if (ah->dport == NULL || ah->sport == NULL || ah->lnk == NULL || 
		ah->maxpktsize == 0)
		return (-1);
	/* 
	 * Fingerprint the incoming packet, if it matches any conditions 
	 * return an OK value.
	 */
	if (ntohs(*ah->dport) == 123
	    || ntohs(*ah->sport) == 456)
		return (0); /* I know how to handle it. */
	return (-1); /* I don't recognize this packet. */
}

/* 
 * Wrap in this general purpose function, the real function used to alias the 
 * packets.
 */

static int 
protohandler(struct libalias *la, struct ip *pip, struct alias_data *ah)
{
	
	AliasHandleDummy(la, pip, ah);
	return (0);
}

/* 
 * NOTA BENE: the next variable MUST NOT be renamed in any case if you want 
 * your module to work in userland, cause it's used to find and use all 
 * the protocol handlers present in every module.
 * So WATCH OUT, your module needs this variables and it needs it with 
 * ITS EXACT NAME: handlers.
 */

struct proto_handler handlers [] = {
	{ 
	  .pri = 666, 
	  .dir = IN|OUT, 
	  .proto = UDP|TCP, 
	  .fingerprint = &fingerprint, 
	  .protohandler = &protohandler
	}, 
	{ EOH }
};

static int
mod_handler(module_t mod, int type, void *data)
{
	int error;

	switch (type) {	  
	case MOD_LOAD:
		error = 0;
		LibAliasAttachHandlers(handlers);
		break;
	case MOD_UNLOAD:
		error = 0;
		LibAliasDetachHandlers(handlers);
		break;
	default:
		error = EINVAL;
	}
	return (error);
}

#ifdef _KERNEL
static
#endif
moduledata_t alias_mod = {
       "alias_dummy", mod_handler, NULL
};

#ifdef	_KERNEL
DECLARE_MODULE(alias_dummy, alias_mod, SI_SUB_DRIVERS, SI_ORDER_SECOND);
MODULE_VERSION(alias_dummy, 1);
MODULE_DEPEND(alias_dummy, libalias, 1, 1, 1);
#endif

static void
AliasHandleDummy(struct libalias *la, struct ip *ip, struct alias_data *ah)
{
	; /* Dummy. */
}