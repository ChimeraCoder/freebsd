
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

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <security/pam_appl.h>
#include <security/openpam.h>
#include <security/pam_mod_misc.h>

/* Print a verbose error, including the function name and a
 * cleaned up filename.
 */
void
_pam_verbose_error(pam_handle_t *pamh, int flags,
    const char *file, const char *function, const char *format, ...)
{
	va_list ap;
	char *fmtbuf, *modname, *period;

	if (!(flags & PAM_SILENT) && !openpam_get_option(pamh, "no_warn")) {
		modname = basename(file);
		period = strchr(modname, '.');
		if (period == NULL)
			period = strchr(modname, '\0');
		va_start(ap, format);
		asprintf(&fmtbuf, "%.*s: %s: %s\n", (int)(period - modname),
		    modname, function, format);
		pam_verror(pamh, fmtbuf, ap);
		free(fmtbuf);
		va_end(ap);
	}
}