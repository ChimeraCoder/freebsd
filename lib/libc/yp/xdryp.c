
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

#include <rpc/rpc.h>
#include <rpcsvc/yp.h>
#include <stdlib.h>
#include <string.h>

extern int (*ypresp_allfn)();
extern void *ypresp_data;

/*
 * I'm leaving the xdr_datum() function in purely for backwards
 * compatibility. yplib.c doesn't actually use it, but it's listed
 * in yp_prot.h as being available, so it's probably a good idea to
 * leave it in case somebody goes looking for it.
 */
typedef struct {
	char *dptr;
	int  dsize;
} datum;

bool_t
xdr_datum(XDR *xdrs, datum *objp)
{
	if (!xdr_bytes(xdrs, (char **)&objp->dptr, (u_int *)&objp->dsize, YPMAXRECORD)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_ypresp_all_seq(XDR *xdrs, u_long *objp)
{
	struct ypresp_all out;
	u_long status;
	char *key, *val;
	int r;

	bzero(&out, sizeof out);
	while (1) {
		if (!xdr_ypresp_all(xdrs, &out)) {
			xdr_free((xdrproc_t)xdr_ypresp_all, &out);
			*objp = YP_YPERR;
			return (FALSE);
		}
		if (out.more == 0) {
			xdr_free((xdrproc_t)xdr_ypresp_all, &out);
			*objp = YP_NOMORE;
			return (TRUE);
		}
		status = out.ypresp_all_u.val.stat;
		switch (status) {
		case YP_TRUE:
			key = (char *)malloc(out.ypresp_all_u.val.key.keydat_len + 1);
			if (key == NULL) {
				xdr_free((xdrproc_t)xdr_ypresp_all, &out);
				*objp = YP_YPERR;
				return (FALSE);
			}
			bcopy(out.ypresp_all_u.val.key.keydat_val, key,
				out.ypresp_all_u.val.key.keydat_len);
			key[out.ypresp_all_u.val.key.keydat_len] = '\0';
			val = (char *)malloc(out.ypresp_all_u.val.val.valdat_len + 1);
			if (val == NULL) {
				free(key);
				xdr_free((xdrproc_t)xdr_ypresp_all, &out);
				*objp = YP_YPERR;
				return (FALSE);
			}
			bcopy(out.ypresp_all_u.val.val.valdat_val, val,
				out.ypresp_all_u.val.val.valdat_len);
			val[out.ypresp_all_u.val.val.valdat_len] = '\0';
			xdr_free((xdrproc_t)xdr_ypresp_all, &out);

			r = (*ypresp_allfn)(status,
				key, out.ypresp_all_u.val.key.keydat_len,
				val, out.ypresp_all_u.val.val.valdat_len,
				ypresp_data);
			*objp = status;
			free(key);
			free(val);
			if (r)
				return (TRUE);
			break;
		case YP_NOMORE:
			xdr_free((xdrproc_t)xdr_ypresp_all, &out);
			*objp = YP_NOMORE;
			return (TRUE);
		default:
			xdr_free((xdrproc_t)xdr_ypresp_all, &out);
			*objp = status;
			return (TRUE);
		}
	}
}