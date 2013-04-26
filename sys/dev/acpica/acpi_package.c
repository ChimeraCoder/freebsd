
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/sbuf.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <contrib/dev/acpica/include/acpi.h>

#include <dev/acpica/acpivar.h>

/*
 * Package manipulation convenience functions
 */

int
acpi_PkgInt(ACPI_OBJECT *res, int idx, UINT64 *dst)
{
    ACPI_OBJECT		*obj;

    obj = &res->Package.Elements[idx];
    if (obj == NULL || obj->Type != ACPI_TYPE_INTEGER)
	return (EINVAL);
    *dst = obj->Integer.Value;

    return (0);
}

int
acpi_PkgInt32(ACPI_OBJECT *res, int idx, uint32_t *dst)
{
    UINT64		tmp;
    int			error;

    error = acpi_PkgInt(res, idx, &tmp);
    if (error == 0)
	*dst = (uint32_t)tmp;

    return (error);
}

int
acpi_PkgStr(ACPI_OBJECT *res, int idx, void *dst, size_t size)
{
    ACPI_OBJECT		*obj;
    void		*ptr;
    size_t		 length;

    obj = &res->Package.Elements[idx];
    if (obj == NULL)
	return (EINVAL);
    bzero(dst, sizeof(dst));

    switch (obj->Type) {
    case ACPI_TYPE_STRING:
	ptr = obj->String.Pointer;
	length = obj->String.Length;
	break;
    case ACPI_TYPE_BUFFER:
	ptr = obj->Buffer.Pointer;
	length = obj->Buffer.Length;
	break;
    default:
	return (EINVAL);
    }

    /* Make sure string will fit, including terminating NUL */
    if (++length > size)
	return (E2BIG);

    strlcpy(dst, ptr, length);
    return (0);
}

int
acpi_PkgGas(device_t dev, ACPI_OBJECT *res, int idx, int *type, int *rid,
    struct resource **dst, u_int flags)
{
    ACPI_GENERIC_ADDRESS gas;
    ACPI_OBJECT *obj;

    obj = &res->Package.Elements[idx];
    if (obj == NULL || obj->Type != ACPI_TYPE_BUFFER ||
	obj->Buffer.Length < sizeof(ACPI_GENERIC_ADDRESS) + 3)
	return (EINVAL);

    memcpy(&gas, obj->Buffer.Pointer + 3, sizeof(gas));

    return (acpi_bus_alloc_gas(dev, type, rid, &gas, dst, flags));
}

ACPI_HANDLE
acpi_GetReference(ACPI_HANDLE scope, ACPI_OBJECT *obj)
{
    ACPI_HANDLE h;

    if (obj == NULL)
	return (NULL);

    switch (obj->Type) {
    case ACPI_TYPE_LOCAL_REFERENCE:
    case ACPI_TYPE_ANY:
	h = obj->Reference.Handle;
	break;
    case ACPI_TYPE_STRING:
	/*
	 * The String object usually contains a fully-qualified path, so
	 * scope can be NULL.
	 *
	 * XXX This may not always be the case.
	 */
	if (ACPI_FAILURE(AcpiGetHandle(scope, obj->String.Pointer, &h)))
	    h = NULL;
	break;
    default:
	h = NULL;
	break;
    }

    return (h);
}