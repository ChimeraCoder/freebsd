
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
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/sysproto.h>
#include <sys/syscall.h>
#include <sys/sysent.h>

#include <machine/sysarch.h>

#ifndef _SYS_SYSPROTO_H_
struct sysarch_args {
	int op;
	char *parms;
};
#endif

int
sysarch(struct thread *td, struct sysarch_args *uap)
{
	int error;
	void *tlsbase;

	switch (uap->op) {
	case MIPS_SET_TLS:
		td->td_md.md_tls = uap->parms;
		return (0);
	case MIPS_GET_TLS: 
		tlsbase = td->td_md.md_tls;
		error = copyout(&tlsbase, uap->parms, sizeof(tlsbase));
		return (error);
	default:
		break;
	}
	return (EINVAL);
}