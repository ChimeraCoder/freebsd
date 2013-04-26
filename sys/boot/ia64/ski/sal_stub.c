
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
#include <machine/md_var.h>
#include <machine/sal.h>
#include <stand.h>
#include "libski.h"

extern void PalProc(void);
static sal_entry_t SalProc;

struct {
	struct sal_system_table header;
	struct sal_entrypoint_descriptor entry;
	struct sal_ap_wakeup_descriptor wakeup;
} sal_systab = {
	/* Header. */
	{
		SAL_SIGNATURE,
		sizeof(sal_systab),
		{ 00, 03 },		/* Revision 3.0. */
		2,			/* Number of decsriptors. */
		0,			/* XXX checksum. */
		{ 0 },
		{ 00, 00 },		/* XXX SAL_A version. */
		{ 00, 00 },		/* XXX SAL_B version. */
		"FreeBSD",
		"Ski loader",
		{ 0 }
	},
	/* Entrypoint. */
	{
		0,			/* Type=entrypoint descr. */
		{ 0 },
		0,			/* XXX PalProc. */
		0,			/* XXX SalProc. */
		0,			/* XXX SalProc GP. */
		{ 0 }
	},
	/* AP wakeup. */
	{
		5,			/* Type=AP wakeup descr. */
		0,			/* External interrupt. */
		{ 0 },
		255			/* Wakeup vector. */
	}
};

static inline void
puts(const char *s)
{
	s = (const char *)((7UL << 61) | (u_long)s);
	while (*s)
		ski_cons_putchar(*s++);
}

static struct ia64_sal_result
SalProc(u_int64_t a1, u_int64_t a2, u_int64_t a3, u_int64_t a4, u_int64_t a5,
    u_int64_t a6, u_int64_t a7, u_int64_t a8)
{
	struct ia64_sal_result res;

	res.sal_status = -3;
	res.sal_result[0] = 0;
	res.sal_result[1] = 0;
	res.sal_result[2] = 0;

	if (a1 == SAL_FREQ_BASE) {
		res.sal_status = 0;
		res.sal_result[0] = 133338184;
	} else if (a1 == SAL_SET_VECTORS) {
		/* XXX unofficial SSC function. */
		ssc(a2, a3, a4, a5, SSC_SAL_SET_VECTORS);
	} else if (a1 != SAL_GET_STATE_INFO_SIZE) {
		puts("SAL: unimplemented function called\n");
	}

	return (res);
}

void
sal_stub_init(void)
{
	struct ia64_fdesc *fd;

	fd = (void*)PalProc;
	sal_systab.entry.sale_pal_proc = fd->func;
	fd = (void*)SalProc;
	sal_systab.entry.sale_sal_proc = fd->func;
	sal_systab.entry.sale_sal_gp = fd->gp;
}