
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

#include <machine/reg.h>
#include <machine/sysarch.h>

int
i386_clr_watch(int watchnum, struct dbreg * d)
{

	if (watchnum < 0 || watchnum >= 4)
		return -1;

	DBREG_DRX(d,7) = DBREG_DRX(d,7) & ~((0x3 << (watchnum*2)) | (0x0f << (watchnum*4+16)));
        DBREG_DRX(d,watchnum) = 0;

	return 0;
}