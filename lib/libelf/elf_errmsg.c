
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

#include <libelf.h>
#include <string.h>

#include "_libelf.h"

/*
 * Retrieve a human readable translation for an error message.
 */

static const char *_libelf_errors[] = {
#define	DEFINE_ERROR(N,S)	[ELF_E_##N] = S
	DEFINE_ERROR(NONE,	"No Error"),
	DEFINE_ERROR(ARCHIVE,	"Malformed ar(1) archive"),
	DEFINE_ERROR(ARGUMENT,	"Invalid argument"),
	DEFINE_ERROR(CLASS,	"ELF class mismatch"),
	DEFINE_ERROR(DATA,	"Invalid data buffer descriptor"),
	DEFINE_ERROR(HEADER,	"Missing or malformed ELF header"),
	DEFINE_ERROR(IO,	"I/O error"),
	DEFINE_ERROR(LAYOUT,	"Layout constraint violation"),
	DEFINE_ERROR(MODE,	"Incorrect ELF descriptor mode"),
	DEFINE_ERROR(RANGE,	"Value out of range of target"),
	DEFINE_ERROR(RESOURCE,	"Resource exhaustion"),
	DEFINE_ERROR(SECTION,	"Invalid section descriptor"),
	DEFINE_ERROR(SEQUENCE,	"API calls out of sequence"),
	DEFINE_ERROR(UNIMPL,	"Unimplemented feature"),
	DEFINE_ERROR(VERSION,	"Unknown ELF API version"),
	DEFINE_ERROR(NUM,	"Unknown error")
#undef	DEFINE_ERROR
};

const char *
elf_errmsg(int error)
{
	int oserr;

	if (error == 0 && (error = LIBELF_PRIVATE(error)) == 0)
	    return NULL;
	else if (error == -1)
	    error = LIBELF_PRIVATE(error);

	oserr = error >> LIBELF_OS_ERROR_SHIFT;
	error &= LIBELF_ELF_ERROR_MASK;

	if (error < 0 || error >= ELF_E_NUM)
		return _libelf_errors[ELF_E_NUM];
	if (oserr) {
		strlcpy(LIBELF_PRIVATE(msg), _libelf_errors[error],
		    sizeof(LIBELF_PRIVATE(msg)));
		strlcat(LIBELF_PRIVATE(msg), ": ", sizeof(LIBELF_PRIVATE(msg)));
		strlcat(LIBELF_PRIVATE(msg), strerror(oserr),
		    sizeof(LIBELF_PRIVATE(msg)));
		return (const char *)&LIBELF_PRIVATE(msg);
	}
	return _libelf_errors[error];
}

#if	defined(LIBELF_TEST_HOOKS)

const char *
_libelf_get_unknown_error_message(void)
{
	return _libelf_errors[ELF_E_NUM];
}

const char *
_libelf_get_no_error_message(void)
{
	return _libelf_errors[0];
}

#endif	/* LIBELF_TEST_HOOKS */