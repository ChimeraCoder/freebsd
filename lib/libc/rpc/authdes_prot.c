
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

/*
 * authdes_prot.c, XDR routines for DES authentication
 */

#include "namespace.h"
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/auth_des.h>
#include "un-namespace.h"

#define ATTEMPT(xdr_op) if (!(xdr_op)) return (FALSE)

bool_t
xdr_authdes_cred(xdrs, cred)
	XDR *xdrs;
	struct authdes_cred *cred;
{
	enum authdes_namekind *padc_namekind = &cred->adc_namekind;
	/*
	 * Unrolled xdr
	 */
	ATTEMPT(xdr_enum(xdrs, (enum_t *) padc_namekind));
	switch (cred->adc_namekind) {
	case ADN_FULLNAME:
		ATTEMPT(xdr_string(xdrs, &cred->adc_fullname.name,
		    MAXNETNAMELEN));
		ATTEMPT(xdr_opaque(xdrs, (caddr_t)&cred->adc_fullname.key,
		    sizeof(des_block)));
		ATTEMPT(xdr_opaque(xdrs, (caddr_t)&cred->adc_fullname.window,
		    sizeof(cred->adc_fullname.window)));
		return (TRUE);
	case ADN_NICKNAME:
		ATTEMPT(xdr_opaque(xdrs, (caddr_t)&cred->adc_nickname,
		    sizeof(cred->adc_nickname)));
		return (TRUE);
	default:
		return (FALSE);
	}
}


bool_t
xdr_authdes_verf(xdrs, verf)
	XDR *xdrs;
	struct authdes_verf *verf;	
{
	/*
 	 * Unrolled xdr
 	 */
	ATTEMPT(xdr_opaque(xdrs, (caddr_t)&verf->adv_xtimestamp,
	    sizeof(des_block)));
	ATTEMPT(xdr_opaque(xdrs, (caddr_t)&verf->adv_int_u,
	    sizeof(verf->adv_int_u)));
	return (TRUE);
}