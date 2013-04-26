
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
/* btoa8.c: The opiebtoa8() library function.
%%% copyright-cmetz-96
This software is Copyright 1996-2001 by Craig Metz, All Rights Reserved.
The Inner Net License Version 3 applies to this software.
You should have received a copy of the license with this software. If
you didn't get a copy, you may request one from <license@inner.net>.

        History:

	Modified by cmetz for OPIE 2.4. Use struct opie_otpkey for binary arg. 
	Created by cmetz for OPIE 2.3 (quick re-write).
*/      

#include "opie_cfg.h"
#include "opie.h"

static char hextochar[16] = 
{'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

char *opiebtoa8 FUNCTION((out, in), char *out AND struct opie_otpkey *inkey)
{
  int i;
  unsigned char *in = (unsigned char *)inkey;
  char *c = out;

  for (i = 0; i < 8; i++) {
    *(c++) = hextochar[((*in) >> 4) & 0x0f];
    *(c++) = hextochar[(*in++) & 0x0f];
  }
  *c = 0;

  return out;
}