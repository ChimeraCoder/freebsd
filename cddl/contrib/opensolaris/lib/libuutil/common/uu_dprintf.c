
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include "libuutil_common.h"

#include <errno.h>
#include <libintl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	FACILITY_FMT	"%s (%s): "

#if !defined(TEXT_DOMAIN)
#define	TEXT_DOMAIN "SYS_TEST"
#endif

static const char *
strseverity(uu_dprintf_severity_t severity)
{
	switch (severity) {
	case UU_DPRINTF_SILENT:
		return (dgettext(TEXT_DOMAIN, "silent"));
	case UU_DPRINTF_FATAL:
		return (dgettext(TEXT_DOMAIN, "FATAL"));
	case UU_DPRINTF_WARNING:
		return (dgettext(TEXT_DOMAIN, "WARNING"));
	case UU_DPRINTF_NOTICE:
		return (dgettext(TEXT_DOMAIN, "note"));
	case UU_DPRINTF_INFO:
		return (dgettext(TEXT_DOMAIN, "info"));
	case UU_DPRINTF_DEBUG:
		return (dgettext(TEXT_DOMAIN, "debug"));
	default:
		return (dgettext(TEXT_DOMAIN, "unspecified"));
	}
}

uu_dprintf_t *
uu_dprintf_create(const char *name, uu_dprintf_severity_t severity,
    uint_t flags)
{
	uu_dprintf_t *D;

	if (uu_check_name(name, UU_NAME_DOMAIN) == -1) {
		uu_set_error(UU_ERROR_INVALID_ARGUMENT);
		return (NULL);
	}

	if ((D = uu_zalloc(sizeof (uu_dprintf_t))) == NULL)
		return (NULL);

	if (name != NULL) {
		D->uud_name = strdup(name);
		if (D->uud_name == NULL) {
			uu_free(D);
			return (NULL);
		}
	} else {
		D->uud_name = NULL;
	}

	D->uud_severity = severity;
	D->uud_flags = flags;

	return (D);
}

/*PRINTFLIKE3*/
void
uu_dprintf(uu_dprintf_t *D, uu_dprintf_severity_t severity,
    const char *format, ...)
{
	va_list alist;

	/* XXX Assert that severity is not UU_DPRINTF_SILENT. */

	if (severity > D->uud_severity)
		return;

	(void) fprintf(stderr, FACILITY_FMT, D->uud_name,
	    strseverity(severity));

	va_start(alist, format);
	(void) vfprintf(stderr, format, alist);
	va_end(alist);
}

void
uu_dprintf_destroy(uu_dprintf_t *D)
{
	if (D->uud_name)
		free(D->uud_name);

	uu_free(D);
}

const char *
uu_dprintf_getname(uu_dprintf_t *D)
{
	return (D->uud_name);
}