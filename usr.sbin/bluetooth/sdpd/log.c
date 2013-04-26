
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

#include <sys/types.h>
#include <stdarg.h>
#include <syslog.h>

void
log_open(char const *prog, int32_t log2stderr)
{
	openlog(prog, LOG_PID|LOG_NDELAY|(log2stderr? LOG_PERROR:0), LOG_USER);
}

void
log_close(void)
{
	closelog();
}

void
log_emerg(char const *message, ...)
{
	va_list	ap;

	va_start(ap, message);
	vsyslog(LOG_EMERG, message, ap);
	va_end(ap);
}

void
log_alert(char const *message, ...)
{
	va_list	ap;

	va_start(ap, message);
	vsyslog(LOG_ALERT, message, ap);
	va_end(ap);
}

void
log_crit(char const *message, ...)
{
	va_list	ap;

	va_start(ap, message);
	vsyslog(LOG_CRIT, message, ap);
	va_end(ap);
}

void
log_err(char const *message, ...)
{
	va_list	ap;

	va_start(ap, message);
	vsyslog(LOG_ERR, message, ap);
	va_end(ap);
}

void
log_warning(char const *message, ...)
{
	va_list	ap;

	va_start(ap, message);
	vsyslog(LOG_WARNING, message, ap);
	va_end(ap);
}

void
log_notice(char const *message, ...)
{
	va_list	ap;

	va_start(ap, message);
	vsyslog(LOG_NOTICE, message, ap);
	va_end(ap);
}

void
log_info(char const *message, ...)
{
	va_list	ap;

	va_start(ap, message);
	vsyslog(LOG_INFO, message, ap);
	va_end(ap);
}

void
log_debug(char const *message, ...)
{
	va_list	ap;

	va_start(ap, message);
	vsyslog(LOG_DEBUG, message, ap);
	va_end(ap);
}