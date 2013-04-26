
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

#include <sys/types.h>
#include <sys/syslog.h>
#include <sys/systm.h>

#include <machine/md_var.h>

#define NMI_PARITY 0x04
#define NMI_EPARITY 0x02

/*
 * Handle a NMI, possibly a machine check.
 * return true to panic system, false to ignore.
 */
int
isa_nmi(int cd)
{
	int retval = 0;
 	int port = inb(0x33);

	log(LOG_CRIT, "NMI PC98 port = %x\n", port);
	if (port & NMI_PARITY) {
		log(LOG_CRIT, "BASE RAM parity error, likely hardware failure.");
		retval = 1;
	} else if (port & NMI_EPARITY) {
		log(LOG_CRIT, "EXTENDED RAM parity error, likely hardware failure.");
		retval = 1;
	} else {
		log(LOG_CRIT, "\nNMI Resume ??\n");
	}

	return(retval);
}