
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

#include "opt_inet6.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/systm.h>

#include <rpc/types.h>

/*
 * For in-kernel use, we use a simple compiled-in configuration.
 */

static struct netconfig netconfigs[] = {
#ifdef INET6
	{
		.nc_netid =	"udp6",
		.nc_semantics =	NC_TPI_CLTS,
		.nc_flag =	NC_VISIBLE,
		.nc_protofmly =	"inet6",
		.nc_proto =	"udp",
	},
	{
		.nc_netid =	"tcp6",
		.nc_semantics =	NC_TPI_COTS_ORD,
		.nc_flag =	NC_VISIBLE,
		.nc_protofmly =	"inet6",
		.nc_proto =	"tcp",
	},
#endif	
	{
		.nc_netid =	"udp",
		.nc_semantics =	NC_TPI_CLTS,
		.nc_flag =	NC_VISIBLE,
		.nc_protofmly =	"inet",
		.nc_proto =	"udp",
	},
	{
		.nc_netid =	"tcp",
		.nc_semantics =	NC_TPI_COTS_ORD,
		.nc_flag =	NC_VISIBLE,
		.nc_protofmly =	"inet",
		.nc_proto =	"tcp",
	},
	{
		.nc_netid =	"local",
		.nc_semantics =	NC_TPI_COTS_ORD,
		.nc_flag =	0,
		.nc_protofmly =	"loopback",
		.nc_proto =	"",
	},
	{
		.nc_netid =	NULL,
	}
};

void *
setnetconfig(void)
{
	struct netconfig **nconfp;

	nconfp = malloc(sizeof(struct netconfig *), M_RPC, M_WAITOK);
	*nconfp = netconfigs;

	return ((void *) nconfp);
}

struct netconfig *
getnetconfig(void *handle)
{
	struct netconfig **nconfp = (struct netconfig **) handle;
	struct netconfig *nconf;

	nconf = *nconfp;
	if (nconf->nc_netid == NULL)
		return (NULL);

	(*nconfp)++;

	return (nconf);
}

struct netconfig *
getnetconfigent(const char *netid)
{
	struct netconfig *nconf;

	for (nconf = netconfigs; nconf->nc_netid; nconf++) {
		if (!strcmp(netid, nconf->nc_netid))
			return (nconf);
	}

	return (NULL);
}

void
freenetconfigent(struct netconfig *nconf)
{

}

int
endnetconfig(void * handle)
{
	struct netconfig **nconfp = (struct netconfig **) handle;

	free(nconfp, M_RPC);
	return (0);
}