
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
static const char sccsid[] = "@(#)table.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <stddef.h>
#include "chpass.h"

ENTRY list[] = {
	{ "login",		p_login,  1,   5, ": ", NULL },
	{ "password",		p_passwd, 1,   8, ": ", NULL },
	{ "uid",		p_uid,    1,   3, ": ", NULL },
	{ "gid",		p_gid,    1,   3, ": ", NULL },
	{ "class",		p_class,  1,   5, ": ", NULL },
	{ "change",		p_change, 1,   6, NULL, NULL },
	{ "expire",		p_expire, 1,   6, NULL, NULL },
#ifdef RESTRICT_FULLNAME_CHANGE		/* do not allow fullname changes */
	{ "full name",		p_gecos,  1,   9, ":,", NULL },
#else
	{ "full name",		p_gecos,  0,   9, ":,", NULL },
#endif
	{ "office phone",	p_gecos,  0,  12, ":,", NULL },
	{ "home phone",		p_gecos,  0,  10, ":,", NULL },
	{ "office location",	p_gecos,  0,  15, ":,", NULL },
	{ "other information",	p_gecos,  0,  11, ": ", NULL },
	{ "home directory",	p_hdir,   1,  14, ": ", NULL },
	{ "shell",		p_shell,  0,   5, ": ", NULL },
	{ NULL, NULL, 0, 0, NULL, NULL },
};