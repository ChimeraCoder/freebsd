
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
#include <sys/param.h>
#include <errno.h>
#include <nss.h>
#include <pthread.h>
#include <pthread_np.h>
#include "un-namespace.h"
#include "libc_private.h"


struct group;
struct passwd;

static int	terminator;

#define DECLARE_TERMINATOR(x)					\
static pthread_key_t	 _term_key_##x;				\
static void							\
_term_create_##x(void)						\
{								\
	(void)_pthread_key_create(&_term_key_##x, NULL);	\
}								\
static void		*_term_main_##x;			\
static pthread_once_t	 _term_once_##x = PTHREAD_ONCE_INIT

#define SET_TERMINATOR(x, y)						\
do {									\
	if (!__isthreaded || _pthread_main_np())			\
		_term_main_##x = (y);					\
	else {								\
		(void)_pthread_once(&_term_once_##x, _term_create_##x);	\
		(void)_pthread_setspecific(_term_key_##x, y);		\
	}								\
} while (0)

#define CHECK_TERMINATOR(x)					\
(!__isthreaded || _pthread_main_np() ?				\
    (_term_main_##x) :						\
    ((void)_pthread_once(&_term_once_##x, _term_create_##x),	\
    _pthread_getspecific(_term_key_##x)))



DECLARE_TERMINATOR(group);


int
__nss_compat_getgrnam_r(void *retval, void *mdata, va_list ap)
{
	int (*fn)(const char *, struct group *, char *, size_t, int *);
	const char	*name;
	struct group	*grp;
	char		*buffer;
	int		*errnop;
	size_t		 bufsize;
	enum nss_status	 status;

	fn = mdata;
	name = va_arg(ap, const char *);
	grp = va_arg(ap, struct group *);
	buffer = va_arg(ap, char *);
	bufsize = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);
	status = fn(name, grp, buffer, bufsize, errnop);
	status = __nss_compat_result(status, *errnop);
	if (status == NS_SUCCESS)
		*(struct group **)retval = grp;
	return (status);
}


int
__nss_compat_getgrgid_r(void *retval, void *mdata, va_list ap)
{
	int (*fn)(gid_t, struct group *, char *, size_t, int *);
	gid_t		 gid;
	struct group	*grp;
	char		*buffer;
	int		*errnop;
	size_t		 bufsize;
	enum nss_status	 status;
	
	fn = mdata;
	gid = va_arg(ap, gid_t);
	grp = va_arg(ap, struct group *);
	buffer = va_arg(ap, char *);
	bufsize = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);
	status = fn(gid, grp, buffer, bufsize, errnop);
	status = __nss_compat_result(status, *errnop);
	if (status == NS_SUCCESS)
		*(struct group **)retval = grp;
	return (status);
}


int
__nss_compat_getgrent_r(void *retval, void *mdata, va_list ap)
{
	int (*fn)(struct group *, char *, size_t, int *);
	struct group	*grp;
	char		*buffer;
	int		*errnop;
	size_t		 bufsize;
	enum nss_status	 status;

	if (CHECK_TERMINATOR(group))
		return (NS_NOTFOUND);
	fn = mdata;
	grp = va_arg(ap, struct group *);
	buffer = va_arg(ap, char *);
	bufsize = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);
	status = fn(grp, buffer, bufsize, errnop);
	status = __nss_compat_result(status, *errnop);
	if (status == NS_SUCCESS)
		*(struct group **)retval = grp;
	else if (status != NS_RETURN)
		SET_TERMINATOR(group, &terminator);
	return (status);
}


int
__nss_compat_setgrent(void *retval, void *mdata, va_list ap)
{

	SET_TERMINATOR(group, NULL);
	((int (*)(void))mdata)();
	return (NS_UNAVAIL);
}


int
__nss_compat_endgrent(void *retval, void *mdata, va_list ap)
{

	SET_TERMINATOR(group, NULL);
	((int (*)(void))mdata)();
	return (NS_UNAVAIL);
}



DECLARE_TERMINATOR(passwd);


int
__nss_compat_getpwnam_r(void *retval, void *mdata, va_list ap)
{
	int (*fn)(const char *, struct passwd *, char *, size_t, int *);
	const char	*name;
	struct passwd	*pwd;
	char		*buffer;
	int		*errnop;
	size_t		 bufsize;
	enum nss_status	 status;

	fn = mdata;
	name = va_arg(ap, const char *);
	pwd = va_arg(ap, struct passwd *);
	buffer = va_arg(ap, char *);
	bufsize = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);
	status = fn(name, pwd, buffer, bufsize, errnop);
	status = __nss_compat_result(status, *errnop);
	if (status == NS_SUCCESS)
		*(struct passwd **)retval = pwd;
	return (status);
}


int
__nss_compat_getpwuid_r(void *retval, void *mdata, va_list ap)
{
	int (*fn)(uid_t, struct passwd *, char *, size_t, int *);
	uid_t		 uid;
	struct passwd	*pwd;
	char		*buffer;
	int		*errnop;
	size_t		 bufsize;
	enum nss_status	 status;
	
	fn = mdata;
	uid = va_arg(ap, uid_t);
	pwd = va_arg(ap, struct passwd *);
	buffer = va_arg(ap, char *);
	bufsize = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);
	status = fn(uid, pwd, buffer, bufsize, errnop);
	status = __nss_compat_result(status, *errnop);
	if (status == NS_SUCCESS)
		*(struct passwd **)retval = pwd;
	return (status);
}


int
__nss_compat_getpwent_r(void *retval, void *mdata, va_list ap)
{
	int (*fn)(struct passwd *, char *, size_t, int *);
	struct passwd	*pwd;
	char		*buffer;
	int		*errnop;
	size_t		 bufsize;
	enum nss_status	 status;

	if (CHECK_TERMINATOR(passwd))
		return (NS_NOTFOUND);
	fn = mdata;
	pwd = va_arg(ap, struct passwd *);
	buffer = va_arg(ap, char *);
	bufsize = va_arg(ap, size_t);
	errnop = va_arg(ap, int *);
	status = fn(pwd, buffer, bufsize, errnop);
	status = __nss_compat_result(status, *errnop);
	if (status == NS_SUCCESS)
		*(struct passwd **)retval = pwd;
	else if (status != NS_RETURN)
		SET_TERMINATOR(passwd, &terminator);
	return (status);
}


int
__nss_compat_setpwent(void *retval, void *mdata, va_list ap)
{

	SET_TERMINATOR(passwd, NULL);
	((int (*)(void))mdata)();
	return (NS_UNAVAIL);
}


int
__nss_compat_endpwent(void *retval, void *mdata, va_list ap)
{

	SET_TERMINATOR(passwd, NULL);
	((int (*)(void))mdata)();
	return (NS_UNAVAIL);
}