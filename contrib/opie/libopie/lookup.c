
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
/* lookup.c: The opielookup() library function.
%%% copyright-cmetz-96
This software is Copyright 1996-2001 by Craig Metz, All Rights Reserved.
The Inner Net License Version 3 applies to this software.
You should have received a copy of the license with this software. If
you didn't get a copy, you may request one from <license@inner.net>.

        History:

        Created by cmetz for OPIE 2.3 (re-write).
*/

#include "opie_cfg.h"
#include <stdio.h>
#include "opie.h"

int opielookup FUNCTION((opie, principal), struct opie *opie AND char *principal)
{
  int i;

  memset(opie, 0, sizeof(struct opie));
  opie->opie_principal = principal;

  if (i = __opiereadrec(opie))
    return i;

  return (opie->opie_flags & __OPIE_FLAGS_RW) ? 0 : 2;
}