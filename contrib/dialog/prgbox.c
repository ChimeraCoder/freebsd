
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

#include <dialog.h>

/*
 * Open a pipe which ties stderr and stdout together.
 */
static FILE *
dlg_popen(const char *command, const char *type)
{
    FILE *result = 0;
    int fd[2];
    int pid;
    const char *argv[4];

    if ((*type == 'r' || *type != 'w') && pipe(fd) == 0) {
	switch (pid = fork()) {
	case -1:		/* Error. */
	    (void) close(fd[0]);
	    (void) close(fd[1]);
	    break;
	case 0:		/* child. */
	    if (*type == 'r') {
		if (fd[1] != STDOUT_FILENO) {
		    (void) dup2(fd[1], STDOUT_FILENO);
		    (void) close(fd[1]);
		}
		(void) dup2(STDOUT_FILENO, STDERR_FILENO);
		(void) close(fd[0]);
	    } else {
		if (fd[0] != STDIN_FILENO) {
		    (void) dup2(fd[0], STDIN_FILENO);
		    (void) close(fd[0]);
		}
		(void) close(fd[1]);
		(void) close(STDERR_FILENO);
	    }
	    /*
	     * Bourne shell needs "-c" option to force it to use only the
	     * given command.  Also, it needs the command to be parsed into
	     * tokens.
	     */
	    argv[0] = "sh";
	    argv[1] = "-c";
	    argv[2] = command;
	    argv[3] = NULL;
	    execvp("sh", (char **)argv);
	    _exit(127);
	    /* NOTREACHED */
	default:		/* parent */
	    if (*type == 'r') {
		result = fdopen(fd[0], type);
		(void) close(fd[1]);
	    } else {
		result = fdopen(fd[1], type);
		(void) close(fd[0]);
	    }
	    break;
	}
    }

    return result;
}

/*
 * Display text from a pipe in a scrolling window.
 */
int
dialog_prgbox(const char *title,
	      const char *cprompt,
	      const char *command,
	      int height,
	      int width,
	      int pauseopt)
{
    int code;
    FILE *fp;

    fp = dlg_popen(command, "r");
    if (fp == NULL)
	dlg_exiterr("pipe open failed: %s", command);

    code = dlg_progressbox(title, cprompt, height, width, pauseopt, fp);

    pclose(fp);

    return code;
}