
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
 * Confirm privilege is required to set process audit properties; we first
 * query current properties so that the attempted operation is a no-op.
 */

#include <sys/types.h>

#include <bsm/audit.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>

#include "main.h"

static auditinfo_t ai;
static auditinfo_addr_t aia;

int
priv_audit_setaudit_setup(int asroot, int injail, struct test *test)
{

	if (getaudit(&ai) < 0) {
		warn("priv_audit_setaudit_setup: getaudit");
		return (-1);
	}
	if (getaudit_addr(&aia, sizeof(aia)) < 0) {
		warn("priv_audit_setaudit_setup: getaudit_addr");
		return (-1);
	}

	return (0);
}

void
priv_audit_setaudit(int asroot, int injail, struct test *test)
{
	int error;

	error = setaudit(&ai);
	if (asroot && injail)
		expect("priv_audit_setaudit(asroot, injail)", error, -1,
		    ENOSYS);
	if (asroot && !injail)
		expect("priv_audit_setaudit(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_audit_setaudit(!asroot, injail)", error, -1,
		    ENOSYS);
	if (!asroot && !injail)
		expect("priv_audit_setaudit(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_audit_setaudit_addr(int asroot, int injail, struct test *test)
{
	int error;

	error = setaudit_addr(&aia, sizeof(aia));
	if (asroot && injail)
		expect("priv_audit_setaudit_addr(asroot, injail)", error, -1,
		    ENOSYS);
	if (asroot && !injail)
		expect("priv_audit_setaudit_addr(asroot, !injail)", error, 0,
		    0);
	if (!asroot && injail)
		expect("priv_audit_setaudit_addr(!asroot, injail)", error,
		    -1, ENOSYS);
	if (!asroot && !injail)
		expect("priv_audit_setaudit_addr(!asroot, !injail)", error,
		    -1, EPERM);
}

void
priv_audit_setaudit_cleanup(int asroot, int injail, struct test *test)
{

}