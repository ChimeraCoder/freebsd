
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
 * Convert the getaddrinfo() error code to a Kerberos et error code.
 *
 * @param eai_errno contains the error code from getaddrinfo().
 * @param system_error should have the value of errno after the failed getaddrinfo().
 *
 * @return Kerberos error code representing the EAI errors.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_eai_to_heim_errno(int eai_errno, int system_error)
{
    switch(eai_errno) {
    case EAI_NOERROR:
	return 0;
#ifdef EAI_ADDRFAMILY
    case EAI_ADDRFAMILY:
	return HEIM_EAI_ADDRFAMILY;
#endif
    case EAI_AGAIN:
	return HEIM_EAI_AGAIN;
    case EAI_BADFLAGS:
	return HEIM_EAI_BADFLAGS;
    case EAI_FAIL:
	return HEIM_EAI_FAIL;
    case EAI_FAMILY:
	return HEIM_EAI_FAMILY;
    case EAI_MEMORY:
	return HEIM_EAI_MEMORY;
#if defined(EAI_NODATA) && EAI_NODATA != EAI_NONAME
    case EAI_NODATA:
	return HEIM_EAI_NODATA;
#endif
#ifdef WSANO_DATA
    case WSANO_DATA:
	return HEIM_EAI_NODATA;
#endif
    case EAI_NONAME:
	return HEIM_EAI_NONAME;
    case EAI_SERVICE:
	return HEIM_EAI_SERVICE;
    case EAI_SOCKTYPE:
	return HEIM_EAI_SOCKTYPE;
#ifdef EAI_SYSTEM
    case EAI_SYSTEM:
	return system_error;
#endif
    default:
	return HEIM_EAI_UNKNOWN; /* XXX */
    }
}

/**
 * Convert the gethostname() error code (h_error) to a Kerberos et
 * error code.
 *
 * @param eai_errno contains the error code from gethostname().
 *
 * @return Kerberos error code representing the gethostname errors.
 *
 * @ingroup krb5_error
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_h_errno_to_heim_errno(int eai_errno)
{
    switch(eai_errno) {
    case 0:
	return 0;
    case HOST_NOT_FOUND:
	return HEIM_EAI_NONAME;
    case TRY_AGAIN:
	return HEIM_EAI_AGAIN;
    case NO_RECOVERY:
	return HEIM_EAI_FAIL;
    case NO_DATA:
	return HEIM_EAI_NONAME;
    default:
	return HEIM_EAI_UNKNOWN; /* XXX */
    }
}