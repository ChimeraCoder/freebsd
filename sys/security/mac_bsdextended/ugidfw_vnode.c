
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
ugidfw_vnode_check_access(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, accmode_t accmode)
{

	return (ugidfw_check_vp(cred, vp, ugidfw_accmode2mbi(accmode)));
}

int
ugidfw_vnode_check_chdir(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel)
{

	return (ugidfw_check_vp(cred, dvp, MBI_EXEC));
}

int
ugidfw_vnode_check_chroot(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel)
{

	return (ugidfw_check_vp(cred, dvp, MBI_EXEC));
}

int
ugidfw_check_create_vnode(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct componentname *cnp, struct vattr *vap)
{

	return (ugidfw_check_vp(cred, dvp, MBI_WRITE));
}

int
ugidfw_vnode_check_deleteacl(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, acl_type_t type)
{

	return (ugidfw_check_vp(cred, vp, MBI_ADMIN));
}

int
ugidfw_vnode_check_deleteextattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int attrnamespace, const char *name)
{

	return (ugidfw_check_vp(cred, vp, MBI_WRITE));
}

int
ugidfw_vnode_check_exec(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, struct image_params *imgp,
    struct label *execlabel)
{

	return (ugidfw_check_vp(cred, vp, MBI_READ|MBI_EXEC));
}

int
ugidfw_vnode_check_getacl(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, acl_type_t type)
{

	return (ugidfw_check_vp(cred, vp, MBI_STAT));
}

int
ugidfw_vnode_check_getextattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int attrnamespace, const char *name)
{

	return (ugidfw_check_vp(cred, vp, MBI_READ));
}

int
ugidfw_vnode_check_link(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct vnode *vp, struct label *label,
    struct componentname *cnp)
{
	int error;

	error = ugidfw_check_vp(cred, dvp, MBI_WRITE);
	if (error)
		return (error);
	error = ugidfw_check_vp(cred, vp, MBI_WRITE);
	if (error)
		return (error);
	return (0);
}

int
ugidfw_vnode_check_listextattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int attrnamespace)
{

	return (ugidfw_check_vp(cred, vp, MBI_READ));
}

int
ugidfw_vnode_check_lookup(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct componentname *cnp)
{

	return (ugidfw_check_vp(cred, dvp, MBI_EXEC));
}

int
ugidfw_vnode_check_open(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, accmode_t accmode)
{

	return (ugidfw_check_vp(cred, vp, ugidfw_accmode2mbi(accmode)));
}

int
ugidfw_vnode_check_readdir(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel)
{

	return (ugidfw_check_vp(cred, dvp, MBI_READ));
}

int
ugidfw_vnode_check_readdlink(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (ugidfw_check_vp(cred, vp, MBI_READ));
}

int
ugidfw_vnode_check_rename_from(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct vnode *vp, struct label *vplabel,
    struct componentname *cnp)
{
	int error;

	error = ugidfw_check_vp(cred, dvp, MBI_WRITE);
	if (error)
		return (error);
	return (ugidfw_check_vp(cred, vp, MBI_WRITE));
}

int
ugidfw_vnode_check_rename_to(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct vnode *vp, struct label *vplabel,
    int samedir, struct componentname *cnp)
{
	int error;

	error = ugidfw_check_vp(cred, dvp, MBI_WRITE);
	if (error)
		return (error);
	if (vp != NULL)
		error = ugidfw_check_vp(cred, vp, MBI_WRITE);
	return (error);
}

int
ugidfw_vnode_check_revoke(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (ugidfw_check_vp(cred, vp, MBI_ADMIN));
}

int
ugidfw_check_setacl_vnode(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, acl_type_t type, struct acl *acl)
{

	return (ugidfw_check_vp(cred, vp, MBI_ADMIN));
}

int
ugidfw_vnode_check_setextattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int attrnamespace, const char *name)
{

	return (ugidfw_check_vp(cred, vp, MBI_WRITE));
}

int
ugidfw_vnode_check_setflags(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, u_long flags)
{

	return (ugidfw_check_vp(cred, vp, MBI_ADMIN));
}

int
ugidfw_vnode_check_setmode(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, mode_t mode)
{

	return (ugidfw_check_vp(cred, vp, MBI_ADMIN));
}

int
ugidfw_vnode_check_setowner(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, uid_t uid, gid_t gid)
{

	return (ugidfw_check_vp(cred, vp, MBI_ADMIN));
}

int
ugidfw_vnode_check_setutimes(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, struct timespec atime, struct timespec utime)
{

	return (ugidfw_check_vp(cred, vp, MBI_ADMIN));
}

int
ugidfw_vnode_check_stat(struct ucred *active_cred,
    struct ucred *file_cred, struct vnode *vp, struct label *vplabel)
{

	return (ugidfw_check_vp(active_cred, vp, MBI_STAT));
}

int
ugidfw_vnode_check_unlink(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct vnode *vp, struct label *vplabel,
    struct componentname *cnp)
{
	int error;

	error = ugidfw_check_vp(cred, dvp, MBI_WRITE);
	if (error)
		return (error);
	return (ugidfw_check_vp(cred, vp, MBI_WRITE));
}