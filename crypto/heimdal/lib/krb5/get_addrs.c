
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

#ifdef __osf__
/* hate */
struct rtentry;
struct mbuf;
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#include <ifaddrs.h>

static krb5_error_code
gethostname_fallback (krb5_context context, krb5_addresses *res)
{
    krb5_error_code ret;
    char hostname[MAXHOSTNAMELEN];
    struct hostent *hostent;

    if (gethostname (hostname, sizeof(hostname))) {
	ret = errno;
	krb5_set_error_message(context, ret, "gethostname: %s", strerror(ret));
	return ret;
    }
    hostent = roken_gethostbyname (hostname);
    if (hostent == NULL) {
	ret = errno;
	krb5_set_error_message (context, ret, "gethostbyname %s: %s",
				hostname, strerror(ret));
	return ret;
    }
    res->len = 1;
    res->val = malloc (sizeof(*res->val));
    if (res->val == NULL) {
	krb5_set_error_message(context, ENOMEM, N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    res->val[0].addr_type = hostent->h_addrtype;
    res->val[0].address.data = NULL;
    res->val[0].address.length = 0;
    ret = krb5_data_copy (&res->val[0].address,
			  hostent->h_addr,
			  hostent->h_length);
    if (ret) {
	free (res->val);
	return ret;
    }
    return 0;
}

enum {
    LOOP            = 1,	/* do include loopback addrs */
    LOOP_IF_NONE    = 2,	/* include loopback addrs if no others */
    EXTRA_ADDRESSES = 4,	/* include extra addresses */
    SCAN_INTERFACES = 8		/* scan interfaces for addresses */
};

/*
 * Try to figure out the addresses of all configured interfaces with a
 * lot of magic ioctls.
 */

static krb5_error_code
find_all_addresses (krb5_context context, krb5_addresses *res, int flags)
{
    struct sockaddr sa_zero;
    struct ifaddrs *ifa0, *ifa;
    krb5_error_code ret = ENXIO;
    unsigned int num, idx;
    krb5_addresses ignore_addresses;

    if (getifaddrs(&ifa0) == -1) {
	ret = errno;
	krb5_set_error_message(context, ret, "getifaddrs: %s", strerror(ret));
	return (ret);
    }

    memset(&sa_zero, 0, sizeof(sa_zero));

    /* First, count all the ifaddrs. */
    for (ifa = ifa0, num = 0; ifa != NULL; ifa = ifa->ifa_next, num++)
	/* nothing */;

    if (num == 0) {
	freeifaddrs(ifa0);
	krb5_set_error_message(context, ENXIO, N_("no addresses found", ""));
	return (ENXIO);
    }

    if (flags & EXTRA_ADDRESSES) {
	/* we'll remove the addresses we don't care about */
	ret = krb5_get_ignore_addresses(context, &ignore_addresses);
	if(ret)
	    return ret;
    }

    /* Allocate storage for them. */
    res->val = calloc(num, sizeof(*res->val));
    if (res->val == NULL) {
	krb5_free_addresses(context, &ignore_addresses);
	freeifaddrs(ifa0);
	krb5_set_error_message(context, ENOMEM, N_("malloc: out of memory", ""));
	return ENOMEM;
    }

    /* Now traverse the list. */
    for (ifa = ifa0, idx = 0; ifa != NULL; ifa = ifa->ifa_next) {
	if ((ifa->ifa_flags & IFF_UP) == 0)
	    continue;
	if (ifa->ifa_addr == NULL)
	    continue;
	if (memcmp(ifa->ifa_addr, &sa_zero, sizeof(sa_zero)) == 0)
	    continue;
	if (krb5_sockaddr_uninteresting(ifa->ifa_addr))
	    continue;
	if (krb5_sockaddr_is_loopback(ifa->ifa_addr) && (flags & LOOP) == 0)
	    /* We'll deal with the LOOP_IF_NONE case later. */
	    continue;

	ret = krb5_sockaddr2address(context, ifa->ifa_addr, &res->val[idx]);
	if (ret) {
	    /*
	     * The most likely error here is going to be "Program
	     * lacks support for address type".  This is no big
	     * deal -- just continue, and we'll listen on the
	     * addresses who's type we *do* support.
	     */
	    continue;
	}
	/* possibly skip this address? */
	if((flags & EXTRA_ADDRESSES) &&
	   krb5_address_search(context, &res->val[idx], &ignore_addresses)) {
	    krb5_free_address(context, &res->val[idx]);
	    flags &= ~LOOP_IF_NONE; /* we actually found an address,
                                       so don't add any loop-back
                                       addresses */
	    continue;
	}

	idx++;
    }

    /*
     * If no addresses were found, and LOOP_IF_NONE is set, then find
     * the loopback addresses and add them to our list.
     */
    if ((flags & LOOP_IF_NONE) != 0 && idx == 0) {
	for (ifa = ifa0; ifa != NULL; ifa = ifa->ifa_next) {
	    if ((ifa->ifa_flags & IFF_UP) == 0)
		continue;
	    if (ifa->ifa_addr == NULL)
		continue;
	    if (memcmp(ifa->ifa_addr, &sa_zero, sizeof(sa_zero)) == 0)
		continue;
	    if (krb5_sockaddr_uninteresting(ifa->ifa_addr))
		continue;
	    if (!krb5_sockaddr_is_loopback(ifa->ifa_addr))
		continue;
	    if ((ifa->ifa_flags & IFF_LOOPBACK) == 0)
		/* Presumably loopback addrs are only used on loopback ifs! */
		continue;
	    ret = krb5_sockaddr2address(context,
					ifa->ifa_addr, &res->val[idx]);
	    if (ret)
		continue; /* We don't consider this failure fatal */
	    if((flags & EXTRA_ADDRESSES) &&
	       krb5_address_search(context, &res->val[idx],
				   &ignore_addresses)) {
		krb5_free_address(context, &res->val[idx]);
		continue;
	    }
	    idx++;
	}
    }

    if (flags & EXTRA_ADDRESSES)
	krb5_free_addresses(context, &ignore_addresses);
    freeifaddrs(ifa0);
    if (ret) {
	free(res->val);
	res->val = NULL;
    } else
	res->len = idx;        /* Now a count. */
    return (ret);
}

static krb5_error_code
get_addrs_int (krb5_context context, krb5_addresses *res, int flags)
{
    krb5_error_code ret = -1;

    res->len = 0;
    res->val = NULL;

    if (flags & SCAN_INTERFACES) {
	ret = find_all_addresses (context, res, flags);
	if(ret || res->len == 0)
	    ret = gethostname_fallback (context, res);
    } else {
	ret = 0;
    }

    if(ret == 0 && (flags & EXTRA_ADDRESSES)) {
	krb5_addresses a;
	/* append user specified addresses */
	ret = krb5_get_extra_addresses(context, &a);
	if(ret) {
	    krb5_free_addresses(context, res);
	    return ret;
	}
	ret = krb5_append_addresses(context, res, &a);
	if(ret) {
	    krb5_free_addresses(context, res);
	    return ret;
	}
	krb5_free_addresses(context, &a);
    }
    if(res->len == 0) {
	free(res->val);
	res->val = NULL;
    }
    return ret;
}

/*
 * Try to get all addresses, but return the one corresponding to
 * `hostname' if we fail.
 *
 * Only include loopback address if there are no other.
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_get_all_client_addrs (krb5_context context, krb5_addresses *res)
{
    int flags = LOOP_IF_NONE | EXTRA_ADDRESSES;

    if (context->scan_interfaces)
	flags |= SCAN_INTERFACES;

    return get_addrs_int (context, res, flags);
}

/*
 * Try to get all local addresses that a server should listen to.
 * If that fails, we return the address corresponding to `hostname'.
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_get_all_server_addrs (krb5_context context, krb5_addresses *res)
{
    return get_addrs_int (context, res, LOOP | SCAN_INTERFACES);
}