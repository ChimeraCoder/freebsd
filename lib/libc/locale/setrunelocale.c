
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

#define __RUNETYPE_INTERNAL 1

#include <runetype.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include "ldpart.h"
#include "mblocal.h"
#include "setlocale.h"

#undef _CurrentRuneLocale
extern _RuneLocale const *_CurrentRuneLocale;
#ifndef __NO_TLS
/*
 * A cached version of the runes for this thread.  Used by ctype.h
 */
_Thread_local const _RuneLocale *_ThreadRuneLocale;
#endif

extern int __mb_sb_limit;

extern _RuneLocale	*_Read_RuneMagi(FILE *);

static int		__setrunelocale(struct xlocale_ctype *l, const char *);

#define __collate_substitute_nontrivial (table->__collate_substitute_nontrivial)
#define __collate_substitute_table_ptr (table->__collate_substitute_table_ptr)
#define __collate_char_pri_table_ptr (table->__collate_char_pri_table_ptr)
#define __collate_chain_pri_table (table->__collate_chain_pri_table)


static void
destruct_ctype(void *v)
{
	struct xlocale_ctype *l = v;

	if (strcmp(l->runes->__encoding, "EUC") == 0)
		free(l->runes->__variable);
	if (&_DefaultRuneLocale != l->runes) 
		free(l->runes);
	free(l);
}

const _RuneLocale *
__getCurrentRuneLocale(void)
{

	return XLOCALE_CTYPE(__get_locale())->runes;
}

static void
free_runes(_RuneLocale *rl)
{

	/* FIXME: The "EUC" check here is a hideous abstraction violation. */
	if ((rl != &_DefaultRuneLocale) && (rl)) {
		if (strcmp(rl->__encoding, "EUC") == 0) {
			free(rl->__variable);
		}
		free(rl);
	}
}

static int
__setrunelocale(struct xlocale_ctype *l, const char *encoding)
{
	FILE *fp;
	char name[PATH_MAX];
	_RuneLocale *rl;
	int saverr, ret;
	struct xlocale_ctype saved = *l;

	/*
	 * The "C" and "POSIX" locale are always here.
	 */
	if (strcmp(encoding, "C") == 0 || strcmp(encoding, "POSIX") == 0) {
		free_runes(saved.runes);
		(void) _none_init(l, (_RuneLocale*)&_DefaultRuneLocale);
		return (0);
	}

	/* Range checking not needed, encoding length already checked before */
	(void) strcpy(name, _PathLocale);
	(void) strcat(name, "/");
	(void) strcat(name, encoding);
	(void) strcat(name, "/LC_CTYPE");

	if ((fp = fopen(name, "re")) == NULL)
		return (errno == 0 ? ENOENT : errno);

	if ((rl = _Read_RuneMagi(fp)) == NULL) {
		saverr = (errno == 0 ? EFTYPE : errno);
		(void)fclose(fp);
		return (saverr);
	}
	(void)fclose(fp);

	l->__mbrtowc = NULL;
	l->__mbsinit = NULL;
	l->__mbsnrtowcs = __mbsnrtowcs_std;
	l->__wcrtomb = NULL;
	l->__wcsnrtombs = __wcsnrtombs_std;

	rl->__sputrune = NULL;
	rl->__sgetrune = NULL;
	if (strcmp(rl->__encoding, "NONE") == 0)
		ret = _none_init(l, rl);
	else if (strcmp(rl->__encoding, "ASCII") == 0)
		ret = _ascii_init(l, rl);
	else if (strcmp(rl->__encoding, "UTF-8") == 0)
		ret = _UTF8_init(l, rl);
	else if (strcmp(rl->__encoding, "EUC") == 0)
		ret = _EUC_init(l, rl);
	else if (strcmp(rl->__encoding, "GB18030") == 0)
 		ret = _GB18030_init(l, rl);
	else if (strcmp(rl->__encoding, "GB2312") == 0)
		ret = _GB2312_init(l, rl);
	else if (strcmp(rl->__encoding, "GBK") == 0)
		ret = _GBK_init(l, rl);
	else if (strcmp(rl->__encoding, "BIG5") == 0)
		ret = _BIG5_init(l, rl);
	else if (strcmp(rl->__encoding, "MSKanji") == 0)
		ret = _MSKanji_init(l, rl);
	else
		ret = EFTYPE;

	if (ret == 0) {
		/* Free the old runes if it exists. */
		free_runes(saved.runes);
	} else {
		/* Restore the saved version if this failed. */
		memcpy(l, &saved, sizeof(struct xlocale_ctype));
		free(rl);
	}

	return (ret);
}

int
__wrap_setrunelocale(const char *locale)
{
	int ret = __setrunelocale(&__xlocale_global_ctype, locale);

	if (ret != 0) {
		errno = ret;
		return (_LDP_ERROR);
	}
	__mb_cur_max = __xlocale_global_ctype.__mb_cur_max;
	__mb_sb_limit = __xlocale_global_ctype.__mb_sb_limit;
	_CurrentRuneLocale = __xlocale_global_ctype.runes;
	return (_LDP_LOADED);
}

#ifndef __NO_TLS
void
__set_thread_rune_locale(locale_t loc)
{

	if (loc == NULL) {
		_ThreadRuneLocale = &_DefaultRuneLocale;
	} else {
		_ThreadRuneLocale = XLOCALE_CTYPE(loc)->runes;
	}
}
#endif

void *
__ctype_load(const char *locale, locale_t unused)
{
	struct xlocale_ctype *l = calloc(sizeof(struct xlocale_ctype), 1);

	l->header.header.destructor = destruct_ctype;
	if (__setrunelocale(l, locale))
	{
		free(l);
		return NULL;
	}
	return l;
}