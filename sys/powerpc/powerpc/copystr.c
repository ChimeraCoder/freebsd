
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

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/systm.h>

/*
 * Emulate copyinstr.
 */
int
copystr(kfaddr, kdaddr, len, done)
	const void *kfaddr;
	void *kdaddr;
	size_t len;
	size_t *done;
{
	const u_char *kfp = kfaddr;
	u_char *kdp = kdaddr;
	size_t l;
	int rv;
	
	rv = ENAMETOOLONG;
	for (l = 0; len-- > 0; l++) {
		if (!(*kdp++ = *kfp++)) {
			l++;
			rv = 0;
			break;
		}
	}
	if (done != NULL) {
		*done = l;
	}
	return rv;
}