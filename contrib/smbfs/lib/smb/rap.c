
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
#include <sys/param.h>
#include <sys/endian.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sysexits.h>

#include <netsmb/smb_lib.h>
#include <netsmb/smb_conn.h>
#include <netsmb/smb_rap.h>

/*#include <sys/ioctl.h>*/

static int
smb_rap_parserqparam(const char *s, char **next, int *rlen)
{
	char *np;
	int len, m;

	m = 1;
	switch (*s++) {
	    case 'L':
	    case 'T':
	    case 'W':
		len = 2;
		break;
	    case 'D':
	    case 'O':
		len = 4;
		break;
	    case 'b':
	    case 'F':
		len = 1;
		break;
	    case 'r':
	    case 's':
		len = 0;
		break;
	    default:
		return EINVAL;
	}
	if (isdigit(*s)) {
		len *= strtoul(s, &np, 10);
		s = np;
	}
	*rlen = len;
	*(const char**)next = s;
	return 0;
}

static int
smb_rap_parserpparam(const char *s, char **next, int *rlen)
{
	char *np;
	int len, m;

	m = 1;
	switch (*s++) {
	    case 'e':
	    case 'h':
		len = 2;
		break;
	    case 'i':
		len = 4;
		break;
	    case 'g':
		len = 1;
		break;
	    default:
		return EINVAL;
	}
	if (isdigit(*s)) {
		len *= strtoul(s, &np, 10);
		s = np;
	}
	*rlen = len;
	*(const char**)next = s;
	return 0;
}

static int
smb_rap_parserpdata(const char *s, char **next, int *rlen)
{
	char *np;
	int len, m;

	m = 1;
	switch (*s++) {
	    case 'B':
		len = 1;
		break;
	    case 'W':
		len = 2;
		break;
	    case 'D':
	    case 'O':
	    case 'z':
		len = 4;
		break;
	    default:
		return EINVAL;
	}
	if (isdigit(*s)) {
		len *= strtoul(s, &np, 10);
		s = np;
	}
	*rlen = len;
	*(const char**)next = s;
	return 0;
}

static int
smb_rap_rqparam_z(struct smb_rap *rap, const char *value)
{
	int len = strlen(value) + 1;

	bcopy(value, rap->r_npbuf, len);
	rap->r_npbuf += len;
	rap->r_plen += len;
	return 0;
}

static int
smb_rap_rqparam(struct smb_rap *rap, char ptype, char plen, long value)
{
	char *p = rap->r_npbuf;
	int len;

	switch (ptype) {
	    case 'L':
	    case 'W':
		setwle(p, 0, value);
		len = 2;
		break;
	    case 'D':
		setdle(p, 0, value);
		len = 4;
		break;
	    case 'b':
		memset(p, value, plen);
		len = plen;
	    default:
		return EINVAL;
	}
	rap->r_npbuf += len;
	rap->r_plen += len;
	return 0;
}

int
smb_rap_create(int fn, const char *param, const char *data,
	struct smb_rap **rapp)
{
	struct smb_rap *rap;
	char *p;
	int plen, len;

	rap = malloc(sizeof(*rap));
	if (rap == NULL)
		return ENOMEM;
	bzero(rap, sizeof(*rap));
	p = rap->r_sparam = rap->r_nparam = strdup(param);
	rap->r_sdata = rap->r_ndata = strdup(data);
	/*
	 * Calculate length of request parameter block
	 */
	len = 2 + strlen(param) + 1 + strlen(data) + 1;
	
	while (*p) {
		if (smb_rap_parserqparam(p, &p, &plen) != 0)
			break;
		len += plen;
	}
	rap->r_pbuf = rap->r_npbuf = malloc(len);
	smb_rap_rqparam(rap, 'W', 1, fn);
	smb_rap_rqparam_z(rap, rap->r_sparam);
	smb_rap_rqparam_z(rap, rap->r_sdata);
	*rapp = rap;
	return 0;
}

void
smb_rap_done(struct smb_rap *rap)
{
	if (rap->r_sparam)
		free(rap->r_sparam);
	if (rap->r_sdata)
		free(rap->r_sdata);
	free(rap);
}

int
smb_rap_setNparam(struct smb_rap *rap, long value)
{
	char *p = rap->r_nparam;
	char ptype = *p;
	int error, plen;

	error = smb_rap_parserqparam(p, &p, &plen);
	if (error)
		return error;
	switch (ptype) {
	    case 'L':
		rap->r_rcvbuflen = value;
		/* FALLTHROUGH */
	    case 'W':
	    case 'D':
	    case 'b':
		error = smb_rap_rqparam(rap, ptype, plen, value);
		break;
	    default:
		return EINVAL;
	}
	rap->r_nparam = p;
	return 0;
}

