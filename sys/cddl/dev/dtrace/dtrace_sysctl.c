
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

int	dtrace_debug = 0;
TUNABLE_INT("debug.dtrace.debug", &dtrace_debug);
SYSCTL_INT(_debug_dtrace, OID_AUTO, debug, CTLFLAG_RW, &dtrace_debug, 0, "");

/* Report registered DTrace providers. */
static int
sysctl_dtrace_providers(SYSCTL_HANDLER_ARGS)
{
	char	*p_name	= NULL;
	dtrace_provider_t
		*prov	= dtrace_provider;
	int	error	= 0;
	size_t	len	= 0;

	mutex_enter(&dtrace_provider_lock);
	mutex_enter(&dtrace_lock);

	/* Compute the length of the space-separated provider name string. */
	while (prov != NULL) {
		len += strlen(prov->dtpv_name) + 1;
		prov = prov->dtpv_next;
	}

	if ((p_name = kmem_alloc(len, KM_SLEEP)) == NULL)
		error = ENOMEM;
	else {
		/* Start with an empty string. */
		*p_name = '\0';

		/* Point to the first provider again. */
		prov = dtrace_provider;

		/* Loop through the providers, appending the names. */
		while (prov != NULL) {
			if (prov != dtrace_provider)
				(void) strlcat(p_name, " ", len);

			(void) strlcat(p_name, prov->dtpv_name, len);

			prov = prov->dtpv_next;
		}
	}

	mutex_exit(&dtrace_lock);
	mutex_exit(&dtrace_provider_lock);

	if (p_name != NULL) {
		error = sysctl_handle_string(oidp, p_name, len, req);

		kmem_free(p_name, 0);
	}

	return (error);
}

SYSCTL_PROC(_debug_dtrace, OID_AUTO, providers, CTLTYPE_STRING | CTLFLAG_RD,
    0, 0, sysctl_dtrace_providers, "A", "");