
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <security/pam_appl.h>

#include "openpam_impl.h"

#define subst_char(ch) do {			\
	int ch_ = (ch);				\
	if (buf && len < *bufsize)		\
		*buf++ = ch_;			\
	++len;					\
} while (0)

#define subst_string(s) do {			\
	const char *s_ = (s);			\
	while (*s_)				\
		subst_char(*s_++);		\
} while (0)

#define subst_item(i) do {			\
	int i_ = (i);				\
	const void *p_;				\
	ret = pam_get_item(pamh, i_, &p_);	\
	if (ret == PAM_SUCCESS && p_ != NULL)	\
		subst_string(p_);		\
} while (0)

/*
 * OpenPAM internal
 *
 * Substitute PAM item values in a string
 */

int
openpam_subst(const pam_handle_t *pamh,
    char *buf, size_t *bufsize, const char *template)
{
	size_t len;
	int ret;

	ENTERS(template);
	if (template == NULL)
		template = "(null)";

	len = 1; /* initialize to 1 for terminating NUL */
	ret = PAM_SUCCESS;
	while (*template && ret == PAM_SUCCESS) {
		if (template[0] == '%') {
			++template;
			switch (*template) {
			case 's':
				subst_item(PAM_SERVICE);
				break;
			case 't':
				subst_item(PAM_TTY);
				break;
			case 'h':
				subst_item(PAM_HOST);
				break;
			case 'u':
				subst_item(PAM_USER);
				break;
			case 'H':
				subst_item(PAM_RHOST);
				break;
			case 'U':
				subst_item(PAM_RUSER);
				break;
			case '\0':
				subst_char('%');
				break;
			default:
				subst_char('%');
				subst_char(*template);
			}
			++template;
		} else {
			subst_char(*template++);
		}
	}
	if (buf)
		*buf = '\0';
	if (ret == PAM_SUCCESS) {
		if (len > *bufsize)
			ret = PAM_TRY_AGAIN;
		*bufsize = len;
	}
	RETURNC(ret);
}

/*
 * Error codes:
 *
 *	=pam_get_item
 *	!PAM_SYMBOL_ERR
 *	PAM_TRY_AGAIN
 */

/**
 * The =openpam_subst function expands a string, substituting PAM item
 * values for all occurrences of specific substitution codes.
 * The =template argument points to the initial string.
 * The result is stored in the buffer pointed to by the =buf argument; the
 * =bufsize argument specifies the size of that buffer.
 * The actual size of the resulting string, including the terminating NUL
 * character, is stored in the location pointed to by the =bufsize
 * argument.
 *
 * If =buf is NULL, or if the buffer is too small to hold the expanded
 * string, =bufsize is updated to reflect the amount of space required to
 * hold the entire string, and =openpam_subst returns =PAM_TRY_AGAIN.
 *
 * If =openpam_subst fails for any other reason, the =bufsize argument is
 * untouched, but part of the buffer may still have been overwritten.
 *
 * Substitution codes are introduced by a percent character and correspond
 * to PAM items:
 *
 *	%H:
 *		Replaced by the current value of the =PAM_RHOST item.
 *	%h:
 *		Replaced by the current value of the =PAM_HOST item.
 *	%s:
 *		Replaced by the current value of the =PAM_SERVICE item.
 *	%t:
 *		Replaced by the current value of the =PAM_TTY item.
 *	%U:
 *		Replaced by the current value of the =PAM_RUSER item.
 *	%u:
 *		Replaced by the current value of the =PAM_USER item.
 *
 * >pam_get_authtok
 * >pam_get_item
 * >pam_get_user
 *
 * AUTHOR DES
 */