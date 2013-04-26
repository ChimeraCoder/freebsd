
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

#include "kcm_locl.h"

#if 0
#include <bsm/audit_session.h>
#endif

void
kcm_session_add(pid_t session_id)
{
    kcm_log(1, "monitor session: %d\n", session_id);
}

void
kcm_session_setup_handler(void)
{
#if 0
    au_sdev_handle_t *h;
    dispatch_queue_t bgq;

    h = au_sdev_open(AU_SDEVF_ALLSESSIONS);
    if (h == NULL)
	return;

    bgq = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);

    dispatch_async(bgq, ^{
	    for (;;) {
		auditinfo_addr_t aio;
		int event;

		if (au_sdev_read_aia(h, &event, &aio) != 0)
		    continue;

		/*
		 * Ignore everything but END. This should relly be
		 * CLOSE but since that is delayed until the credential
		 * is reused, we can't do that
		 * */
		if (event != AUE_SESSION_END)
		    continue;

		dispatch_async(dispatch_get_main_queue(), ^{
			kcm_cache_remove_session(aio.ai_asid);
		    });
	    }
	});
#endif
}