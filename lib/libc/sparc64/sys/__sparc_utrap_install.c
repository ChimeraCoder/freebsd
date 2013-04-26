
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

#include <machine/utrap.h>
#include <machine/sysarch.h>

int
__sparc_utrap_install(utrap_entry_t type, utrap_handler_t new_precise,
    utrap_handler_t new_deferred, utrap_handler_t *old_precise,
    utrap_handler_t *old_deferred)
{
	struct sparc_utrap_install_args uia;
	struct sparc_utrap_args ua[1];

	ua[0].type = type;
	ua[0].new_precise = new_precise;
	ua[0].new_deferred = new_deferred;
	ua[0].old_precise = old_precise;
	ua[0].old_deferred = old_deferred;
	uia.num = 1;
	uia.handlers = ua;
	return (sysarch(SPARC_UTRAP_INSTALL, &uia));
}