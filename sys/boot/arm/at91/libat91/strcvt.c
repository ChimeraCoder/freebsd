
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

/******************************************************************************
 *
 * Filename: p_string.c
 *
 * Instantiation of basic string operations to prevent inclusion of full
 * string library.  These are simple implementations not necessarily optimized
 * for speed, but rather to show intent.
 *
 * Revision information:
 *
 * 20AUG2004	kb_admin	initial creation
 * 12JAN2005	kb_admin	minor updates
 *
 * BEGIN_KBDD_BLOCK
 * No warranty, expressed or implied, is included with this software.  It is
 * provided "AS IS" and no warranty of any kind including statutory or aspects
 * relating to merchantability or fitness for any purpose is provided.  All
 * intellectual property rights of others is maintained with the respective
 * owners.  This software is not copyrighted and is intended for reference
 * only.
 * END_BLOCK
 *****************************************************************************/

#include "lib.h"

/*
 * .KB_C_FN_DEFINITION_START
 * int p_IsWhiteSpace(char)
 *  This global function returns true if the character is not considered
 * a non-space character.
 * .KB_C_FN_DEFINITION_END
 */
int
p_IsWhiteSpace(char cValue)
{
	return ((cValue == ' ') ||
		(cValue == '\t') ||
		(cValue == 0) ||
		(cValue == '\r') ||
		(cValue == '\n'));
}


/*
 * .KB_C_FN_DEFINITION_START
 * unsigned p_HexCharValue(char)
 *  This global function returns the decimal value of the validated hex char.
 * .KB_C_FN_DEFINITION_END
 */
unsigned
p_HexCharValue(char cValue)
{
	if (cValue < ('9' + 1))
		return (cValue - '0');
	if (cValue < ('F' + 1))
		return (cValue - 'A' + 10);
	return (cValue - 'a' + 10);
}

/*
 * .KB_C_FN_DEFINITION_START
 * unsigned p_ASCIIToHex(char *)
 *  This global function set the unsigned value equal to the converted
 * hex number passed as a string.  No error checking is performed; the
 * string must be valid hex value, point at the start of string, and be
 * NULL-terminated.
 * .KB_C_FN_DEFINITION_END
 */
unsigned
p_ASCIIToHex(const char *buf)
{
	unsigned	lValue = 0;

	if ((*buf == '0') && ((buf[1] == 'x') || (buf[1] == 'X')))
		buf += 2;

	while (*buf) {
		lValue <<= 4;
		lValue += p_HexCharValue(*buf++);
	}
	return (lValue);
}


/*
 * .KB_C_FN_DEFINITION_START
 * unsigned p_ASCIIToDec(char *)
 *  This global function set the unsigned value equal to the converted
 * decimal number passed as a string.  No error checking is performed; the
 * string must be valid decimal value, point at the start of string, and be
 * NULL-terminated.
 * .KB_C_FN_DEFINITION_END
 */
unsigned
p_ASCIIToDec(const char *buf)
{
	unsigned v = 0;

	while (*buf) {
		v *= 10;
		v += (*buf++) - '0';
	}
	return (v);
}