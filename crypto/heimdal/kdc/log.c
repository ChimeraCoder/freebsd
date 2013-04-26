
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

#include "kdc_locl.h"

void
kdc_openlog(krb5_context context,
	    const char *service,
	    krb5_kdc_configuration *config)
{
    char **s = NULL, **p;
    krb5_initlog(context, "kdc", &config->logf);
    s = krb5_config_get_strings(context, NULL, service, "logging", NULL);
    if(s == NULL)
	s = krb5_config_get_strings(context, NULL, "logging", service, NULL);
    if(s){
	for(p = s; *p; p++)
	    krb5_addlog_dest(context, config->logf, *p);
	krb5_config_free_strings(s);
    }else {
	char *ss;
	if (asprintf(&ss, "0-1/FILE:%s/%s", hdb_db_dir(context),
	    KDC_LOG_FILE) < 0)
	    err(1, NULL);
	krb5_addlog_dest(context, config->logf, ss);
	free(ss);
    }
    krb5_set_warn_dest(context, config->logf);
}

char*
kdc_log_msg_va(krb5_context context,
	       krb5_kdc_configuration *config,
	       int level, const char *fmt, va_list ap)
{
    char *msg;
    krb5_vlog_msg(context, config->logf, &msg, level, fmt, ap);
    return msg;
}

char*
kdc_log_msg(krb5_context context,
	    krb5_kdc_configuration *config,
	    int level, const char *fmt, ...)
{
    va_list ap;
    char *s;
    va_start(ap, fmt);
    s = kdc_log_msg_va(context, config, level, fmt, ap);
    va_end(ap);
    return s;
}

void
kdc_log(krb5_context context,
	krb5_kdc_configuration *config,
	int level, const char *fmt, ...)
{
    va_list ap;
    char *s;
    va_start(ap, fmt);
    s = kdc_log_msg_va(context, config, level, fmt, ap);
    if(s) free(s);
    va_end(ap);
}