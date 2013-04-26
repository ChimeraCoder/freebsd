
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

/*
 * Common code for dealing with 3COM ethernet cards.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <machine/cpufunc.h>

#include <i386/isa/elink.h>

/*
 * Issue a `global reset' to all cards.  We have to be careful to do this only
 * once during autoconfig, to prevent resetting boards that have already been
 * configured.
 */
void
elink_reset()
{
	static int x = 0;

	if (x == 0) {
		x = 1;
		outb(ELINK_ID_PORT, ELINK_RESET);
	}
	outb(ELINK_ID_PORT, 0);
	outb(ELINK_ID_PORT, 0);

	return;
}

/*
 * The `ID sequence' is really just snapshots of an 8-bit CRC register as 0
 * bits are shifted in.  Different board types use different polynomials.
 */
void
elink_idseq(u_char p)
{
	register int i;
	register u_char c;

	c = 0xff;
	for (i = 255; i; i--) {
		outb(ELINK_ID_PORT, c);
		if (c & 0x80) {
			c <<= 1;
			c ^= p;
		} else
			c <<= 1;
	}
}

static moduledata_t elink_mod = {
	"elink",/* module name */
	NULL,	/* event handler */
	0	/* extra data */
};

DECLARE_MODULE(elink, elink_mod, SI_SUB_PSEUDO, SI_ORDER_ANY);
MODULE_VERSION(elink, 1);