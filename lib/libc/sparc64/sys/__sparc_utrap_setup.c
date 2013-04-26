
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

#include <machine/utrap.h>
#include <machine/sysarch.h>

#include <stdlib.h>

#include "__sparc_utrap_private.h"

static const struct sparc_utrap_args ua[] = {
	{ UT_FP_DISABLED, __sparc_utrap_fp_disabled, NULL, NULL, NULL },
	{ UT_FP_EXCEPTION_IEEE_754, __sparc_utrap_gen, NULL, NULL, NULL },
	{ UT_FP_EXCEPTION_OTHER, __sparc_utrap_gen, NULL, NULL, NULL },
	{ UT_ILLEGAL_INSTRUCTION, __sparc_utrap_gen, NULL, NULL, NULL },
	{ UT_MEM_ADDRESS_NOT_ALIGNED, __sparc_utrap_gen, NULL, NULL, NULL },
};

static const struct sparc_utrap_install_args uia[] = {
	{ sizeof (ua) / sizeof (*ua), ua }
};

void __sparc_utrap_setup(void) __attribute__((constructor));

void
__sparc_utrap_setup(void)
{

	sysarch(SPARC_UTRAP_INSTALL, &uia);
}