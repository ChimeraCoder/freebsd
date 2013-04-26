
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

#include <sys/param.h>
#include <sys/acl.h>
#include <sys/kernel.h>
#include <sys/jail.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/priv.h>
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/sysctl.h>
#include <sys/syslog.h>
#include <sys/stat.h>

#include <security/mac/mac_policy.h>
#include <security/mac_bsdextended/mac_bsdextended.h>
#include <security/mac_bsdextended/ugidfw_internal.h>

int
ugidfw_system_check_acct(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	if (vp != NULL)
		return (ugidfw_check_vp(cred, vp, MBI_WRITE));
	else
		return (0);
}

int
ugidfw_system_check_auditctl(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	if (vp != NULL)
		return (ugidfw_check_vp(cred, vp, MBI_WRITE));
	else
		return (0);
}

int
ugidfw_system_check_swapon(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (ugidfw_check_vp(cred, vp, MBI_WRITE));
}