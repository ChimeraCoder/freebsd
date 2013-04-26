
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

#include <sys/param.h>

#include <stdlib.h>
#include <string.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * XSSO 4.2.1
 * XSSO 6 page 60
 *
 * Set authentication information
 */

int
pam_set_item(pam_handle_t *pamh,
	int item_type,
	const void *item)
{
	void **slot, *tmp;
	size_t nsize, osize;

	ENTERI(item_type);
	if (pamh == NULL)
		RETURNC(PAM_SYSTEM_ERR);
	slot = &pamh->item[item_type];
	osize = nsize = 0;
	switch (item_type) {
	case PAM_SERVICE:
		/* set once only, by pam_start() */
		if (*slot != NULL)
			RETURNC(PAM_SYSTEM_ERR);
		/* fall through */
	case PAM_USER:
	case PAM_AUTHTOK:
	case PAM_OLDAUTHTOK:
	case PAM_TTY:
	case PAM_RHOST:
	case PAM_RUSER:
	case PAM_USER_PROMPT:
	case PAM_AUTHTOK_PROMPT:
	case PAM_OLDAUTHTOK_PROMPT:
	case PAM_HOST:
		if (*slot != NULL)
			osize = strlen(*slot) + 1;
		if (item != NULL)
			nsize = strlen(item) + 1;
		break;
	case PAM_REPOSITORY:
		osize = nsize = sizeof(struct pam_repository);
		break;
	case PAM_CONV:
		osize = nsize = sizeof(struct pam_conv);
		break;
	default:
		RETURNC(PAM_SYMBOL_ERR);
	}
	if (*slot != NULL) {
		memset(*slot, 0xd0, osize);
		FREE(*slot);
	}
	if (item != NULL) {
		if ((tmp = malloc(nsize)) == NULL)
			RETURNC(PAM_BUF_ERR);
		memcpy(tmp, item, nsize);
	} else {
		tmp = NULL;
	}
	*slot = tmp;
	RETURNC(PAM_SUCCESS);
}

/*
 * Error codes:
 *
 *	PAM_SYMBOL_ERR
 *	PAM_SYSTEM_ERR
 *	PAM_BUF_ERR
 */

/**
 * The =pam_set_item function sets the item specified by the =item_type
 * argument to a copy of the object pointed to by the =item argument.
 * The item is stored in the PAM context specified by the =pamh argument.
 * See =pam_get_item for a list of recognized item types.
 */