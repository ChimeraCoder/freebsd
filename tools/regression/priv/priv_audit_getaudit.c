
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

/*
 * Confirm that privilege is required to query process audit state.
 */

#include <sys/types.h>

#include <bsm/audit.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>

#include "main.h"

int
priv_audit_getaudit_setup(int asroot, int injail, struct test *test)
{

	/*
	 * XXXRW: It would be nice if we checked for audit being configured
	 * here.
	 */
	return (0);
}

void
priv_audit_getaudit(int asroot, int injail, struct test *test)
{
	auditinfo_t ai;
	int error;

	error = getaudit(&ai);
	if (asroot && injail)
		expect("priv_audit_getaudit(asroot, injail)", error, -1,
		    ENOSYS);
	if (asroot && !injail)
		expect("priv_audit_getaudit(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_audit_getaudit(!asroot, injail)", error, -1,
		    ENOSYS);
	if (!asroot && !injail)
		expect("priv_audit_getaudit(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_audit_getaudit_addr(int asroot, int injail, struct test *test)
{
	auditinfo_addr_t aia;
	int error;

	error = getaudit_addr(&aia, sizeof(aia));
	if (asroot && injail)
		expect("priv_audit_getaudit_addr(asroot, injail)", error, -1,
		    ENOSYS);
	if (asroot && !injail)
		expect("priv_audit_getaudit_addr(asroot, !injail)", error, 0,
		    0);
	if (!asroot && injail)
		expect("priv_audit_getaudit_addr(!asroot, injail)", error,
		    -1, ENOSYS);
	if (!asroot && !injail)
		expect("priv_audit_getaudit_addr(!asroot, !injail)", error,
		    -1, EPERM);
}

void
priv_audit_getaudit_cleanup(int asroot, int injail, struct test *test)
{

}