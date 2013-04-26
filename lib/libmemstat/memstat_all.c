
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

#include <sys/types.h>
#include <sys/queue.h>

#include "memstat.h"

/*
 * Query all available memory allocator sources.  Currently this consists of
 * malloc(9) and UMA(9).
 */
int
memstat_sysctl_all(struct memory_type_list *mtlp, int flags)
{

	if (memstat_sysctl_malloc(mtlp, flags) < 0)
		return (-1);
	if (memstat_sysctl_uma(mtlp, flags) < 0)
		return (-1);
	return (0);
}

int
memstat_kvm_all(struct memory_type_list *mtlp, void *kvm_handle)
{

	if (memstat_kvm_malloc(mtlp, kvm_handle) < 0)
		return (-1);
	if (memstat_kvm_uma(mtlp, kvm_handle) < 0)
		return (-1);
	return (0);
}