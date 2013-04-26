
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
static char sccsid[] = "@(#)getprotoname.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <errno.h>
#include <netdb.h>
#include <nsswitch.h>
#include <string.h>
#include "netdb_private.h"
#ifdef NS_CACHING
#include "nscache.h"
#endif
#include "nss_tls.h"

static const ns_src defaultsrc[] = {
	{ NSSRC_FILES, NS_SUCCESS },
	{ NULL, 0 }
};

#ifdef NS_CACHING
extern int __proto_id_func(char *, size_t *, va_list, void *);
extern int __proto_marshal_func(char *, size_t *, void *, va_list, void *);
extern int __proto_unmarshal_func(char *, size_t, void *, va_list, void *);
#endif

static int
files_getprotobyname(void *retval, void *mdata, va_list ap)
{
	struct protoent pe;
	struct protoent_data *ped;
	char **cp;
	int error;

	char *name;
	struct protoent	*pptr;
	char *buffer;
	size_t buflen;
	int *errnop;

	name = va_arg(ap, char *);
	pptr = va_arg(ap, struct protoent *);
	buffer = va_arg(ap, char *);
	buflen = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);


	if ((ped = __protoent_data_init()) == NULL) {
		*errnop = errno;
		return (NS_NOTFOUND);
	}

	__setprotoent_p(ped->stayopen, ped);
	while ((error = __getprotoent_p(&pe, ped)) == 0) {
		if (strcmp(pe.p_name, name) == 0)
			break;
		for (cp = pe.p_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	if (!ped->stayopen)
		__endprotoent_p(ped);
	if (error != 0) {
		*errnop = errno;
		return (NS_NOTFOUND);
	}
	if (__copy_protoent(&pe, pptr, buffer, buflen) != 0) {
		*errnop = errno;
		return (NS_RETURN);
	}

	*((struct protoent **)retval) = pptr;
	return (NS_SUCCESS);
}


int
getprotobyname_r(const char *name, struct protoent *pptr, char *buffer,
    size_t buflen, struct protoent **result)
{
#ifdef NS_CACHING
	static const nss_cache_info cache_info =
		NS_COMMON_CACHE_INFO_INITIALIZER(
		protocols, (void *)nss_lt_name,
		__proto_id_func, __proto_marshal_func, __proto_unmarshal_func);
#endif
	static const ns_dtab dtab[] = {
		{ NSSRC_FILES, files_getprotobyname, NULL },
#ifdef NS_CACHING
		NS_CACHE_CB(&cache_info)
#endif
		{ NULL, NULL, NULL }
	};
	int	rv, ret_errno;

	ret_errno = 0;
	*result = NULL;
	rv = nsdispatch(result, dtab, NSDB_PROTOCOLS, "getprotobyname_r",
	    defaultsrc, name, pptr, buffer, buflen, &ret_errno);

	if (rv != NS_SUCCESS) {
		errno = ret_errno;
		return (ret_errno);
	}
	return (0);
}

struct protoent *
getprotobyname(const char *name)
{
	struct protodata *pd;
	struct protoent *rval;

	if ((pd = __protodata_init()) == NULL)
		return (NULL);
	if (getprotobyname_r(name, &pd->proto, pd->data, sizeof(pd->data),
	    &rval) != 0)
		return (NULL);
	return (rval);
}