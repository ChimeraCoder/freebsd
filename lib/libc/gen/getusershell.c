
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
static char sccsid[] = "@(#)getusershell.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
/*	$NetBSD: getusershell.c,v 1.17 1999/01/25 01:09:34 lukem Exp $	*/
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include <sys/file.h>

#include <ctype.h>
#include <errno.h>
#include <nsswitch.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stringlist.h>
#include <unistd.h>

#ifdef HESIOD
#include <hesiod.h>
#endif
#ifdef YP
#include <rpc/rpc.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>
#endif
#include "un-namespace.h"

static const char *const *curshell;
static StringList	 *sl;

static const char *const *initshells(void);

/*
 * Get a list of shells from "shells" nsswitch database
 */
char *
getusershell(void)
{
	char *ret;

	if (curshell == NULL)
		curshell = initshells();
	/*LINTED*/
	ret = (char *)*curshell;
	if (ret != NULL)
		curshell++;
	return (ret);
}

void
endusershell(void)
{
	if (sl) {
		sl_free(sl, 1);
		sl = NULL;
	}
	curshell = NULL;
}

void
setusershell(void)
{

	curshell = initshells();
}


static int	_local_initshells(void *, void *, va_list);

/*ARGSUSED*/
static int
_local_initshells(rv, cb_data, ap)
	void	*rv;
	void	*cb_data;
	va_list	 ap;
{
	char	*sp, *cp;
	FILE	*fp;
	char	 line[MAXPATHLEN + 2];

	if (sl)
		sl_free(sl, 1);
	sl = sl_init();

	if ((fp = fopen(_PATH_SHELLS, "re")) == NULL)
		return NS_UNAVAIL;

	cp = line;
	while (fgets(cp, MAXPATHLEN + 1, fp) != NULL) {
		while (*cp != '#' && *cp != '/' && *cp != '\0')
			cp++;
		if (*cp == '#' || *cp == '\0')
			continue;
		sp = cp;
		while (!isspace(*cp) && *cp != '#' && *cp != '\0')
			cp++;
		*cp++ = '\0';
		sl_add(sl, strdup(sp));
	}
	(void)fclose(fp);
	return NS_SUCCESS;
}

#ifdef HESIOD
static int	_dns_initshells(void *, void *, va_list);

/*ARGSUSED*/
static int
_dns_initshells(rv, cb_data, ap)
	void	*rv;
	void	*cb_data;
	va_list	 ap;
{
	char	  shellname[] = "shells-XXXXX";
	int	  hsindex, hpi, r;
	char	**hp;
	void	 *context;

	if (sl)
		sl_free(sl, 1);
	sl = sl_init();
	r = NS_UNAVAIL;
	if (hesiod_init(&context) == -1)
		return (r);

	for (hsindex = 0; ; hsindex++) {
		snprintf(shellname, sizeof(shellname)-1, "shells-%d", hsindex);
		hp = hesiod_resolve(context, shellname, "shells");
		if (hp == NULL) {
			if (errno == ENOENT) {
				if (hsindex == 0)
					r = NS_NOTFOUND;
				else
					r = NS_SUCCESS;
			}
			break;
		} else {
			for (hpi = 0; hp[hpi]; hpi++)
				sl_add(sl, hp[hpi]);
			free(hp);
		}
	}
	hesiod_end(context);
	return (r);
}
#endif /* HESIOD */

#ifdef YP
static int	_nis_initshells(void *, void *, va_list);

/*ARGSUSED*/
static int
_nis_initshells(rv, cb_data, ap)
	void	*rv;
	void	*cb_data;
	va_list	 ap;
{
	static char *ypdomain;
	char	*key, *data;
	char	*lastkey;
	int	 keylen, datalen;
	int	 r;

	if (sl)
		sl_free(sl, 1);
	sl = sl_init();

	if (ypdomain == NULL) {
		switch (yp_get_default_domain(&ypdomain)) {
		case 0:
			break;
		case YPERR_RESRC:
			return NS_TRYAGAIN;
		default:
			return NS_UNAVAIL;
		}
	}

	/*
	 * `key' and `data' point to strings dynamically allocated by
	 * the yp_... functions.
	 * `data' is directly put into the stringlist of shells.
	 */
	key = data = NULL;
	if (yp_first(ypdomain, "shells", &key, &keylen, &data, &datalen))
		return NS_UNAVAIL;
	do {
		data[datalen] = '\0';		/* clear trailing \n */
		sl_add(sl, data);

		lastkey = key;
		r = yp_next(ypdomain, "shells", lastkey, keylen,
		    &key, &keylen, &data, &datalen);
		free(lastkey);
	} while (r == 0);
	
	if (r == YPERR_NOMORE) {
		/*
		 * `data' and `key' ought to be NULL - do not try to free them.
		 */
		return NS_SUCCESS;
	}

	return NS_UNAVAIL;
}
#endif /* YP */

static const char *const *
initshells()
{
	static const ns_dtab dtab[] = {
		NS_FILES_CB(_local_initshells, NULL)
		NS_DNS_CB(_dns_initshells, NULL)
		NS_NIS_CB(_nis_initshells, NULL)
		{ 0 }
	};
	if (sl)
		sl_free(sl, 1);
	sl = sl_init();

	if (_nsdispatch(NULL, dtab, NSDB_SHELLS, "initshells", __nsdefaultsrc)
	    != NS_SUCCESS) {
		if (sl)
			sl_free(sl, 1);
		sl = sl_init();
		/*
		 * Local shells should NOT be added here.  They should be
		 * added in /etc/shells.
		 */
		sl_add(sl, strdup(_PATH_BSHELL));
		sl_add(sl, strdup(_PATH_CSHELL));
	}
	sl_add(sl, NULL);

	return (const char *const *)(sl->sl_str);
}