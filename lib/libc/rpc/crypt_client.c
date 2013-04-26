
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <err.h>
#include <sys/types.h>
#include <rpc/des_crypt.h>
#include <rpc/des.h>
#include <string.h>
#include <rpcsvc/crypt.h>
#include "un-namespace.h"

int
_des_crypt_call(buf, len, dparms)
	char *buf;
	int len;
	struct desparams *dparms;
{
	CLIENT *clnt;
	desresp  *result_1;
	desargs  des_crypt_1_arg;
	struct netconfig *nconf;
	void *localhandle;
	int stat;

	nconf = NULL;
	localhandle = setnetconfig();
	while ((nconf = getnetconfig(localhandle)) != NULL) {
		if (nconf->nc_protofmly != NULL &&
		     strcmp(nconf->nc_protofmly, NC_LOOPBACK) == 0)
			break;
	}
	if (nconf == NULL) {
		warnx("getnetconfig: %s", nc_sperror());
		return(DESERR_HWERROR);
	}
	clnt = clnt_tp_create(NULL, CRYPT_PROG, CRYPT_VERS, nconf);
	if (clnt == (CLIENT *) NULL) {
		endnetconfig(localhandle);
		return(DESERR_HWERROR);
	}
	endnetconfig(localhandle);

	des_crypt_1_arg.desbuf.desbuf_len = len;
	des_crypt_1_arg.desbuf.desbuf_val = buf;
	des_crypt_1_arg.des_dir = (dparms->des_dir == ENCRYPT) ? ENCRYPT_DES : DECRYPT_DES;
	des_crypt_1_arg.des_mode = (dparms->des_mode == CBC) ? CBC_DES : ECB_DES;
	bcopy(dparms->des_ivec, des_crypt_1_arg.des_ivec, 8);
	bcopy(dparms->des_key, des_crypt_1_arg.des_key, 8);

	result_1 = des_crypt_1(&des_crypt_1_arg, clnt);
	if (result_1 == (desresp *) NULL) {
		clnt_destroy(clnt);
		return(DESERR_HWERROR);
	}

	stat = result_1->stat;

	if (result_1->stat == DESERR_NONE ||
	    result_1->stat == DESERR_NOHWDEVICE) {
		bcopy(result_1->desbuf.desbuf_val, buf, len);
		bcopy(result_1->des_ivec, dparms->des_ivec, 8);
	}

	clnt_freeres(clnt, (xdrproc_t)xdr_desresp, result_1);
	clnt_destroy(clnt);

	return(stat);
}