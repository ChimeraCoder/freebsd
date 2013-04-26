
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
///////////////////////////////////////////////////////////////////////////////
//
/// \file       tuklib_progname.c
/// \brief      Program name to be displayed in messages
//
//  Author:     Lasse Collin
//
//  This file has been put into the public domain.
//  You can do whatever you want with this file.
//
///////////////////////////////////////////////////////////////////////////////
#include "tuklib_progname.h"
#include <string.h>


#if !HAVE_DECL_PROGRAM_INVOCATION_NAME
char *progname = NULL;
#endif


extern void
tuklib_progname_init(char **argv)
{
#ifdef TUKLIB_DOSLIKE
	// On these systems, argv[0] always has the full path and .exe
	// suffix even if the user just types the plain program name.
	// We modify argv[0] to make it nicer to read.

	// Strip the leading path.
	char *p = argv[0] + strlen(argv[0]);
	while (argv[0] < p && p[-1] != '/' && p[-1] != '\\')
		--p;

	argv[0] = p;

	// Strip the .exe suffix.
	p = strrchr(p, '.');
	if (p != NULL)
		*p = '\0';

	// Make it lowercase.
	for (p = argv[0]; *p != '\0'; ++p)
		if (*p >= 'A' && *p <= 'Z')
			*p = *p - 'A' + 'a';
#endif

	progname = argv[0];
	return;
}