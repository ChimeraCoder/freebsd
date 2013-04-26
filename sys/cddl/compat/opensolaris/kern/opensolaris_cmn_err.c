
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

#include <sys/assfail.h>
#include <sys/cmn_err.h>

void
vcmn_err(int ce, const char *fmt, va_list adx)
{
	char buf[256];
	const char *prefix;

	prefix = NULL; /* silence unwitty compilers */
	switch (ce) {
	case CE_CONT:
		prefix = "Solaris(cont): ";
		break;
	case CE_NOTE:
		prefix = "Solaris: NOTICE: ";
		break;
	case CE_WARN:
		prefix = "Solaris: WARNING: ";
		break;
	case CE_PANIC:
		prefix = "Solaris(panic): ";
		break;
	case CE_IGNORE:
		break;
	default:
		panic("Solaris: unknown severity level");
	}
	if (ce == CE_PANIC) {
		vsnprintf(buf, sizeof(buf), fmt, adx);
		panic("%s%s", prefix, buf);
	}
	if (ce != CE_IGNORE) {
		printf("%s", prefix);
		vprintf(fmt, adx);
		printf("\n");
	}
}

void
cmn_err(int type, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vcmn_err(type, fmt, ap);
	va_end(ap);
}

int
assfail(const char *a, const char *f, int l) {

	panic("solaris assert: %s, file: %s, line: %d", a, f, l);

	return (0);
}

void
assfail3(const char *a, uintmax_t lv, const char *op, uintmax_t rv,
    const char *f, int l) {

	panic("solaris assert: %s (0x%jx %s 0x%jx), file: %s, line: %d",
	    a, lv, op, rv, f, l);
}