
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)gethostnamadr.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <nsswitch.h>
#include <arpa/nameser.h>	/* XXX */
#include <resolv.h>		/* XXX */
#include "netdb_private.h"

void
_sethosthtent(int f, struct hostent_data *hed)
{
	if (!hed->hostf)
		hed->hostf = fopen(_PATH_HOSTS, "r");
	else
		rewind(hed->hostf);
	hed->stayopen = f;
}

void
_endhosthtent(struct hostent_data *hed)
{
	if (hed->hostf && !hed->stayopen) {
		(void) fclose(hed->hostf);
		hed->hostf = NULL;
	}
}

static int
gethostent_p(struct hostent *he, struct hostent_data *hed, int mapped,
    res_state statp)
{
	char *p, *bp, *ep;
	char *cp, **q;
	int af, len;
	char hostbuf[BUFSIZ + 1];

	if (!hed->hostf && !(hed->hostf = fopen(_PATH_HOSTS, "r"))) {
		RES_SET_H_ERRNO(statp, NETDB_INTERNAL);
		return (-1);
	}
 again:
	if (!(p = fgets(hostbuf, sizeof hostbuf, hed->hostf))) {
		RES_SET_H_ERRNO(statp, HOST_NOT_FOUND);
		return (-1);
	}
	if (*p == '#')
		goto again;
	cp = strpbrk(p, "#\n");
	if (cp != NULL)
		*cp = '\0';
	if (!(cp = strpbrk(p, " \t")))
		goto again;
	*cp++ = '\0';
	if (inet_pton(AF_INET6, p, hed->host_addr) > 0) {
		af = AF_INET6;
		len = IN6ADDRSZ;
	} else if (inet_pton(AF_INET, p, hed->host_addr) > 0) {
		if (mapped) {
			_map_v4v6_address((char *)hed->host_addr,
			    (char *)hed->host_addr);
			af = AF_INET6;
			len = IN6ADDRSZ;
		} else {
			af = AF_INET;
			len = INADDRSZ;
		}
	} else {
		goto again;
	}
	hed->h_addr_ptrs[0] = (char *)hed->host_addr;
	hed->h_addr_ptrs[1] = NULL;
	he->h_addr_list = hed->h_addr_ptrs;
	he->h_length = len;
	he->h_addrtype = af;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	bp = hed->hostbuf;
	ep = hed->hostbuf + sizeof hed->hostbuf;
	he->h_name = bp;
	q = he->h_aliases = hed->host_aliases;
	if ((p = strpbrk(cp, " \t")) != NULL)
		*p++ = '\0';
	len = strlen(cp) + 1;
	if (ep - bp < len) {
		RES_SET_H_ERRNO(statp, NO_RECOVERY);
		return (-1);
	}
	strlcpy(bp, cp, ep - bp);
	bp += len;
	cp = p;
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q >= &hed->host_aliases[_MAXALIASES - 1])
			break;
		if ((p = strpbrk(cp, " \t")) != NULL)
			*p++ = '\0';
		len = strlen(cp) + 1;
		if (ep - bp < len)
			break;
		strlcpy(bp, cp, ep - bp);
		*q++ = bp;
		bp += len;
		cp = p;
	}
	*q = NULL;
	RES_SET_H_ERRNO(statp, NETDB_SUCCESS);
	return (0);
}

int
gethostent_r(struct hostent *hptr, char *buffer, size_t buflen,
    struct hostent **result, int *h_errnop)
{
	struct hostent_data *hed;
	struct hostent he;
	res_state statp;

	statp = __res_state();
	if ((statp->options & RES_INIT) == 0 && res_ninit(statp) == -1) {
		RES_SET_H_ERRNO(statp, NETDB_INTERNAL);
		*h_errnop = statp->res_h_errno;
		return (-1);
	}
	if ((hed = __hostent_data_init()) == NULL) {
		RES_SET_H_ERRNO(statp, NETDB_INTERNAL);
		*h_errnop = statp->res_h_errno;
		return (-1);
	}
	if (gethostent_p(&he, hed, statp->options & RES_USE_INET6, statp) != 0)
		return (-1);
	if (__copy_hostent(&he, hptr, buffer, buflen) != 0) {
		RES_SET_H_ERRNO(statp, NETDB_INTERNAL);
		*h_errnop = statp->res_h_errno;
		return ((errno != 0) ? errno : -1);
	}
	*result = hptr;
	return (0);
}

