
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

#ifndef lint
static const char sccsid[] = "@(#)talk.c	8.1 (Berkeley) 6/6/93";
#endif

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#include <locale.h>
#include <unistd.h>

#include "talk.h"

/*
 * talk:	A visual form of write. Using sockets, a two way
 *		connection is set up between the two people talking.
 *		With the aid of curses, the screen is split into two
 *		windows, and each users text is added to the window,
 *		one character at a time...
 *
 *		Written by Kipp Hickman
 *
 *		Modified to run under 4.1a by Clem Cole and Peter Moore
 *		Modified to run between hosts by Peter Moore, 8/19/82
 *		Modified to run under 4.1c by Peter Moore 3/17/83
 *		Fixed to not run with unwriteable terminals MRVM 28/12/94
 */

int
main(int argc, char **argv)
{
	(void) setlocale(LC_CTYPE, "");

	get_names(argc, argv);
	setproctitle(" ");
	check_writeable();
	init_display();
	open_ctl();
	open_sockt();
	start_msgs();
	if (!check_local())
		invite_remote();
	end_msgs();
	set_edit_chars();
	talk();
	return 0;
}