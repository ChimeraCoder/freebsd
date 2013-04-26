
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
 * 6.8 : Debugging support
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_ddb.h"
#include <sys/param.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <ddb/ddb.h>
#include <ddb/db_output.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>
#include <contrib/dev/acpica/include/acdebug.h>

#include <dev/acpica/acpivar.h>

ACPI_STATUS
AcpiOsGetLine(char *Buffer, UINT32 BufferLength, UINT32 *BytesRead)
{
#ifdef DDB
	char *cp;

	cp = Buffer;
	if (db_readline(Buffer, BufferLength) > 0)
		while (*cp != '\0' && *cp != '\n' && *cp != '\r')
			cp++;
	*cp = '\0';
	if (BytesRead != NULL)
		*BytesRead = cp - Buffer;
	return (AE_OK);
#else
	printf("AcpiOsGetLine called but no input support");
	return (AE_NOT_EXIST);
#endif /* DDB */
}

ACPI_STATUS
AcpiOsSignal(UINT32 Function, void *Info)
{
    ACPI_SIGNAL_FATAL_INFO	*fatal;

    switch (Function) {
    case ACPI_SIGNAL_FATAL:
	fatal = (ACPI_SIGNAL_FATAL_INFO *)Info;
	printf("ACPI fatal signal, type 0x%x code 0x%x argument 0x%x",
	      fatal->Type, fatal->Code, fatal->Argument);
#ifdef ACPI_DEBUG
	kdb_enter(KDB_WHY_ACPI, "AcpiOsSignal");
#endif
	break;

    case ACPI_SIGNAL_BREAKPOINT:
#ifdef ACPI_DEBUG
	kdb_enter(KDB_WHY_ACPI, (char *)Info);
#endif
	break;

    default:
	return (AE_BAD_PARAMETER);
    }

    return (AE_OK);
}

#ifdef ACPI_DEBUGGER
void
acpi_EnterDebugger(void)
{
    ACPI_PARSE_OBJECT	obj;
    static int		initted = 0;

    if (!initted) {
	printf("Initialising ACPICA debugger...\n");
	AcpiDbInitialize();
	initted = 1;
    }

    printf("Entering ACPICA debugger...\n");
    AcpiDbUserCommands('A', &obj);
}
#endif /* ACPI_DEBUGGER */