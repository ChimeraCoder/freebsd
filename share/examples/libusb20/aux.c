
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

/*
 * Helper functions common to all examples
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <libusb20.h>
#include <libusb20_desc.h>

#include "aux.h"

/*
 * Return a textual description for error "r".
 */
const char *
usb_error(enum libusb20_error r)
{
  const char *msg = "UNKNOWN";

  switch (r)
    {
    case LIBUSB20_SUCCESS:
      msg = "success";
      break;

    case LIBUSB20_ERROR_IO:
      msg = "IO error";
      break;

    case LIBUSB20_ERROR_INVALID_PARAM:
      msg = "Invalid parameter";
      break;

    case LIBUSB20_ERROR_ACCESS:
      msg = "Access denied";
      break;

    case LIBUSB20_ERROR_NO_DEVICE:
      msg = "No such device";
      break;

    case LIBUSB20_ERROR_NOT_FOUND:
      msg = "Entity not found";
      break;

    case LIBUSB20_ERROR_BUSY:
      msg = "Resource busy";
      break;

    case LIBUSB20_ERROR_TIMEOUT:
      msg = "Operation timed out";
      break;

    case LIBUSB20_ERROR_OVERFLOW:
      msg = "Overflow";
      break;

    case LIBUSB20_ERROR_PIPE:
      msg = "Pipe error";
      break;

    case LIBUSB20_ERROR_INTERRUPTED:
      msg = "System call interrupted";
      break;

    case LIBUSB20_ERROR_NO_MEM:
      msg = "Insufficient memory";
      break;

    case LIBUSB20_ERROR_NOT_SUPPORTED:
      msg = "Operation not supported";
      break;

    case LIBUSB20_ERROR_OTHER:
      msg = "Other error";
      break;
    }

  return msg;
}

/*
 * Print "len" bytes from "buf" in hex, followed by an ASCII
 * representation (somewhat resembling the output of hd(1)).
 */
void
print_formatted(uint8_t *buf, uint32_t len)
{
  int i, j;

  for (j = 0; j < len; j += 16)
    {
      printf("%02x: ", j);

      for (i = 0; i < 16 && i + j < len; i++)
	printf("%02x ", buf[i + j]);
      printf("  ");
      for (i = 0; i < 16 && i + j < len; i++)
	{
	  uint8_t c = buf[i + j];
	  if(c >= ' ' && c <= '~')
	    printf("%c", (char)c);
	  else
	    putchar('.');
	}
      putchar('\n');
    }
}