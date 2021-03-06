
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

/* $Id: gai_strerror.c,v 1.22 2007/06/19 23:47:22 tbox Exp $ */

/*! \file gai_strerror.c
 * lwres_gai_strerror() returns an error message corresponding to an
 * error code returned by getaddrinfo(). The following error codes and
 * their meaning are defined in \link netdb.h include/lwres/netdb.h.\endlink
 *
 * \li #EAI_ADDRFAMILY address family for hostname not supported
 * \li #EAI_AGAIN temporary failure in name resolution
 * \li #EAI_BADFLAGS invalid value for #ai_flags
 * \li #EAI_FAIL non-recoverable failure in name resolution
 * \li #EAI_FAMILY ai_family not supported
 * \li #EAI_MEMORY memory allocation failure
 * \li #EAI_NODATA no address associated with hostname
 * \li #EAI_NONAME hostname or servname not provided, or not known
 * \li #EAI_SERVICE servname not supported for ai_socktype
 * \li #EAI_SOCKTYPE ai_socktype not supported
 * \li #EAI_SYSTEM system error returned in errno
 *
 * The message invalid error code is returned if ecode is out of range.
 *
 * ai_flags, ai_family and ai_socktype are elements of the struct
 * addrinfo used by lwres_getaddrinfo().
 *
 * \section gai_strerror_see See Also
 *
 * strerror, lwres_getaddrinfo(), getaddrinfo(), RFC2133.
 */

#include <config.h>

#include <lwres/netdb.h>

/*% Text of error messages. */
static const char *gai_messages[] = {
	"no error",
	"address family for hostname not supported",
	"temporary failure in name resolution",
	"invalid value for ai_flags",
	"non-recoverable failure in name resolution",
	"ai_family not supported",
	"memory allocation failure",
	"no address associated with hostname",
	"hostname nor servname provided, or not known",
	"servname not supported for ai_socktype",
	"ai_socktype not supported",
	"system error returned in errno",
	"bad hints",
	"bad protocol"
};

/*% Returns an error message corresponding to an error code returned by getaddrinfo() */
char *
lwres_gai_strerror(int ecode) {
	union {
		const char *const_ptr;
		char *deconst_ptr;
	} ptr;

	if ((ecode < 0) ||
	    (ecode >= (int)(sizeof(gai_messages)/sizeof(*gai_messages))))
		ptr.const_ptr = "invalid error code";
	else
		ptr.const_ptr = gai_messages[ecode];
	return (ptr.deconst_ptr);
}