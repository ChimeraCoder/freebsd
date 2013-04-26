
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

#include "kcm_locl.h"

RCSID("$Id$");

static krb5_log_facility *logf;

void
kcm_openlog(void)
{
    char **s = NULL, **p;
    krb5_initlog(kcm_context, "kcm", &logf);
    s = krb5_config_get_strings(kcm_context, NULL, "kcm", "logging", NULL);
    if(s == NULL)
	s = krb5_config_get_strings(kcm_context, NULL, "logging", "kcm", NULL);
    if(s){
	for(p = s; *p; p++)
	    krb5_addlog_dest(kcm_context, logf, *p);
	krb5_config_free_strings(s);
    }else
	krb5_addlog_dest(kcm_context, logf, DEFAULT_LOG_DEST);
    krb5_set_warn_dest(kcm_context, logf);
}

char*
kcm_log_msg_va(int level, const char *fmt, va_list ap)
{
    char *msg;
    krb5_vlog_msg(kcm_context, logf, &msg, level, fmt, ap);
    return msg;
}

char*
kcm_log_msg(int level, const char *fmt, ...)
{
    va_list ap;
    char *s;
    va_start(ap, fmt);
    s = kcm_log_msg_va(level, fmt, ap);
    va_end(ap);
    return s;
}

void
kcm_log(int level, const char *fmt, ...)
{
    va_list ap;
    char *s;
    va_start(ap, fmt);
    s = kcm_log_msg_va(level, fmt, ap);
    if(s) free(s);
    va_end(ap);
}