struct hostent *
gethostent(void)
{
	struct hostdata *hd;
	struct hostent *rval;
	int ret_h_errno;

	if ((hd = __hostdata_init()) == NULL)
		return (NULL);
	if (gethostent_r(&hd->host, hd->data, sizeof(hd->data), &rval,
	    &ret_h_errno) != 0)
		return (NULL);
	return (rval);
}

int
_ht_gethostbyname(void *rval, void *cb_data, va_list ap)
{
	const char *name;
	int af;
	char *buffer;
	size_t buflen;
	int *errnop, *h_errnop;
	struct hostent *hptr, he;
	struct hostent_data *hed;
	char **cp;
	res_state statp;
	int error;

	name = va_arg(ap, const char *);
	af = va_arg(ap, int);
	hptr = va_arg(ap, struct hostent *);
	buffer = va_arg(ap, char *);
	buflen = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);
	h_errnop = va_arg(ap, int *);

	*((struct hostent **)rval) = NULL;

	statp = __res_state();
	if ((hed = __hostent_data_init()) == NULL) {
		RES_SET_H_ERRNO(statp, NETDB_INTERNAL);
		*h_errnop = statp->res_h_errno;
		return (NS_NOTFOUND);
	}

	_sethosthtent(0, hed);
	while ((error = gethostent_p(&he, hed, 0, statp)) == 0) {
		if (he.h_addrtype != af)
			continue;
		if (he.h_addrtype == AF_INET &&
		    statp->options & RES_USE_INET6) {
			_map_v4v6_address(he.h_addr, he.h_addr);
			he.h_length = IN6ADDRSZ;
			he.h_addrtype = AF_INET6;
		}
		if (strcasecmp(he.h_name, name) == 0)
			break;
		for (cp = he.h_aliases; *cp != 0; cp++)
			if (strcasecmp(*cp, name) == 0)
				goto found;
	}
found:
	_endhosthtent(hed);

	if (error != 0) {
		*h_errnop = statp->res_h_errno;
		return (NS_NOTFOUND);
	}
	if (__copy_hostent(&he, hptr, buffer, buflen) != 0) {
		*errnop = errno;
		RES_SET_H_ERRNO(statp, NETDB_INTERNAL);
		*h_errnop = statp->res_h_errno;
		return (NS_RETURN);
	}
	*((struct hostent **)rval) = hptr;
	return (NS_SUCCESS);
}

int
_ht_gethostbyaddr(void *rval, void *cb_data, va_list ap)
{
	const void *addr;
	socklen_t len;
	int af;
	char *buffer;
	size_t buflen;
	int *errnop, *h_errnop;
	struct hostent *hptr, he;
	struct hostent_data *hed;
	res_state statp;
	int error;

	addr = va_arg(ap, const void *);
	len = va_arg(ap, socklen_t);
	af = va_arg(ap, int);
	hptr = va_arg(ap, struct hostent *);
	buffer = va_arg(ap, char *);
	buflen = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);
	h_errnop = va_arg(ap, int *);

	*((struct hostent **)rval) = NULL;

	statp = __res_state();
	if ((hed = __hostent_data_init()) == NULL) {
		RES_SET_H_ERRNO(statp, NETDB_INTERNAL);
		*h_errnop = statp->res_h_errno;
		return (NS_NOTFOUND);
	}

	_sethosthtent(0, hed);
	while ((error = gethostent_p(&he, hed, 0, statp)) == 0)
		if (he.h_addrtype == af && !bcmp(he.h_addr, addr, len)) {
			if (he.h_addrtype == AF_INET &&
			    statp->options & RES_USE_INET6) {
				_map_v4v6_address(he.h_addr, he.h_addr);
				he.h_length = IN6ADDRSZ;
				he.h_addrtype = AF_INET6;
			}
			break;
		}
	_endhosthtent(hed);

	if (error != 0)
		return (NS_NOTFOUND);
	if (__copy_hostent(&he, hptr, buffer, buflen) != 0) {
		*errnop = errno;
		RES_SET_H_ERRNO(statp, NETDB_INTERNAL);
		*h_errnop = statp->res_h_errno;
		return (NS_RETURN);
	}
	*((struct hostent **)rval) = hptr;
	return (NS_SUCCESS);
}