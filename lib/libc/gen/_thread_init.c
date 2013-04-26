
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

#include <sys/types.h>

void _thread_init_stub(void);

__weak_reference(_thread_init_stub, _thread_init);
__weak_reference(_thread_autoinit_dummy_decl_stub,
    _thread_autoinit_dummy_decl);

int	_thread_autoinit_dummy_decl_stub = 0;

void
_thread_init_stub(void)
{
	/* This is just a stub; there is nothing to do. */
}