int
smb_rap_setPparam(struct smb_rap *rap, void *value)
{
	char *p = rap->r_nparam;
	char ptype = *p;
	int error, plen;

	error = smb_rap_parserqparam(p, &p, &plen);
	if (error)
		return error;
	switch (ptype) {
	    case 'r':
		rap->r_rcvbuf = value;
		break;
	    default:
		return EINVAL;
	}
	rap->r_nparam = p;
	return 0;
}

static int
smb_rap_getNparam(struct smb_rap *rap, long *value)
{
	char *p = rap->r_nparam;
	char ptype = *p;
	int error, plen;

	error = smb_rap_parserpparam(p, &p, &plen);
	if (error)
		return error;
	switch (ptype) {
	    case 'h':
		*value = le16toh(*(u_int16_t*)rap->r_npbuf);
		break;
	    default:
		return EINVAL;
	}
	rap->r_npbuf += plen;
	rap->r_nparam = p;
	return 0;
}

int
smb_rap_request(struct smb_rap *rap, struct smb_ctx *ctx)
{
	u_int16_t *rp, conv;
	u_int32_t *p32;
	char *dp, *p = rap->r_nparam;
	char ptype;
	int error, rdatacnt, rparamcnt, entries, done, dlen;

	rdatacnt = rap->r_rcvbuflen;
	rparamcnt = rap->r_plen;
	error = smb_t2_request(ctx, 0, 0, "\\PIPE\\LANMAN",
	    rap->r_plen, rap->r_pbuf,		/* int tparamcnt, void *tparam */
	    0, NULL,				/* int tdatacnt, void *tdata */
	    &rparamcnt, rap->r_pbuf,		/* rparamcnt, void *rparam */
	    &rdatacnt, rap->r_rcvbuf		/* int *rdatacnt, void *rdata */
	);
	if (error)
		return error;
	rp = (u_int16_t*)rap->r_pbuf;
	rap->r_result = le16toh(*rp++);
	conv = le16toh(*rp++);
	rap->r_npbuf = (char*)rp;
	rap->r_entries = entries = 0;
	done = 0;
	while (!done && *p) {
		ptype = *p;
		switch (ptype) {
		    case 'e':
			rap->r_entries = entries = le16toh(*(u_int16_t*)rap->r_npbuf);
			rap->r_npbuf += 2;
			p++;
			break;
		    default:
			done = 1;
		}
/*		error = smb_rap_parserpparam(p, &p, &plen);
		if (error) {
			smb_error("reply parameter mismath %s", 0, p);
			return EBADRPC;
		}*/
	}
	rap->r_nparam = p;
	/*
	 * In general, unpacking entries we may need to relocate
	 * entries for proper alingning. For now use them as is.
	 */
	dp = rap->r_rcvbuf;
	while (entries--) {
		p = rap->r_sdata;
		while (*p) {
			ptype = *p;
			error = smb_rap_parserpdata(p, &p, &dlen);
			if (error) {
				smb_error("reply data mismath %s", 0, p);
				return EBADRPC;
			}
			switch (ptype) {
			    case 'z':
				p32 = (u_int32_t*)dp;
				*p32 = (*p32 & 0xffff) - conv;
				break;
			}
			dp += dlen;
		}
	}
	return error;
}

int
smb_rap_error(struct smb_rap *rap, int error)
{
	if (error)
		return error;
	if (rap->r_result == 0)
		return 0;
	return rap->r_result | SMB_RAP_ERROR;
}

int
smb_rap_NetShareEnum(struct smb_ctx *ctx, int sLevel, void *pbBuffer,
	int cbBuffer, int *pcEntriesRead, int *pcTotalAvail)
{
	struct smb_rap *rap;
	long lval;
	int error;

	error = smb_rap_create(0, "WrLeh", "B13BWz", &rap);
	if (error)
		return error;
	smb_rap_setNparam(rap, sLevel);		/* W - sLevel */
	smb_rap_setPparam(rap, pbBuffer);	/* r - pbBuffer */
	smb_rap_setNparam(rap, cbBuffer);	/* L - cbBuffer */
	error = smb_rap_request(rap, ctx);
	if (error == 0) {
		*pcEntriesRead = rap->r_entries;
		error = smb_rap_getNparam(rap, &lval);
		*pcTotalAvail = lval;
	}
	error = smb_rap_error(rap, error);
	smb_rap_done(rap);
	return error;
}