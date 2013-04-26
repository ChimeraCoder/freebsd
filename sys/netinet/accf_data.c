
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

#define ACCEPT_FILTER_MOD

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/sysctl.h>
#include <sys/signalvar.h>
#include <sys/socketvar.h>

/* accept filter that holds a socket until data arrives */

static int	sohasdata(struct socket *so, void *arg, int waitflag);

static struct accept_filter accf_data_filter = {
	"dataready",
	sohasdata,
	NULL,
	NULL
};

static moduledata_t accf_data_mod = {
	"accf_data",
	accept_filt_generic_mod_event,
	&accf_data_filter
};

DECLARE_MODULE(accf_data, accf_data_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);

static int
sohasdata(struct socket *so, void *arg, int waitflag)
{

	if (!soreadable(so))
		return (SU_OK);

	return (SU_ISCONNECTED);
}