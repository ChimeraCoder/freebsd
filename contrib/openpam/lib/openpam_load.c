
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

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * Locate a matching dynamic or static module.
 */

pam_module_t *
openpam_load_module(const char *path)
{
	pam_module_t *module;

	module = openpam_dynamic(path);
	openpam_log(PAM_LOG_DEBUG, "%s dynamic %s",
	    (module == NULL) ? "no" : "using", path);

#ifdef OPENPAM_STATIC_MODULES
	/* look for a static module */
	if (module == NULL && strchr(path, '/') == NULL) {
		module = openpam_static(path);
		openpam_log(PAM_LOG_DEBUG, "%s static %s",
		    (module == NULL) ? "no" : "using", path);
	}
#endif
	if (module == NULL) {
		openpam_log(PAM_LOG_ERROR, "no %s found", path);
		return (NULL);
	}
	return (module);
}


/*
 * Release a module.
 * XXX highly thread-unsafe
 */

static void
openpam_release_module(pam_module_t *module)
{
	if (module == NULL)
		return;
	if (module->dlh == NULL)
		/* static module */
		return;
	dlclose(module->dlh);
	openpam_log(PAM_LOG_DEBUG, "releasing %s", module->path);
	FREE(module->path);
	FREE(module);
}


/*
 * Destroy a chain, freeing all its links and releasing the modules
 * they point to.
 */

static void
openpam_destroy_chain(pam_chain_t *chain)
{
	if (chain == NULL)
		return;
	openpam_destroy_chain(chain->next);
	chain->next = NULL;
	FREEV(chain->optc, chain->optv);
	openpam_release_module(chain->module);
	chain->module = NULL;
	FREE(chain);
}


/*
 * Clear the chains and release the modules
 */

void
openpam_clear_chains(pam_chain_t *policy[])
{
	int i;

	for (i = 0; i < PAM_NUM_FACILITIES; ++i) {
		openpam_destroy_chain(policy[i]);
		policy[i] = NULL;
	}
}

/*
 * NOPARSE
 */