
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

/* $Id: iterated_hash.c,v 1.6 2009/02/18 23:47:48 tbox Exp $ */

#include "config.h"

#include <stdio.h>

#include <isc/sha1.h>
#include <isc/iterated_hash.h>

int
isc_iterated_hash(unsigned char out[ISC_SHA1_DIGESTLENGTH],
		  unsigned int hashalg, int iterations,
		  const unsigned char *salt, int saltlength,
		  const unsigned char *in, int inlength)
{
	isc_sha1_t ctx;
	int n = 0;

	if (hashalg != 1)
		return (0);

	do {
		isc_sha1_init(&ctx);
		isc_sha1_update(&ctx, in, inlength);
		isc_sha1_update(&ctx, salt, saltlength);
		isc_sha1_final(&ctx, out);
		in = out;
		inlength = ISC_SHA1_DIGESTLENGTH;
	} while (n++ < iterations);

	return (ISC_SHA1_DIGESTLENGTH);
}