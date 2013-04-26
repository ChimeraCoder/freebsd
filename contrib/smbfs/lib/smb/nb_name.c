
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
#include <sys/endian.h>
#include <sys/socket.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netsmb/netbios.h>
#include <netsmb/smb_lib.h>
#include <netsmb/nb_lib.h>

int
nb_snballoc(int namelen, struct sockaddr_nb **dst)
{
	struct sockaddr_nb *snb;
	int slen;

	slen = namelen + sizeof(*snb) - sizeof(snb->snb_name);
	snb = malloc(slen);
	if (snb == NULL)
		return ENOMEM;
	bzero(snb, slen);
	snb->snb_family = AF_NETBIOS;
	snb->snb_len = slen;
	*dst = snb;
	return 0;
}

void
nb_snbfree(struct sockaddr *snb)
{
	free(snb);
}

/*
 * Create a full NETBIOS address
 */
int
nb_sockaddr(struct sockaddr *peer, struct nb_name *np,
	struct sockaddr_nb **dst)

{
	struct sockaddr_nb *snb;
	int nmlen, error;

	if (peer && (peer->sa_family != AF_INET && peer->sa_family != AF_IPX))
		return EPROTONOSUPPORT;
	nmlen = nb_name_len(np);
	if (nmlen < NB_ENCNAMELEN)
		return EINVAL;
	error = nb_snballoc(nmlen, &snb);
	if (error)
		return error;
	if (nmlen != nb_name_encode(np, snb->snb_name))
		printf("a bug somewhere in the nb_name* code\n");
	if (peer)
		memcpy(&snb->snb_tran, peer, peer->sa_len);
	*dst = snb;
	return 0;
}

int
nb_name_len(struct nb_name *np)
{
	u_char *name;
	int len, sclen;

	len = 1 + NB_ENCNAMELEN;
	if (np->nn_scope == NULL)
		return len + 1;
	sclen = 0;
	for (name = np->nn_scope; *name; name++) {
		if (*name == '.') {
			sclen = 0;
		} else {
			if (sclen < NB_MAXLABLEN) {
				sclen++;
				len++;
			}
		}
	}
	return len + 1;
}

int
nb_encname_len(const char *str)
{
	const u_char *cp = (const u_char *)str;
	int len, blen;

	if ((cp[0] & 0xc0) == 0xc0)
		return -1;	/* first two bytes are offset to name */

	len = 1;
	for (;;) {
		blen = *cp;
		if (blen++ == 0)
			break;
		len += blen;
		cp += blen;
	}
	return len;
}

#define	NBENCODE(c)	(htole16((u_short)(((u_char)(c) >> 4) | \
			 (((u_char)(c) & 0xf) << 8)) + 0x4141))

static void
memsetw(char *dst, int n, u_short word)
{
	while (n--) {
		*(u_short*)dst = word;
		dst += 2;
	}
}

int
nb_name_encode(struct nb_name *np, u_char *dst)
{
	u_char *name, *plen;
	u_char *cp = dst;
	int i, lblen;

	*cp++ = NB_ENCNAMELEN;
	name = np->nn_name;
	if (name[0] == '*' && name[1] == 0) {
		*(u_short*)cp = NBENCODE('*');
		memsetw(cp + 2, NB_NAMELEN - 1, NBENCODE(' '));
		cp += NB_ENCNAMELEN;
	} else {
		for (i = 0; *name && i < NB_NAMELEN - 1; i++, cp += 2, name++)
			*(u_short*)cp = NBENCODE(toupper(*name));
		i = NB_NAMELEN - i - 1;
		if (i > 0) {
			memsetw(cp, i, NBENCODE(' '));
			cp += i * 2;
		}
		*(u_short*)cp = NBENCODE(np->nn_type);
		cp += 2;
	}
	*cp = 0;
	if (np->nn_scope == NULL)
		return nb_encname_len(dst);
	plen = cp++;
	lblen = 0;
	for (name = np->nn_scope; ; name++) {
		if (*name == '.' || *name == 0) {
			*plen = lblen;
			plen = cp++;
			*plen = 0;
			if (*name == 0)
				break;
		} else {
			if (lblen < NB_MAXLABLEN) {
				*cp++ = *name;
				lblen++;
			}
		}
	}
	return nb_encname_len(dst);
}