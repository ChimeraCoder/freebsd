
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
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>

#include "sanity.h"
#include "user.h"

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

uid_t
atf_user_euid(void)
{
    return geteuid();
}

bool
atf_user_is_member_of_group(gid_t gid)
{
    static gid_t groups[NGROUPS_MAX];
    static int ngroups = -1;
    bool found;
    int i;

    if (ngroups == -1) {
        ngroups = getgroups(NGROUPS_MAX, groups);
        INV(ngroups >= 0);
    }

    found = false;
    for (i = 0; !found && i < ngroups; i++)
        if (groups[i] == gid)
            found = true;
    return found;
}

bool
atf_user_is_root(void)
{
    return geteuid() == 0;
}

bool
atf_user_is_unprivileged(void)
{
    return geteuid() != 0;
}