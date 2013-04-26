
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

/* $Id: base64.c,v 1.8 2007/08/28 07:20:43 tbox Exp $ */

/*! \file */

#include <config.h>

#include <isc/base64.h>
#include <isc/buffer.h>
#include <isc/region.h>
#include <isc/result.h>

#include <isccc/base64.h>
#include <isccc/result.h>
#include <isccc/util.h>

isc_result_t
isccc_base64_encode(isccc_region_t *source, int wordlength,
		  const char *wordbreak, isccc_region_t *target)
{
	isc_region_t sr;
	isc_buffer_t tb;
	isc_result_t result;

	sr.base = source->rstart;
	sr.length = source->rend - source->rstart;
	isc_buffer_init(&tb, target->rstart, target->rend - target->rstart);

	result = isc_base64_totext(&sr, wordlength, wordbreak, &tb);
	if (result != ISC_R_SUCCESS)
		return (result);
	source->rstart = source->rend;
	target->rstart = isc_buffer_used(&tb);
	return (ISC_R_SUCCESS);
}

isc_result_t
isccc_base64_decode(const char *cstr, isccc_region_t *target) {
	isc_buffer_t b;
	isc_result_t result;

	isc_buffer_init(&b, target->rstart, target->rend - target->rstart);
	result = isc_base64_decodestring(cstr, &b);
	if (result != ISC_R_SUCCESS)
		return (result);
	target->rstart = isc_buffer_used(&b);
	return (ISC_R_SUCCESS);
}