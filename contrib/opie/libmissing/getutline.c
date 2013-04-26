
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
/* getutline.c: A replacement for the getutline() function
%%% copyright-cmetz-96
This software is Copyright 1996-2001 by Craig Metz, All Rights Reserved.
The Inner Net License Version 3 applies to this software.
You should have received a copy of the license with this software. If
you didn't get a copy, you may request one from <license@inner.net>.

        History:

	Modified by cmetz for OPIE 2.32. Fixed check for fread() return
		value.
	Modified by cmetz for OPIE 2.31. If the OS won't tell us where
		_PATH_UTMP is, play the SVID game, then use
		Autoconf-discovered values.
	Created by cmetz for OPIE 2.3.
*/

#include "opie_cfg.h"
#include <stdio.h>
#include <utmp.h>
#include "opie.h"

static struct utmp u;

#ifndef _PATH_UTMP
#ifdef UTMP_FILE
#define _PATH_UTMP UTMP_FILE
#else /* UTMP_FILE */
#define _PATH_UTMP PATH_UTMP_AC
#endif /* UTMP_FILE */
#endif /* _PATH_UTMP */

struct utmp *getutline FUNCTION((utmp), struct utmp *utmp)
{
  FILE *f;
  int i;

  if (!(f = __opieopen(_PATH_UTMP, 0, 0644)))
    return 0;

#if HAVE_TTYSLOT
  if (i = ttyslot()) {
    if (fseek(f, i * sizeof(struct utmp), SEEK_SET) < 0)
      goto ret;
    if (fread(&u, sizeof(struct utmp), 1, f) != 1)
      goto ret;
    fclose(f);
    return &u;
  }
#endif /* HAVE_TTYSLOT */

  while(fread(&u, sizeof(struct utmp), 1, f) == 1) {
    if (!strncmp(utmp->ut_line, u.ut_line, sizeof(u.ut_line) - 1)) {
      fclose(f);
      return &u;
    }
  }

ret:
  fclose(f);
  return NULL;
}