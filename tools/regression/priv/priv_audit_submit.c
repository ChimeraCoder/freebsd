
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
 * Confirm that privilege is required to submit an audit record; we don't
 * actually submit a record, but instead rely on the fact that length
 * validation of the record will occur after the kernel privilege check.
 *
 * XXX: It might be better to submit a nul record of some sort.
 */

#include <sys/types.h>

#include <bsm/audit.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

int
priv_audit_submit_setup(int asroot, int injail, struct test *test)
{

	/*
	 * XXXRW: It would be nice if we checked for audit being configured
	 * here.
	 */
	return (0);
}

void
priv_audit_submit(int asroot, int injail, struct test *test)
{
	char record[MAX_AUDIT_RECORD_SIZE+10];
	int error;

	bzero(record, sizeof(record));
	error = audit(record, sizeof(record));
	if (asroot && injail)
		expect("priv_audit_submit(asroot, injail)", error, -1,
		    ENOSYS);
	if (asroot && !injail)
		expect("priv_audit_submit(asroot, !injail)", error, -1,
		    EINVAL);
	if (!asroot && injail)
		expect("priv_audit_submit(!asroot, injail)", error, -1,
		    ENOSYS);
	if (!asroot && !injail)
		expect("priv_audit_submit(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_audit_submit_cleanup(int asroot, int injail, struct test *test)
{

}