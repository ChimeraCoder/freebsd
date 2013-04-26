
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
i386_set_watch(int watchnum, unsigned int watchaddr, int size,
               int access, struct dbreg * d)
{
	int i;
	unsigned int mask;

	if (watchnum == -1) {
		for (i = 0, mask = 0x3; i < 4; i++, mask <<= 2)
			if ((DBREG_DRX(d,7) & mask) == 0)
				break;
		if (i < 4)
			watchnum = i;
		else
			return -1;
	}

	switch (access) {
	case DBREG_DR7_EXEC:
		size = 1; /* size must be 1 for an execution breakpoint */
		/* fall through */
	case DBREG_DR7_WRONLY:
	case DBREG_DR7_RDWR:
		break;
	default : return -1; break;
	}

	/*
	 * we can watch a 1, 2, or 4 byte sized location
	 */
	switch (size) {
	case 1  : mask = 0x00; break;
	case 2  : mask = 0x01 << 2; break;
	case 4  : mask = 0x03 << 2; break;
	default : return -1; break;
	}

	mask |= access;

	/* clear the bits we are about to affect */
	DBREG_DRX(d,7) &= ~((0x3 << (watchnum*2)) | (0x0f << (watchnum*4+16)));

	/* set drN register to the address, N=watchnum */
	DBREG_DRX(d,watchnum) = watchaddr;

	/* enable the watchpoint */
	DBREG_DRX(d,7) |= (0x2 << (watchnum*2)) | (mask << (watchnum*4+16));

	return watchnum;
}