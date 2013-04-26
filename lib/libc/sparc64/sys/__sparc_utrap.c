
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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fpu_extern.h"
#include "__sparc_utrap_private.h"

extern ssize_t __sys_write(int, const void *, size_t);
extern int __sys_kill(pid_t, int);
extern pid_t __sys_getpid(void);

static const char *utrap_msg[] = {
	"reserved",
	"instruction access exception",
	"instruction access error",
	"instruction access protection",
	"illtrap instruction",
	"illegal instruction",
	"privileged opcode",
	"floating point disabled",
	"floating point exception ieee 754",
	"floating point exception other",
	"tag overflow",
	"division by zero",
	"data access exception",
	"data access error",
	"data access protection",
	"memory address not aligned",
	"privileged action",
	"async data error",
	"trap instruction 16",
	"trap instruction 17",
	"trap instruction 18",
	"trap instruction 19",
	"trap instruction 20",
	"trap instruction 21",
	"trap instruction 22",
	"trap instruction 23",
	"trap instruction 24",
	"trap instruction 25",
	"trap instruction 26",
	"trap instruction 27",
	"trap instruction 28",
	"trap instruction 29",
	"trap instruction 30",
	"trap instruction 31",
};

void
__sparc_utrap(struct utrapframe *uf)
{
	int sig;

	switch (uf->uf_type) {
	case UT_FP_EXCEPTION_IEEE_754:
	case UT_FP_EXCEPTION_OTHER:
		sig = __fpu_exception(uf);
		break;
	case UT_ILLEGAL_INSTRUCTION:
		sig = __emul_insn(uf);
		break;
	case UT_MEM_ADDRESS_NOT_ALIGNED:
		sig = __unaligned_fixup(uf);
		break;
	default:
		break;
	}
	if (sig) {
		__utrap_write("__sparc_utrap: fatal ");
		__utrap_write(utrap_msg[uf->uf_type]);
		__utrap_write("\n");
		__utrap_kill_self(sig);
	}
	UF_DONE(uf);
}

void
__utrap_write(const char *str)
{
	int berrno;

	berrno = errno;
	__sys_write(STDERR_FILENO, str, strlen(str));
	errno = berrno;
}

void
__utrap_kill_self(int sig)
{
	int berrno;

	berrno = errno;
	__sys_kill(__sys_getpid(), sig);
	errno = berrno;
}

void
__utrap_panic(const char *msg)
{

	__utrap_write(msg);
	__utrap_write("\n");
	__utrap_kill_self(SIGKILL);
}