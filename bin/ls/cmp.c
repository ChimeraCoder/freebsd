
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)cmp.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");


#include <sys/types.h>
#include <sys/stat.h>

#include <fts.h>
#include <string.h>

#include "ls.h"
#include "extern.h"

int
namecmp(const FTSENT *a, const FTSENT *b)
{

	return (strcoll(a->fts_name, b->fts_name));
}

int
revnamecmp(const FTSENT *a, const FTSENT *b)
{

	return (strcoll(b->fts_name, a->fts_name));
}

int
modcmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_mtim.tv_sec >
	    a->fts_statp->st_mtim.tv_sec)
		return (1);
	if (b->fts_statp->st_mtim.tv_sec <
	    a->fts_statp->st_mtim.tv_sec)
		return (-1);
	if (b->fts_statp->st_mtim.tv_nsec >
	    a->fts_statp->st_mtim.tv_nsec)
		return (1);
	if (b->fts_statp->st_mtim.tv_nsec <
	    a->fts_statp->st_mtim.tv_nsec)
		return (-1);
	if (f_samesort)
		return (strcoll(b->fts_name, a->fts_name));
	else
		return (strcoll(a->fts_name, b->fts_name));
}

int
revmodcmp(const FTSENT *a, const FTSENT *b)
{

	return (modcmp(b, a));
}

int
acccmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_atim.tv_sec >
	    a->fts_statp->st_atim.tv_sec)
		return (1);
	if (b->fts_statp->st_atim.tv_sec <
	    a->fts_statp->st_atim.tv_sec)
		return (-1);
	if (b->fts_statp->st_atim.tv_nsec >
	    a->fts_statp->st_atim.tv_nsec)
		return (1);
	if (b->fts_statp->st_atim.tv_nsec <
	    a->fts_statp->st_atim.tv_nsec)
		return (-1);
	if (f_samesort)
		return (strcoll(b->fts_name, a->fts_name));
	else
		return (strcoll(a->fts_name, b->fts_name));
}

int
revacccmp(const FTSENT *a, const FTSENT *b)
{

	return (acccmp(b, a));
}

int
birthcmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_birthtim.tv_sec >
	    a->fts_statp->st_birthtim.tv_sec)
		return (1);
	if (b->fts_statp->st_birthtim.tv_sec <
	    a->fts_statp->st_birthtim.tv_sec)
		return (-1);
	if (b->fts_statp->st_birthtim.tv_nsec >
	    a->fts_statp->st_birthtim.tv_nsec)
		return (1);
	if (b->fts_statp->st_birthtim.tv_nsec <
	    a->fts_statp->st_birthtim.tv_nsec)
		return (-1);
	if (f_samesort)
		return (strcoll(b->fts_name, a->fts_name));
	else
		return (strcoll(a->fts_name, b->fts_name));
}

int
revbirthcmp(const FTSENT *a, const FTSENT *b)
{

	return (birthcmp(b, a));
}

int
statcmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_ctim.tv_sec >
	    a->fts_statp->st_ctim.tv_sec)
		return (1);
	if (b->fts_statp->st_ctim.tv_sec <
	    a->fts_statp->st_ctim.tv_sec)
		return (-1);
	if (b->fts_statp->st_ctim.tv_nsec >
	    a->fts_statp->st_ctim.tv_nsec)
		return (1);
	if (b->fts_statp->st_ctim.tv_nsec <
	    a->fts_statp->st_ctim.tv_nsec)
		return (-1);
	if (f_samesort)
		return (strcoll(b->fts_name, a->fts_name));
	else
		return (strcoll(a->fts_name, b->fts_name));
}

int
revstatcmp(const FTSENT *a, const FTSENT *b)
{

	return (statcmp(b, a));
}

int
sizecmp(const FTSENT *a, const FTSENT *b)
{

	if (b->fts_statp->st_size > a->fts_statp->st_size)
		return (1);
	if (b->fts_statp->st_size < a->fts_statp->st_size)
		return (-1);
	return (strcoll(a->fts_name, b->fts_name));
}

int
revsizecmp(const FTSENT *a, const FTSENT *b)
{

	return (sizecmp(b, a));
}