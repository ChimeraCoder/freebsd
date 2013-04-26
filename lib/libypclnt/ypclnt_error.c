
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ypclnt.h"

void
ypclnt_error(ypclnt_t *ypclnt, const char *func, const char *fmt, ...)
{
	char *errmsg;
	va_list ap;

	free(ypclnt->error);
	ypclnt->error = NULL;
	if (fmt == NULL)
		return;

	va_start(ap, fmt);
	vasprintf(&errmsg, fmt, ap);
	va_end(ap);
	asprintf(&ypclnt->error, "%s(): %s", func, errmsg);
	free(errmsg);
}