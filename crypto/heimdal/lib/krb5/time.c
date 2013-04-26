
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

#include "krb5_locl.h"

/**
 * Set the absolute time that the caller knows the kdc has so the
 * kerberos library can calculate the relative diffrence beteen the
 * KDC time and local system time.
 *
 * @param context Keberos 5 context.
 * @param sec The applications new of "now" in seconds
 * @param usec The applications new of "now" in micro seconds

 * @return Kerberos 5 error code, see krb5_get_error_message().
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_set_real_time (krb5_context context,
		    krb5_timestamp sec,
		    int32_t usec)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    context->kdc_sec_offset = sec - tv.tv_sec;

    /**
     * If the caller passes in a negative usec, its assumed to be
     * unknown and the function will use the current time usec.
     */
    if (usec >= 0) {
	context->kdc_usec_offset = usec - tv.tv_usec;

	if (context->kdc_usec_offset < 0) {
	    context->kdc_sec_offset--;
	    context->kdc_usec_offset += 1000000;
	}
    } else
	context->kdc_usec_offset = tv.tv_usec;

    return 0;
}

/*
 * return ``corrected'' time in `timeret'.
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_timeofday (krb5_context context,
		krb5_timestamp *timeret)
{
    *timeret = time(NULL) + context->kdc_sec_offset;
    return 0;
}

/*
 * like gettimeofday but with time correction to the KDC
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_us_timeofday (krb5_context context,
		   krb5_timestamp *sec,
		   int32_t *usec)
{
    struct timeval tv;

    gettimeofday (&tv, NULL);

    *sec  = tv.tv_sec + context->kdc_sec_offset;
    *usec = tv.tv_usec;		/* XXX */
    return 0;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_format_time(krb5_context context, time_t t,
		 char *s, size_t len, krb5_boolean include_time)
{
    struct tm *tm;
    if(context->log_utc)
	tm = gmtime (&t);
    else
	tm = localtime(&t);
    if(tm == NULL ||
       strftime(s, len, include_time ? context->time_fmt : context->date_fmt, tm) == 0)
	snprintf(s, len, "%ld", (long)t);
    return 0;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_string_to_deltat(const char *string, krb5_deltat *deltat)
{
    if((*deltat = parse_time(string, "s")) == -1)
	return KRB5_DELTAT_BADFORMAT;
    return 0;
}