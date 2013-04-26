
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
#include <sys/libkern.h>

/*
 * Return (1) if the buffer pointed to by kernel pointer 'buffer' and
 * of length 'bufferlen' contains a valid NUL-terminated string
 */
int
strvalid(const char *buffer, size_t bufferlen)
{
	size_t i;

	/* Must be NUL-terminated. */
	for (i = 0; i < bufferlen; i++)
		if (buffer[i] == '\0')
			return (1);

	return (0);
}