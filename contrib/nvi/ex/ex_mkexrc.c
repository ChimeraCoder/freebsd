
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

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)ex_mkexrc.c	10.11 (Berkeley) 3/6/96";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/stat.h>

#include <bitstring.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../common/common.h"
#include "pathnames.h"

/*
 * ex_mkexrc -- :mkexrc[!] [file]
 *
 * Create (or overwrite) a .exrc file with the current info.
 *
 * PUBLIC: int ex_mkexrc __P((SCR *, EXCMD *));
 */
int
ex_mkexrc(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	struct stat sb;
	FILE *fp;
	int fd, sverrno;
	char *fname;

	switch (cmdp->argc) {
	case 0:
		fname = _PATH_EXRC;
		break;
	case 1:
		fname = cmdp->argv[0]->bp;
		set_alt_name(sp, fname);
		break;
	default:
		abort();
	}

	if (!FL_ISSET(cmdp->iflags, E_C_FORCE) && !stat(fname, &sb)) {
		msgq_str(sp, M_ERR, fname,
		    "137|%s exists, not written; use ! to override");
		return (1);
	}

	/* Create with max permissions of rw-r--r--. */
	if ((fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY,
	    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		msgq_str(sp, M_SYSERR, fname, "%s");
		return (1);
	}

	if ((fp = fdopen(fd, "w")) == NULL) {
		sverrno = errno;
		(void)close(fd);
		goto e2;
	}

	if (seq_save(sp, fp, "abbreviate ", SEQ_ABBREV) || ferror(fp))
		goto e1;
	if (seq_save(sp, fp, "map ", SEQ_COMMAND) || ferror(fp))
		goto e1;
	if (seq_save(sp, fp, "map! ", SEQ_INPUT) || ferror(fp))
		goto e1;
	if (opts_save(sp, fp) || ferror(fp))
		goto e1;
	if (fclose(fp)) {
		sverrno = errno;
		goto e2;
	}

	msgq_str(sp, M_INFO, fname, "138|New exrc file: %s");
	return (0);

e1:	sverrno = errno;
	(void)fclose(fp);
e2:	errno = sverrno;
	msgq_str(sp, M_SYSERR, fname, "%s");
	return (1);
}