
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

#include <stddef.h>

#include "ldpart.h"
#include "lmessages.h"

#define LCMESSAGES_SIZE_FULL (sizeof(struct lc_messages_T) / sizeof(char *))
#define LCMESSAGES_SIZE_MIN \
		(offsetof(struct lc_messages_T, yesstr) / sizeof(char *))

struct xlocale_messages {
	struct xlocale_component header;
	char *buffer;
	struct lc_messages_T locale;
};

struct xlocale_messages __xlocale_global_messages;

static char empty[] = "";

static const struct lc_messages_T _C_messages_locale = {
	"^[yY]" ,	/* yesexpr */
	"^[nN]" ,	/* noexpr */
	"yes" , 	/* yesstr */
	"no"		/* nostr */
};

static void destruct_messages(void *v)
{
	struct xlocale_messages *l = v;
	if (l->buffer)
		free(l->buffer);
	free(l);
}

static int
messages_load_locale(struct xlocale_messages *loc, int *using_locale, const char *name)
{
	int ret;
	struct lc_messages_T *l = &loc->locale;

	ret = __part_load_locale(name, using_locale,
		  &loc->buffer, "LC_MESSAGES",
		  LCMESSAGES_SIZE_FULL, LCMESSAGES_SIZE_MIN,
		  (const char **)l);
	if (ret == _LDP_LOADED) {
		if (l->yesstr == NULL)
			l->yesstr = empty;
		if (l->nostr == NULL)
			l->nostr = empty;
	}
	return (ret);
}
int
__messages_load_locale(const char *name)
{
	return messages_load_locale(&__xlocale_global_messages,
			&__xlocale_global_locale.using_messages_locale, name);
}
void *
__messages_load(const char *name, locale_t l)
{
	struct xlocale_messages *new = calloc(sizeof(struct xlocale_messages), 1);
	new->header.header.destructor = destruct_messages;
	if (messages_load_locale(new, &l->using_messages_locale, name) == _LDP_ERROR) {
		xlocale_release(new);
		return NULL;
	}
	return new;
}

struct lc_messages_T *
__get_current_messages_locale(locale_t loc)
{
	return (loc->using_messages_locale
		? &((struct xlocale_messages *)loc->components[XLC_MESSAGES])->locale
		: (struct lc_messages_T *)&_C_messages_locale);
}

#ifdef LOCALE_DEBUG
void
msgdebug() {
printf(	"yesexpr = %s\n"
	"noexpr = %s\n"
	"yesstr = %s\n"
	"nostr = %s\n",
	_messages_locale.yesexpr,
	_messages_locale.noexpr,
	_messages_locale.yesstr,
	_messages_locale.nostr
);
}
#endif /* LOCALE_DEBUG */