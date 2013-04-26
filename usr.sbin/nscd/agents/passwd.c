
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

#include <sys/types.h>

#include <assert.h>
#include <nsswitch.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"
#include "passwd.h"

static int passwd_marshal_func(struct passwd *, char *, size_t *);
static int passwd_lookup_func(const char *, size_t, char **, size_t *);
static void *passwd_mp_init_func(void);
static int passwd_mp_lookup_func(char **, size_t *, void *);
static void passwd_mp_destroy_func(void *mdata);

static int
passwd_marshal_func(struct passwd *pwd, char *buffer, size_t *buffer_size)
{
	char		*p;
	struct passwd	new_pwd;
	size_t		desired_size, size;

	TRACE_IN(passwd_marshal_func);
	desired_size = sizeof(struct passwd) + sizeof(char *) +
		strlen(pwd->pw_name) + 1;
	if (pwd->pw_passwd != NULL)
		desired_size += strlen(pwd->pw_passwd) + 1;
	if (pwd->pw_class != NULL)
		desired_size += strlen(pwd->pw_class) + 1;
	if (pwd->pw_gecos != NULL)
		desired_size += strlen(pwd->pw_gecos) + 1;
	if (pwd->pw_dir != NULL)
		desired_size += strlen(pwd->pw_dir) + 1;
	if (pwd->pw_shell != NULL)
		desired_size += strlen(pwd->pw_shell) + 1;

	if ((*buffer_size < desired_size) || (buffer == NULL)) {
		*buffer_size = desired_size;
		TRACE_OUT(passwd_marshal_func);
		return (NS_RETURN);
	}

	memcpy(&new_pwd, pwd, sizeof(struct passwd));
	memset(buffer, 0, desired_size);

	*buffer_size = desired_size;
	p = buffer + sizeof(struct passwd) + sizeof(char *);
	memcpy(buffer + sizeof(struct passwd), &p, sizeof(char *));

	if (new_pwd.pw_name != NULL) {
		size = strlen(new_pwd.pw_name);
		memcpy(p, new_pwd.pw_name, size);
		new_pwd.pw_name = p;
		p += size + 1;
	}

	if (new_pwd.pw_passwd != NULL) {
		size = strlen(new_pwd.pw_passwd);
		memcpy(p, new_pwd.pw_passwd, size);
		new_pwd.pw_passwd = p;
		p += size + 1;
	}

	if (new_pwd.pw_class != NULL) {
		size = strlen(new_pwd.pw_class);
		memcpy(p, new_pwd.pw_class, size);
		new_pwd.pw_class = p;
		p += size + 1;
	}

	if (new_pwd.pw_gecos != NULL) {
		size = strlen(new_pwd.pw_gecos);
		memcpy(p, new_pwd.pw_gecos, size);
		new_pwd.pw_gecos = p;
		p += size + 1;
	}

	if (new_pwd.pw_dir != NULL) {
		size = strlen(new_pwd.pw_dir);
		memcpy(p, new_pwd.pw_dir, size);
		new_pwd.pw_dir = p;
		p += size + 1;
	}

	if (new_pwd.pw_shell != NULL) {
		size = strlen(new_pwd.pw_shell);
		memcpy(p, new_pwd.pw_shell, size);
		new_pwd.pw_shell = p;
		p += size + 1;
	}

	memcpy(buffer, &new_pwd, sizeof(struct passwd));
	TRACE_OUT(passwd_marshal_func);
	return (NS_SUCCESS);
}

static int
passwd_lookup_func(const char *key, size_t key_size, char **buffer,
	size_t *buffer_size)
{
	enum nss_lookup_type lookup_type;
	char	*login;
	size_t	size;
	uid_t	uid;

	struct passwd *result;

	TRACE_IN(passwd_lookup_func);
	assert(buffer != NULL);
	assert(buffer_size != NULL);

	if (key_size < sizeof(enum nss_lookup_type)) {
		TRACE_OUT(passwd_lookup_func);
		return (NS_UNAVAIL);
	}
	memcpy(&lookup_type, key, sizeof(enum nss_lookup_type));

	switch (lookup_type) {
	case nss_lt_name:
		size = key_size - sizeof(enum nss_lookup_type)	+ 1;
		login = calloc(1, size);
		assert(login != NULL);
		memcpy(login, key + sizeof(enum nss_lookup_type), size - 1);
		break;
	case nss_lt_id:
		if (key_size < sizeof(enum nss_lookup_type) +
			sizeof(uid_t)) {
			TRACE_OUT(passwd_lookup_func);
			return (NS_UNAVAIL);
		}

		memcpy(&uid, key + sizeof(enum nss_lookup_type), sizeof(uid_t));
		break;
	default:
		TRACE_OUT(passwd_lookup_func);
		return (NS_UNAVAIL);
	}

	switch (lookup_type) {
	case nss_lt_name:
		result = getpwnam(login);
		free(login);
		break;
	case nss_lt_id:
		result = getpwuid(uid);
		break;
	default:
		/* SHOULD NOT BE REACHED */
		break;
	}

	if (result != NULL) {
		passwd_marshal_func(result, NULL, buffer_size);
		*buffer = malloc(*buffer_size);
		assert(*buffer != NULL);
		passwd_marshal_func(result, *buffer, buffer_size);
	}

	TRACE_OUT(passwd_lookup_func);
	return (result == NULL ? NS_NOTFOUND : NS_SUCCESS);
}

static void *
passwd_mp_init_func(void)
{
	TRACE_IN(passwd_mp_init_func);
	setpwent();
	TRACE_OUT(passwd_mp_init_func);

	return (NULL);
}

static int
passwd_mp_lookup_func(char **buffer, size_t *buffer_size, void *mdata)
{
	struct passwd	*result;

	TRACE_IN(passwd_mp_lookup_func);
	result = getpwent();
	if (result != NULL) {
		passwd_marshal_func(result, NULL, buffer_size);
		*buffer = malloc(*buffer_size);
		assert(*buffer != NULL);
		passwd_marshal_func(result, *buffer, buffer_size);
	}

	TRACE_OUT(passwd_mp_lookup_func);
	return (result == NULL ? NS_NOTFOUND : NS_SUCCESS);
}

static void
passwd_mp_destroy_func(void *mdata)
{
	TRACE_IN(passwd_mp_destroy_func);
	TRACE_OUT(passwd_mp_destroy_func);
}

struct agent *
init_passwd_agent(void)
{
	struct common_agent	*retval;

	TRACE_IN(init_passwd_agent);
	retval = calloc(1, sizeof(*retval));
	assert(retval != NULL);

	retval->parent.name = strdup("passwd");
	assert(retval->parent.name != NULL);

	retval->parent.type = COMMON_AGENT;
	retval->lookup_func = passwd_lookup_func;

	TRACE_OUT(init_passwd_agent);
	return ((struct agent *)retval);
}

struct agent *
init_passwd_mp_agent(void)
{
	struct multipart_agent	*retval;

	TRACE_IN(init_passwd_mp_agent);
	retval = calloc(1,
		sizeof(*retval));
	assert(retval != NULL);

	retval->parent.name = strdup("passwd");
	retval->parent.type = MULTIPART_AGENT;
	retval->mp_init_func = passwd_mp_init_func;
	retval->mp_lookup_func = passwd_mp_lookup_func;
	retval->mp_destroy_func = passwd_mp_destroy_func;
	assert(retval->parent.name != NULL);

	TRACE_OUT(init_passwd_mp_agent);
	return ((struct agent *)retval);
}