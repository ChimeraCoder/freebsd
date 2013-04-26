
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

#include "cvs.h"
#include <assert.h>

int
No_Difference (finfo, vers)
    struct file_info *finfo;
    Vers_TS *vers;
{
    Node *p;
    int ret;
    char *ts, *options;
    int retcode = 0;
    char *tocvsPath;

    /* If ts_user is "Is-modified", we can only conclude the files are
       different (since we don't have the file's contents).  */
    if (vers->ts_user != NULL
	&& strcmp (vers->ts_user, "Is-modified") == 0)
	return -1;

    if (!vers->srcfile || !vers->srcfile->path)
	return (-1);			/* different since we couldn't tell */

#ifdef PRESERVE_PERMISSIONS_SUPPORT
    /* If special files are in use, then any mismatch of file metadata
       information also means that the files should be considered different. */
    if (preserve_perms && special_file_mismatch (finfo, vers->vn_user, NULL))
	return 1;
#endif

    if (vers->entdata && vers->entdata->options)
	options = xstrdup (vers->entdata->options);
    else
	options = xstrdup ("");

    tocvsPath = wrap_tocvs_process_file (finfo->file);
    retcode = RCS_cmp_file( vers->srcfile, vers->vn_user, (char **)NULL,
                            (char *)NULL, options,
			    tocvsPath == NULL ? finfo->file : tocvsPath );
    if (retcode == 0)
    {
	/* no difference was found, so fix the entries file */
	ts = time_stamp (finfo->file);
	Register (finfo->entries, finfo->file,
		  vers->vn_user ? vers->vn_user : vers->vn_rcs, ts,
		  options, vers->tag, vers->date, (char *) 0);
#ifdef SERVER_SUPPORT
	if (server_active)
	{
	    /* We need to update the entries line on the client side.  */
	    server_update_entries
	      (finfo->file, finfo->update_dir, finfo->repository, SERVER_UPDATED);
	}
#endif
	free (ts);

	/* update the entdata pointer in the vers_ts structure */
	p = findnode (finfo->entries, finfo->file);
	assert (p);
	vers->entdata = p->data;

	ret = 0;
    }
    else
	ret = 1;			/* files were really different */

    if (tocvsPath)
    {
	/* Need to call unlink myself because the noexec variable
	 * has been set to 1.  */
	if (trace)
	    (void) fprintf (stderr, "%s-> unlink (%s)\n",
			    CLIENT_SERVER_STR, tocvsPath);
	if ( CVS_UNLINK (tocvsPath) < 0)
	    error (0, errno, "could not remove %s", tocvsPath);
    }

    free (options);
    return (ret);
}