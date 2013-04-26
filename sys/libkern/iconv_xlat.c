
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/iconv.h>

#include "iconv_converter_if.h"

/*
 * "XLAT" converter
 */

#ifdef MODULE_DEPEND
MODULE_DEPEND(iconv_xlat, libiconv, 2, 2, 2);
#endif

/*
 * XLAT converter instance
 */
struct iconv_xlat {
	KOBJ_FIELDS;
	u_char *		d_table;
	struct iconv_cspair *	d_csp;
};

static int
iconv_xlat_open(struct iconv_converter_class *dcp,
	struct iconv_cspair *csp, struct iconv_cspair *cspf, void **dpp)
{
	struct iconv_xlat *dp;

	dp = (struct iconv_xlat *)kobj_create((struct kobj_class*)dcp, M_ICONV, M_WAITOK);
	dp->d_table = csp->cp_data;
	dp->d_csp = csp;
	csp->cp_refcount++;
	*dpp = (void*)dp;
	return 0;
}

static int
iconv_xlat_close(void *data)
{
	struct iconv_xlat *dp = data;

	dp->d_csp->cp_refcount--;
	kobj_delete((struct kobj*)data, M_ICONV);
	return 0;
}

static int
iconv_xlat_conv(void *d2p, const char **inbuf,
	size_t *inbytesleft, char **outbuf, size_t *outbytesleft,
	int convchar, int casetype)
{
	struct iconv_xlat *dp = (struct iconv_xlat*)d2p;
	const char *src;
	char *dst;
	int n, r;

	if (inbuf == NULL || *inbuf == NULL || outbuf == NULL || *outbuf == NULL)
		return 0;
	if (casetype != 0)
		return -1;
	if (convchar == 1)
		r = n = 1;
	else
		r = n = min(*inbytesleft, *outbytesleft);
	src = *inbuf;
	dst = *outbuf;
	while(r--)
		*dst++ = dp->d_table[(u_char)*src++];
	*inbuf += n;
	*outbuf += n;
	*inbytesleft -= n;
	*outbytesleft -= n;
	return 0;
}

static const char *
iconv_xlat_name(struct iconv_converter_class *dcp)
{
	return "xlat";
}

static kobj_method_t iconv_xlat_methods[] = {
	KOBJMETHOD(iconv_converter_open,	iconv_xlat_open),
	KOBJMETHOD(iconv_converter_close,	iconv_xlat_close),
	KOBJMETHOD(iconv_converter_conv,	iconv_xlat_conv),
#if 0
	KOBJMETHOD(iconv_converter_init,	iconv_xlat_init),
	KOBJMETHOD(iconv_converter_done,	iconv_xlat_done),
#endif
	KOBJMETHOD(iconv_converter_name,	iconv_xlat_name),
	{0, 0}
};

KICONV_CONVERTER(xlat, sizeof(struct iconv_xlat));