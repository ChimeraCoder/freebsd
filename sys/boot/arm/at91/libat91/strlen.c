
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

/******************************************************************************
 *
 * Filename: p_string.c
 *
 * Instantiation of basic string operations to prevent inclusion of full
 * string library.  These are simple implementations not necessarily optimized
 * for speed, but rather to show intent.
 *
 * Revision information:
 *
 * 20AUG2004	kb_admin	initial creation
 * 12JAN2005	kb_admin	minor updates
 *
 * BEGIN_KBDD_BLOCK
 * No warranty, expressed or implied, is included with this software.  It is
 * provided "AS IS" and no warranty of any kind including statutory or aspects
 * relating to merchantability or fitness for any purpose is provided.  All
 * intellectual property rights of others is maintained with the respective
 * owners.  This software is not copyrighted and is intended for reference
 * only.
 * END_BLOCK
 *****************************************************************************/

#include "lib.h"

/*
 * .KB_C_FN_DEFINITION_START
 * int p_strlen(char *)
 *  This global function returns the number of bytes starting at the pointer
 * before (not including) the string termination character ('/0').
 * .KB_C_FN_DEFINITION_END
 */
int
p_strlen(const char *buffer)
{
	const char *ptr = buffer;
	while (*ptr++)
		continue;
	return (ptr - buffer - 1);
}