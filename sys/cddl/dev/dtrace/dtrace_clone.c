
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

static void
dtrace_clone(void *arg, struct ucred *cred, char *name, int namelen, struct cdev **dev)
{
	int u = -1;
	size_t len;

	if (*dev != NULL)
		return;

	len = strlen(name);

	if (len != 6 && len != 13)
		return;

	if (bcmp(name,"dtrace",6) != 0)
		return;

	if (len == 13 && bcmp(name,"dtrace/dtrace",13) != 0)
		return;

	/* Clone the device to the new minor number. */
	if (clone_create(&dtrace_clones, &dtrace_cdevsw, &u, dev, 0) != 0)
		/* Create the /dev/dtrace/dtraceNN entry. */
		*dev = make_dev_cred(&dtrace_cdevsw, u, cred,
		     UID_ROOT, GID_WHEEL, 0600, "dtrace/dtrace%d", u);
	if (*dev != NULL) {
		dev_ref(*dev);
		(*dev)->si_flags |= SI_CHEAPCLONE;
	}
}