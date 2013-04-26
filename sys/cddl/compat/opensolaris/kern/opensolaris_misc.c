
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

#include <sys/param.h>
#include <sys/jail.h>
#include <sys/kernel.h>
#include <sys/libkern.h>
#include <sys/limits.h>
#include <sys/misc.h>
#include <sys/sysctl.h>

char hw_serial[11] = "0";

struct opensolaris_utsname utsname = {
	.machine = MACHINE
};

static void
opensolaris_utsname_init(void *arg)
{

	utsname.sysname = ostype;
	utsname.nodename = prison0.pr_hostname;
	utsname.release = osrelease;
	snprintf(utsname.version, sizeof(utsname.version), "%d", osreldate);
}
SYSINIT(opensolaris_utsname_init, SI_SUB_TUNABLES, SI_ORDER_ANY,
    opensolaris_utsname_init, NULL);