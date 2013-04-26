
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

/* $Id: parseint.c,v 1.8 2007/06/19 23:47:17 tbox Exp $ */

/*! \file */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include <isc/parseint.h>
#include <isc/result.h>
#include <isc/stdlib.h>

isc_result_t
isc_parse_uint32(isc_uint32_t *uip, const char *string, int base) {
	unsigned long n;
	char *e;
	if (! isalnum((unsigned char)(string[0])))
		return (ISC_R_BADNUMBER);
	errno = 0;
	n = strtoul(string, &e, base);
	if (*e != '\0')
		return (ISC_R_BADNUMBER);
	if (n == ULONG_MAX && errno == ERANGE)
		return (ISC_R_RANGE);
	*uip = n;
	return (ISC_R_SUCCESS);
}

isc_result_t
isc_parse_uint16(isc_uint16_t *uip, const char *string, int base) {
	isc_uint32_t val;
	isc_result_t result;
	result = isc_parse_uint32(&val, string, base);
	if (result != ISC_R_SUCCESS)
		return (result);
	if (val > 0xFFFF)
		return (ISC_R_RANGE);
	*uip = (isc_uint16_t) val;
	return (ISC_R_SUCCESS);
}

isc_result_t
isc_parse_uint8(isc_uint8_t *uip, const char *string, int base) {
	isc_uint32_t val;
	isc_result_t result;
	result = isc_parse_uint32(&val, string, base);
	if (result != ISC_R_SUCCESS)
		return (result);
	if (val > 0xFF)
		return (ISC_R_RANGE);
	*uip = (isc_uint8_t) val;
	return (ISC_R_SUCCESS);
}