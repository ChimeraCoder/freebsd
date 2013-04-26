
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
 * Written by Alexander Kabaev <kan@FreeBSD.org>
 * The file is in public domain.
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

void __stack_chk_fail(void);

#ifdef PIC
void
__stack_chk_fail_local_hidden(void)
{

	__stack_chk_fail();
}

__sym_compat(__stack_chk_fail_local, __stack_chk_fail_local_hidden, FBSD_1.0);
#endif