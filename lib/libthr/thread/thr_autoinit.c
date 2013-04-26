
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

#include <pthread.h>

#include "thr_private.h"

/*
 * This module uses GCC extentions to initialize the
 * threads package at program start-up time.
 */

void	_thread_init_hack(void) __attribute__ ((constructor));

void
_thread_init_hack(void)
{

	_thread_init();
}

/*
 * For the shared version of the threads library, the above is sufficient.
 * But for the archive version of the library, we need a little bit more.
 * Namely, we must arrange for this particular module to be pulled in from
 * the archive library at link time.  To accomplish that, we define and
 * initialize a variable, "_thread_autoinit_dummy_decl".  This variable is
 * referenced (as an extern) from libc/stdlib/exit.c. This will always
 * create a need for this module, ensuring that it is present in the
 * executable.
 */
extern int _thread_autoinit_dummy_decl;
int _thread_autoinit_dummy_decl = 0;