
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

#include <config.h>

#include "roken.h"

ROKEN_LIB_FUNCTION char * ROKEN_LIB_CALL
pid_file_write (const char *progname)
{
    char *ret = NULL;
    FILE *fp;

    if (asprintf (&ret, "%s%s.pid", _PATH_VARRUN, progname) < 0 || ret == NULL)
	return NULL;
    fp = fopen (ret, "w");
    if (fp == NULL) {
	free (ret);
	return NULL;
    }
    fprintf (fp, "%u", (unsigned)getpid());
    fclose (fp);
    return ret;
}

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
pid_file_delete (char **filename)
{
    if (*filename != NULL) {
	unlink (*filename);
	free (*filename);
	*filename = NULL;
    }
}

#ifndef HAVE_PIDFILE
static char *pidfile_path;

static void
pidfile_cleanup(void)
{
    if(pidfile_path != NULL)
	pid_file_delete(&pidfile_path);
}

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
pidfile(const char *basename)
{
    if(pidfile_path != NULL)
	return;
    if(basename == NULL)
	basename = getprogname();
    pidfile_path = pid_file_write(basename);
#if defined(HAVE_ATEXIT)
    atexit(pidfile_cleanup);
#elif defined(HAVE_ON_EXIT)
    on_exit(pidfile_cleanup);
#endif
}
#endif