
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
 * Developed by the TrustedBSD Project.
 *
 * Sample policy implementing no entry points; for performance measurement
 * purposes only.  If you're looking for a stub policy to base new policies
 * on, try mac_stub.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <security/mac/mac_policy.h>

static struct mac_policy_ops none_ops =
{
};

MAC_POLICY_SET(&none_ops, mac_none, "TrustedBSD MAC/None",
    MPC_LOADTIME_FLAG_UNLOADOK, NULL);