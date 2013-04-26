
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

#include <security/pam_appl.h>

#include "openpam_impl.h"

#if !defined(OPENPAM_RELAX_CHECKS)
static void openpam_check_error_code(int, int);
#else
#define openpam_check_error_code(a, b)
#endif /* !defined(OPENPAM_RELAX_CHECKS) */

/*
 * OpenPAM internal
 *
 * Execute a module chain
 */

int
openpam_dispatch(pam_handle_t *pamh,
	int primitive,
	int flags)
{
	pam_chain_t *chain;
	int err, fail, r;
	int debug;

	ENTER();
	if (pamh == NULL)
		RETURNC(PAM_SYSTEM_ERR);

	/* prevent recursion */
	if (pamh->current != NULL) {
		openpam_log(PAM_LOG_ERROR,
		    "%s() called while %s::%s() is in progress",
		    pam_func_name[primitive],
		    pamh->current->module->path,
		    pam_sm_func_name[pamh->primitive]);
		RETURNC(PAM_ABORT);
	}

	/* pick a chain */
	switch (primitive) {
	case PAM_SM_AUTHENTICATE:
	case PAM_SM_SETCRED:
		chain = pamh->chains[PAM_AUTH];
		break;
	case PAM_SM_ACCT_MGMT:
		chain = pamh->chains[PAM_ACCOUNT];
		break;
	case PAM_SM_OPEN_SESSION:
	case PAM_SM_CLOSE_SESSION:
		chain = pamh->chains[PAM_SESSION];
		break;
	case PAM_SM_CHAUTHTOK:
		chain = pamh->chains[PAM_PASSWORD];
		break;
	default:
		RETURNC(PAM_SYSTEM_ERR);
	}

	/* execute */
	for (err = fail = 0; chain != NULL; chain = chain->next) {
		if (chain->module->func[primitive] == NULL) {
			openpam_log(PAM_LOG_ERROR, "%s: no %s()",
			    chain->module->path, pam_sm_func_name[primitive]);
			r = PAM_SYSTEM_ERR;
		} else {
			pamh->primitive = primitive;
			pamh->current = chain;
			debug = (openpam_get_option(pamh, "debug") != NULL);
			if (debug)
				++openpam_debug;
			openpam_log(PAM_LOG_DEBUG, "calling %s() in %s",
			    pam_sm_func_name[primitive], chain->module->path);
			r = (chain->module->func[primitive])(pamh, flags,
			    chain->optc, (const char **)chain->optv);
			pamh->current = NULL;
			openpam_log(PAM_LOG_DEBUG, "%s: %s(): %s",
			    chain->module->path, pam_sm_func_name[primitive],
			    pam_strerror(pamh, r));
			if (debug)
				--openpam_debug;
		}

		if (r == PAM_IGNORE)
			continue;
		if (r == PAM_SUCCESS) {
			/*
			 * For pam_setcred() and pam_chauthtok() with the
			 * PAM_PRELIM_CHECK flag, treat "sufficient" as
			 * "optional".
			 */
			if ((chain->flag == PAM_SUFFICIENT ||
			    chain->flag == PAM_BINDING) && !fail &&
			    primitive != PAM_SM_SETCRED &&
			    !(primitive == PAM_SM_CHAUTHTOK &&
				(flags & PAM_PRELIM_CHECK)))
				break;
			continue;
		}

		openpam_check_error_code(primitive, r);

		/*
		 * Record the return code from the first module to
		 * fail.  If a required module fails, record the
		 * return code from the first required module to fail.
		 */
		if (err == 0)
			err = r;
		if ((chain->flag == PAM_REQUIRED ||
		    chain->flag == PAM_BINDING) && !fail) {
			openpam_log(PAM_LOG_DEBUG, "required module failed");
			fail = 1;
			err = r;
		}

		/*
		 * If a requisite module fails, terminate the chain
		 * immediately.
		 */
		if (chain->flag == PAM_REQUISITE) {
			openpam_log(PAM_LOG_DEBUG, "requisite module failed");
			fail = 1;
			break;
		}
	}

	if (!fail && err != PAM_NEW_AUTHTOK_REQD)
		err = PAM_SUCCESS;
	RETURNC(err);
}

#if !defined(OPENPAM_RELAX_CHECKS)
static void
openpam_check_error_code(int primitive, int r)
{
	/* common error codes */
	if (r == PAM_SUCCESS ||
	    r == PAM_SERVICE_ERR ||
	    r == PAM_BUF_ERR ||
	    r == PAM_CONV_ERR ||
	    r == PAM_PERM_DENIED ||
	    r == PAM_ABORT)
		return;

	/* specific error codes */
	switch (primitive) {
	case PAM_SM_AUTHENTICATE:
		if (r == PAM_AUTH_ERR ||
		    r == PAM_CRED_INSUFFICIENT ||
		    r == PAM_AUTHINFO_UNAVAIL ||
		    r == PAM_USER_UNKNOWN ||
		    r == PAM_MAXTRIES)
			return;
		break;
	case PAM_SM_SETCRED:
		if (r == PAM_CRED_UNAVAIL ||
		    r == PAM_CRED_EXPIRED ||
		    r == PAM_USER_UNKNOWN ||
		    r == PAM_CRED_ERR)
			return;
		break;
	case PAM_SM_ACCT_MGMT:
		if (r == PAM_USER_UNKNOWN ||
		    r == PAM_AUTH_ERR ||
		    r == PAM_NEW_AUTHTOK_REQD ||
		    r == PAM_ACCT_EXPIRED)
			return;
		break;
	case PAM_SM_OPEN_SESSION:
	case PAM_SM_CLOSE_SESSION:
		if (r == PAM_SESSION_ERR)
			return;
		break;
	case PAM_SM_CHAUTHTOK:
		if (r == PAM_PERM_DENIED ||
		    r == PAM_AUTHTOK_ERR ||
		    r == PAM_AUTHTOK_RECOVERY_ERR ||
		    r == PAM_AUTHTOK_LOCK_BUSY ||
		    r == PAM_AUTHTOK_DISABLE_AGING ||
		    r == PAM_TRY_AGAIN)
			return;
		break;
	}

	openpam_log(PAM_LOG_ERROR, "%s(): unexpected return value %d",
	    pam_sm_func_name[primitive], r);
}
#endif /* !defined(OPENPAM_RELAX_CHECKS) */

/*
 * NODOC
 *
 * Error codes:
 */