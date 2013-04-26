
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
 * 6.6 : Stream I/O
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <contrib/dev/acpica/include/acpi.h>

void
AcpiOsPrintf(const char *Format, ...)
{
    va_list	ap;

    va_start(ap, Format);
    vprintf(Format, ap);
    va_end(ap);
}

void
AcpiOsVprintf(const char *Format, va_list Args)
{
    vprintf(Format, Args);
}