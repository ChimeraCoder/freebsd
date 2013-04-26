
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

/*
 * ctm_dequeue:  Dequeue queued delta pieces and mail them.
 *
 * The pieces have already been packaged up as mail messages by ctm_smail,
 * and will be simply passed to sendmail in alphabetical order.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fts.h>
#include <limits.h>
#include <errno.h>
#include <paths.h>
#include "error.h"
#include "options.h"

#define DEFAULT_NUM	1	/* Default number of pieces mailed per run. */

int fts_sort(const FTSENT * const *, const FTSENT * const *);
int run_sendmail(int ifd);

int
main(int argc, char **argv)
{
    char *log_file = NULL;
    char *queue_dir = NULL;
    char *list[2];
    int num_to_send = DEFAULT_NUM, chunk;
    int fd;
    FTS *fts;
    FTSENT *ftsent;
    int piece, npieces;
    char filename[PATH_MAX];

    err_prog_name(argv[0]);

    OPTIONS("[-l log] [-n num] queuedir")
	NUMBER('n', num_to_send)
	STRING('l', log_file)
    ENDOPTS

    if (argc != 2)
	usage();

    if (log_file)
	err_set_log(log_file);

    queue_dir = argv[1];
    list[0] = queue_dir;
    list[1] = NULL;

    fts = fts_open(list, FTS_PHYSICAL|FTS_COMFOLLOW, fts_sort);
    if (fts == NULL)
    {
	err("fts failed on `%s'", queue_dir);
	exit(1);
    }

    ftsent = fts_read(fts);
    if (ftsent == NULL || ftsent->fts_info != FTS_D)
    {
	err("not a directory: %s", queue_dir);
	exit(1);
    }

    ftsent = fts_children(fts, 0);
    if (ftsent == NULL && errno)
    {
	err("*ftschildren failed");
	exit(1);
    }

    for (chunk = 1; ftsent != NULL; ftsent = ftsent->fts_link)
    {
	/*
	 * Skip non-files and ctm_smail tmp files (ones starting with `.')
	 */
	if (ftsent->fts_info != FTS_F || ftsent->fts_name[0] == '.')
	    continue;

	sprintf(filename, "%s/%s", queue_dir, ftsent->fts_name);
	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
	    err("*open: %s", filename);
	    exit(1);
	}

	if (run_sendmail(fd))
	    exit(1);

	close(fd);
	
	if (unlink(filename) < 0)
	{
	    err("*unlink: %s", filename);
	    exit(1);
	}
	
	/*
	 * Deduce the delta, piece number, and number of pieces from
	 * the name of the file in the queue.  Ideally, we should be
	 * able to get the mail alias name too.
	 *
	 * NOTE: This depends intimately on the queue name used in ctm_smail.
	 */
	npieces = atoi(&ftsent->fts_name[ftsent->fts_namelen-3]);
	piece = atoi(&ftsent->fts_name[ftsent->fts_namelen-7]);
	err("%.*s %d/%d sent", (int)(ftsent->fts_namelen-8), ftsent->fts_name,
		piece, npieces);

	if (chunk++ == num_to_send)
	    break;
    }

    fts_close(fts);

    return(0);
}

int
fts_sort(const FTSENT * const * a, const FTSENT * const * b)
{
    if ((*a)->fts_info != FTS_F)
	return(0);
    if ((*a)->fts_info != FTS_F)
	return(0);

    return (strcmp((*a)->fts_name, (*b)->fts_name));
}

/*
 * Run sendmail with the given file descriptor as standard input.
 * Sendmail will decode the destination from the message contents.
 * Returns 0 on success, 1 on failure.
 */
int
run_sendmail(int ifd)
{
    pid_t child, pid;
    int status;

    switch (child = fork())
    {
    case -1:
	err("*fork");
	return(1);

    case 0:	/* Child */
	dup2(ifd, 0);
	execl(_PATH_SENDMAIL, _PATH_SENDMAIL, "-odq", "-t", (char *)NULL);
	err("*exec: %s", _PATH_SENDMAIL);
	_exit(1);

    default:	/* Parent */
	while ((pid = wait(&status)) != child)
	{
	    if (pid == -1 && errno != EINTR)
	    {
		err("*wait");
		return(1);
	    }
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
	    err("sendmail failed");
	    return(1);
	}
    }

    return(0);
}