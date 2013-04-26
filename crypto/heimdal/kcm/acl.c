
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

#include "kcm_locl.h"

krb5_error_code
kcm_access(krb5_context context,
	   kcm_client *client,
	   kcm_operation opcode,
	   kcm_ccache ccache)
{
    int read_p = 0;
    int write_p = 0;
    uint16_t mask;
    krb5_error_code ret;

    KCM_ASSERT_VALID(ccache);

    switch (opcode) {
    case KCM_OP_INITIALIZE:
    case KCM_OP_DESTROY:
    case KCM_OP_STORE:
    case KCM_OP_REMOVE_CRED:
    case KCM_OP_SET_FLAGS:
    case KCM_OP_CHOWN:
    case KCM_OP_CHMOD:
    case KCM_OP_GET_INITIAL_TICKET:
    case KCM_OP_GET_TICKET:
    case KCM_OP_MOVE_CACHE:
    case KCM_OP_SET_DEFAULT_CACHE:
    case KCM_OP_SET_KDC_OFFSET:
	write_p = 1;
	read_p = 0;
	break;
    case KCM_OP_NOOP:
    case KCM_OP_GET_NAME:
    case KCM_OP_RESOLVE:
    case KCM_OP_GEN_NEW:
    case KCM_OP_RETRIEVE:
    case KCM_OP_GET_PRINCIPAL:
    case KCM_OP_GET_CRED_UUID_LIST:
    case KCM_OP_GET_CRED_BY_UUID:
    case KCM_OP_GET_CACHE_UUID_LIST:
    case KCM_OP_GET_CACHE_BY_UUID:
    case KCM_OP_GET_DEFAULT_CACHE:
    case KCM_OP_GET_KDC_OFFSET:
	write_p = 0;
	read_p = 1;
	break;
    default:
	ret = KRB5_FCC_PERM;
	goto out;
    }

    if (ccache->flags & KCM_FLAGS_OWNER_IS_SYSTEM) {
	/* System caches cannot be reinitialized or destroyed by users */
	if (opcode == KCM_OP_INITIALIZE ||
	    opcode == KCM_OP_DESTROY ||
	    opcode == KCM_OP_REMOVE_CRED ||
	    opcode == KCM_OP_MOVE_CACHE) {
	    ret = KRB5_FCC_PERM;
	    goto out;
	}

	/* Let root always read system caches */
	if (CLIENT_IS_ROOT(client)) {
	    ret = 0;
	    goto out;
	}
    }

    /* start out with "other" mask */
    mask = S_IROTH|S_IWOTH;

    /* root can do anything */
    if (CLIENT_IS_ROOT(client)) {
	if (read_p)
	    mask |= S_IRUSR|S_IRGRP|S_IROTH;
	if (write_p)
	    mask |= S_IWUSR|S_IWGRP|S_IWOTH;
    }
    /* same session same as owner */
    if (kcm_is_same_session(client, ccache->uid, ccache->session)) {
	if (read_p)
	    mask |= S_IROTH;
	if (write_p)
	    mask |= S_IWOTH;
    }
    /* owner */
    if (client->uid == ccache->uid) {
	if (read_p)
	    mask |= S_IRUSR;
	if (write_p)
	    mask |= S_IWUSR;
    }
    /* group */
    if (client->gid == ccache->gid) {
	if (read_p)
	    mask |= S_IRGRP;
	if (write_p)
	    mask |= S_IWGRP;
    }

    ret = (ccache->mode & mask) ? 0 : KRB5_FCC_PERM;

out:
    if (ret) {
	kcm_log(2, "Process %d is not permitted to call %s on cache %s",
		client->pid, kcm_op2string(opcode), ccache->name);
    }

    return ret;
}

krb5_error_code
kcm_chmod(krb5_context context,
	  kcm_client *client,
	  kcm_ccache ccache,
	  uint16_t mode)
{
    KCM_ASSERT_VALID(ccache);

    /* System cache mode can only be set at startup */
    if (ccache->flags & KCM_FLAGS_OWNER_IS_SYSTEM)
	return KRB5_FCC_PERM;

    if (ccache->uid != client->uid)
	return KRB5_FCC_PERM;

    if (ccache->gid != client->gid)
	return KRB5_FCC_PERM;

    HEIMDAL_MUTEX_lock(&ccache->mutex);

    ccache->mode = mode;

    HEIMDAL_MUTEX_unlock(&ccache->mutex);

    return 0;
}

krb5_error_code
kcm_chown(krb5_context context,
	  kcm_client *client,
	  kcm_ccache ccache,
	  uid_t uid,
	  gid_t gid)
{
    KCM_ASSERT_VALID(ccache);

    /* System cache owner can only be set at startup */
    if (ccache->flags & KCM_FLAGS_OWNER_IS_SYSTEM)
	return KRB5_FCC_PERM;

    if (ccache->uid != client->uid)
	return KRB5_FCC_PERM;

    if (ccache->gid != client->gid)
	return KRB5_FCC_PERM;

    HEIMDAL_MUTEX_lock(&ccache->mutex);

    ccache->uid = uid;
    ccache->gid = gid;

    HEIMDAL_MUTEX_unlock(&ccache->mutex);

    return 0;
}