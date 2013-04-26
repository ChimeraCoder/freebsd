
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

#include <machine/utrap.h>
#include <machine/sysarch.h>

#include <stdlib.h>

extern char __sigtramp[];

static const struct sparc_sigtramp_install_args sia = { __sigtramp, NULL };

void
__sparc_sigtramp_setup(void)
{

	sysarch(SPARC_SIGTRAMP_INSTALL, (void *)&sia);
}