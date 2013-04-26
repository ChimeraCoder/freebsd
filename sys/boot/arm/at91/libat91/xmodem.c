
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

#include "lib.h"

#define PACKET_SIZE	128

/* Line control codes */
#define SOH			0x01	/* start of header */
#define ACK			0x06	/* Acknowledge */
#define NAK			0x15	/* Negative acknowledge */
#define CAN			0x18	/* Cancel */
#define EOT			0x04	/* end of text */

#define TO	10
/*
 * int GetRecord(char , char *)
 *  This private function receives a x-modem record to the pointer and
 * returns non-zero on success.
 */
static int
GetRecord(char blocknum, char *dest)
{
	int		size;
	int		ch;
	unsigned	chk, j;

	chk = 0;

	if ((ch = getc(TO)) == -1)
		goto err;
	if (ch != blocknum) 
		goto err;
	if ((ch = getc(TO)) == -1) 
		goto err;
	if (ch != (~blocknum & 0xff))
		goto err;
	
	for (size = 0; size < PACKET_SIZE; ++size) {
		if ((ch = getc(TO)) == -1)
			goto err;
		chk = chk ^ ch << 8;
		for (j = 0; j < 8; ++j) {
			if (chk & 0x8000)
				chk = chk << 1 ^ 0x1021;
			else
				chk = chk << 1;
		}
		*dest++ = ch;
	}

	chk &= 0xFFFF;

	if (((ch = getc(TO)) == -1) || ((ch & 0xff) != ((chk >> 8) & 0xFF)))
		goto err;
	if (((ch = getc(TO)) == -1) || ((ch & 0xff) != (chk & 0xFF)))
		goto err;
	putchar(ACK);

	return (1);
err:;
	putchar(CAN);
	// We should allow for resend, but we don't.
	return (0);
}

/*
 * int xmodem_rx(char *)
 *  This global function receives a x-modem transmission consisting of
 * (potentially) several blocks.  Returns the number of bytes received or
 * -1 on error.
 */
int
xmodem_rx(char *dest)
{
	int		starting, ch;
	char		packetNumber, *startAddress = dest;

	packetNumber = 1;
	starting = 1;

	while (1) {
		if (starting)
			putchar('C');
		if (((ch = getc(1)) == -1) || (ch != SOH && ch != EOT))
			continue;
		if (ch == EOT) {
			putchar(ACK);
			return (dest - startAddress);
		}
		starting = 0;
		// Xmodem packets: SOH PKT# ~PKT# 128-bytes CRC16
		if (!GetRecord(packetNumber, dest))
			return (-1);
		dest += PACKET_SIZE;
		packetNumber++;
	}

	// the loop above should return in all cases
	return (-1);
}