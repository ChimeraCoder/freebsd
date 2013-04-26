
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

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "auditd.h"

/*
 * Write an audit-related error to the system log via syslog(3).
 */
static int
auditwarnlog(char *args[])
{
	char *loc_args[9];
	pid_t pid;
	int i;

	loc_args[0] = AUDITWARN_SCRIPT;
	for (i = 0; args[i] != NULL && i < 8; i++)
		loc_args[i+1] = args[i];
	loc_args[i+1] = NULL;

	pid = fork();
	if (pid == -1)
		return (-1);
	if (pid == 0) {
		/*
		 * Child.
		 */
		execv(AUDITWARN_SCRIPT, loc_args);
		syslog(LOG_ERR, "Could not exec %s (%m)\n",
		    AUDITWARN_SCRIPT);
		exit(1);
	}
	/*
	 * Parent.
	 */
	return (0);
}

/*
 * Indicates that the hard limit for all filesystems has been exceeded.
 */
int
audit_warn_allhard(void)
{
	char *args[2];

	args[0] = HARDLIM_ALL_WARN;
	args[1] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that the soft limit for all filesystems has been exceeded.
 */
int
audit_warn_allsoft(void)
{
	char *args[2];

	args[0] = SOFTLIM_ALL_WARN;
	args[1] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that someone other than the audit daemon turned off auditing.
 * XXX Its not clear at this point how this function will be invoked.
 *
 * XXXRW: This function is not used.
 */
int
audit_warn_auditoff(void)
{
	char *args[2];

	args[0] = AUDITOFF_WARN;
	args[1] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicate that a trail file has been closed, so can now be post-processed.
 */
int
audit_warn_closefile(char *filename)
{
	char *args[3];

	args[0] = CLOSEFILE_WARN;
	args[1] = filename;
	args[2] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that the audit deammn is already running
 */
int
audit_warn_ebusy(void)
{
	char *args[2];

	args[0] = EBUSY_WARN;
	args[1] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that there is a problem getting the directory from
 * audit_control.
 *
 * XXX Note that we take the filename instead of a count as the argument here
 * (different from BSM).
 */
int
audit_warn_getacdir(char *filename)
{
	char *args[3];

	args[0] = GETACDIR_WARN;
	args[1] = filename;
	args[2] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that the hard limit for this file has been exceeded.
 */
int
audit_warn_hard(char *filename)
{
	char *args[3];

	args[0] = HARDLIM_WARN;
	args[1] = filename;
	args[2] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that auditing could not be started.
 */
int
audit_warn_nostart(void)
{
	char *args[2];

	args[0] = NOSTART_WARN;
	args[1] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicaes that an error occrred during the orderly shutdown of the audit
 * daemon.
 */
int
audit_warn_postsigterm(void)
{
	char *args[2];

	args[0] = POSTSIGTERM_WARN;
	args[1] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that the soft limit for this file has been exceeded.
 */
int
audit_warn_soft(char *filename)
{
	char *args[3];

	args[0] = SOFTLIM_WARN;
	args[1] = filename;
	args[2] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that the temporary audit file already exists indicating a fatal
 * error.
 */
int
audit_warn_tmpfile(void)
{
	char *args[2];

	args[0] = TMPFILE_WARN;
	args[1] = NULL;

	return (auditwarnlog(args));
}

/*
 * Indicates that this trail file has expired and was removed.
 */
int
audit_warn_expired(char *filename)
{
	char *args[3];

	args[0] = EXPIRED_WARN;
	args[1] = filename;
	args[2] = NULL;

	return (auditwarnlog(args));
}