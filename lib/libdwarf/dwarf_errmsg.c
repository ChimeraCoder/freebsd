
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

#include <stdio.h>
#include "_libdwarf.h"

static const char *_libdwarf_errors[] = {
#define	DEFINE_ERROR(N,S)		[DWARF_E_##N] = S
	DEFINE_ERROR(NONE,		"No Error"),
	DEFINE_ERROR(ERROR,		"An error"),
	DEFINE_ERROR(NO_ENTRY,		"No entry found"),
	DEFINE_ERROR(ARGUMENT,		"Invalid argument"),
	DEFINE_ERROR(DEBUG_INFO,	"Debug info NULL"),
	DEFINE_ERROR(MEMORY,		"Insufficient memory"),
	DEFINE_ERROR(ELF,		"ELF error"),
	DEFINE_ERROR(INVALID_CU,	"Invalid compilation unit data"),
	DEFINE_ERROR(CU_VERSION,	"Wrong CU version. Only 2 and 3 supported"),
	DEFINE_ERROR(MISSING_ABBREV,	"Abbrev not found"),
	DEFINE_ERROR(NOT_IMPLEMENTED,	"Unimplemented code at"),
	DEFINE_ERROR(CU_CURRENT,	"No current compilation unit"),
	DEFINE_ERROR(BAD_FORM,		"Wrong form type for attribute value"),
	DEFINE_ERROR(INVALID_EXPR,	"Invalid DWARF expression"),
	DEFINE_ERROR(NUM,		"Unknown DWARF error")
#undef	DEFINE_ERROR
};

const char *
dwarf_errmsg(Dwarf_Error *error)
{
	const char *p;

	if (error == NULL)
		return NULL;

	if (error->err_error < 0 || error->err_error >= DWARF_E_NUM)
		return _libdwarf_errors[DWARF_E_NUM];
	else if (error->err_error == DWARF_E_NONE)
		return _libdwarf_errors[DWARF_E_NONE];
	else
		p = _libdwarf_errors[error->err_error];

	if (error->err_error == DWARF_E_ELF)
		snprintf(error->err_msg, sizeof(error->err_msg),
		    "ELF error : %s [%s(%d)]", elf_errmsg(error->elf_error),
		    error->err_func, error->err_line);
	else
		snprintf(error->err_msg, sizeof(error->err_msg),
		    "%s [%s(%d)]", p, error->err_func, error->err_line);

	return (const char *) error->err_msg;
}