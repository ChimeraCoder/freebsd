
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

#include <efi.h>
#include <efilib.h>

#include "bootstrap.h"

static SIMPLE_TEXT_OUTPUT_INTERFACE	*conout;
static SIMPLE_INPUT_INTERFACE		*conin;

static void
efi_cons_probe(struct console *cp)
{
	conout = ST->ConOut;
	conin = ST->ConIn;
	cp->c_flags |= C_PRESENTIN | C_PRESENTOUT;
}

static int
efi_cons_init(int arg)
{
	conout->SetAttribute(conout, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
	return 0;
}

void
efi_cons_putchar(int c)
{
	CHAR16 buf[2];

	if (c == '\n')
		efi_cons_putchar('\r');

	buf[0] = c;
	buf[1] = 0;

	conout->OutputString(conout, buf);
}

int
efi_cons_getchar()
{
	EFI_INPUT_KEY key;
	EFI_STATUS status;
	UINTN junk;

	/* Try to read a key stroke. We wait for one if none is pending. */
	status = conin->ReadKeyStroke(conin, &key);
	if (status == EFI_NOT_READY) {
		BS->WaitForEvent(1, &conin->WaitForKey, &junk);
		status = conin->ReadKeyStroke(conin, &key);
	}
	return (key.UnicodeChar);
}

int
efi_cons_poll()
{
	/* This can clear the signaled state. */
	return (BS->CheckEvent(conin->WaitForKey) == EFI_SUCCESS);
}

struct console efi_console = {
	"efi",
	"EFI console",
	0,
	efi_cons_probe,
	efi_cons_init,
	efi_cons_putchar,
	efi_cons_getchar,
	efi_cons_poll
};