
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

#include <machine/sysarch.h>

int
i386_get_ioperm(unsigned int start, unsigned int *length, int *enable)
{
	struct i386_ioperm_args p;
	int error;

	p.start = start;
	p.length = *length;
	p.enable = *enable;

	error = sysarch(I386_GET_IOPERM, &p);

	*length = p.length;
	*enable = p.enable;

	return (error);
}