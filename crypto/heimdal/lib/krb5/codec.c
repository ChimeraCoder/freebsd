
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

#ifndef HEIMDAL_SMALLER

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_decode_EncTicketPart (krb5_context context,
			   const void *data,
			   size_t length,
			   EncTicketPart *t,
			   size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return decode_EncTicketPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_encode_EncTicketPart (krb5_context context,
			   void *data,
			   size_t length,
			   EncTicketPart *t,
			   size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return encode_EncTicketPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_decode_EncASRepPart (krb5_context context,
			  const void *data,
			  size_t length,
			  EncASRepPart *t,
			  size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return decode_EncASRepPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_encode_EncASRepPart (krb5_context context,
			  void *data,
			  size_t length,
			  EncASRepPart *t,
			  size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return encode_EncASRepPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_decode_EncTGSRepPart (krb5_context context,
			   const void *data,
			   size_t length,
			   EncTGSRepPart *t,
			   size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return decode_EncTGSRepPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_encode_EncTGSRepPart (krb5_context context,
			   void *data,
			   size_t length,
			   EncTGSRepPart *t,
			   size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return encode_EncTGSRepPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_decode_EncAPRepPart (krb5_context context,
			  const void *data,
			  size_t length,
			  EncAPRepPart *t,
			  size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return decode_EncAPRepPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_encode_EncAPRepPart (krb5_context context,
			  void *data,
			  size_t length,
			  EncAPRepPart *t,
			  size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return encode_EncAPRepPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_decode_Authenticator (krb5_context context,
			   const void *data,
			   size_t length,
			   Authenticator *t,
			   size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return decode_Authenticator(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_encode_Authenticator (krb5_context context,
			   void *data,
			   size_t length,
			   Authenticator *t,
			   size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return encode_Authenticator(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_decode_EncKrbCredPart (krb5_context context,
			    const void *data,
			    size_t length,
			    EncKrbCredPart *t,
			    size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return decode_EncKrbCredPart(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_encode_EncKrbCredPart (krb5_context context,
			    void *data,
			    size_t length,
			    EncKrbCredPart *t,
			    size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return encode_EncKrbCredPart (data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_decode_ETYPE_INFO (krb5_context context,
			const void *data,
			size_t length,
			ETYPE_INFO *t,
			size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return decode_ETYPE_INFO(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_encode_ETYPE_INFO (krb5_context context,
			void *data,
			size_t length,
			ETYPE_INFO *t,
			size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return encode_ETYPE_INFO (data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_decode_ETYPE_INFO2 (krb5_context context,
			const void *data,
			size_t length,
			ETYPE_INFO2 *t,
			size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return decode_ETYPE_INFO2(data, length, t, len);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_encode_ETYPE_INFO2 (krb5_context context,
			 void *data,
			 size_t length,
			 ETYPE_INFO2 *t,
			 size_t *len)
    KRB5_DEPRECATED_FUNCTION("Use X instead")
{
    return encode_ETYPE_INFO2 (data, length, t, len);
}

#endif /* HEIMDAL_SMALLER */