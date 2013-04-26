
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
 * MAC checks for system privileges.
 */

#include "sys/cdefs.h"
__FBSDID("$FreeBSD$");

#include "opt_kdtrace.h"
#include "opt_mac.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/priv.h>
#include <sys/sdt.h>
#include <sys/module.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

/*
 * The MAC Framework interacts with kernel privilege checks in two ways: it
 * may restrict the granting of privilege to a subject, and it may grant
 * additional privileges to the subject.  Policies may implement none, one,
 * or both of these entry points.  Restriction of privilege by any policy
 * always overrides granting of privilege by any policy or other privilege
 * mechanism.  See kern_priv.c:priv_check_cred() for details of the
 * composition.
 */

MAC_CHECK_PROBE_DEFINE2(priv_check, "struct ucred *", "int");

/*
 * Restrict access to a privilege for a credential.  Return failure if any
 * policy denies access.
 */
int
mac_priv_check(struct ucred *cred, int priv)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(priv_check, cred, priv);
	MAC_CHECK_PROBE2(priv_check, error, cred, priv);

	return (error);
}

MAC_GRANT_PROBE_DEFINE2(priv_grant, "struct ucred *", "int");

/*
 * Grant access to a privilege for a credential.  Return success if any
 * policy grants access.
 */
int
mac_priv_grant(struct ucred *cred, int priv)
{
	int error;

	MAC_POLICY_GRANT_NOSLEEP(priv_grant, cred, priv);
	MAC_GRANT_PROBE2(priv_grant, error, cred, priv);

	return (error);
}