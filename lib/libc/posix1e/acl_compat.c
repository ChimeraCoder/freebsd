
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

#include <sys/acl.h>

/*
 * Compatibility wrappers for applications compiled against libc from before
 * NFSv4 ACLs were added.
 */
int
__oldacl_get_perm_np(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_get_perm_np(permset_d, perm));
}

int
__oldacl_add_perm(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_add_perm(permset_d, perm));
}

int
__oldacl_delete_perm(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_delete_perm(permset_d, perm));
}

__sym_compat(acl_get_perm_np, __oldacl_get_perm_np, FBSD_1.0);
__sym_compat(acl_add_perm, __oldacl_add_perm, FBSD_1.0);
__sym_compat(acl_delete_perm, __oldacl_delete_perm, FBSD_1.0);