
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
#include <rtld_tls.h>

#include "pthread_md.h"

/*
 * The constructors.
 */
struct tcb *
_tcb_ctor(struct pthread *thread, int initial)
{
	struct tcb *tcb;

	if (initial)
		__asm __volatile("movq %%fs:0, %0" : "=r" (tcb));
	else
		tcb = _rtld_allocate_tls(NULL, sizeof(struct tcb), 16);
	if (tcb)
		tcb->tcb_thread = thread;
	return (tcb);
}

void
_tcb_dtor(struct tcb *tcb)
{
	_rtld_free_tls(tcb, sizeof(struct tcb), 16);
}