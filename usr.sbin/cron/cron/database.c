
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

#if !defined(lint) && !defined(LINT)
static const char rcsid[] =
  "$FreeBSD$";
#endif

/* vix 26jan87 [RCS has the log]
 */


#include "cron.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>


#define TMAX(a,b) ((a)>(b)?(a):(b))


static	void		process_crontab(char *, char *, char *,
					     struct stat *,
					     cron_db *, cron_db *);


void
load_database(old_db)
	cron_db		*old_db;
{
	DIR		*dir;
	struct stat	statbuf;
	struct stat	syscron_stat;
	DIR_T   	*dp;
	cron_db		new_db;
	user		*u, *nu;

	Debug(DLOAD, ("[%d] load_database()\n", getpid()))

	/* before we start loading any data, do a stat on SPOOL_DIR
	 * so that if anything changes as of this moment (i.e., before we've
	 * cached any of the database), we'll see the changes next time.
	 */
	if (stat(SPOOL_DIR, &statbuf) < OK) {
		log_it("CRON", getpid(), "STAT FAILED", SPOOL_DIR);
		(void) exit(ERROR_EXIT);
	}

	/* track system crontab file
	 */
	if (stat(SYSCRONTAB, &syscron_stat) < OK)
		syscron_stat.st_mtime = 0;

	/* if spooldir's mtime has not changed, we don't need to fiddle with
	 * the database.
	 *
	 * Note that old_db->mtime is initialized to 0 in main(), and
	 * so is guaranteed to be different than the stat() mtime the first
	 * time this function is called.
	 */
	if (old_db->mtime == TMAX(statbuf.st_mtime, syscron_stat.st_mtime)) {
		Debug(DLOAD, ("[%d] spool dir mtime unch, no load needed.\n",
			      getpid()))
		return;
	}

	/* something's different.  make a new database, moving unchanged
	 * elements from the old database, reloading elements that have
	 * actually changed.  Whatever is left in the old database when
	 * we're done is chaff -- crontabs that disappeared.
	 */
	new_db.mtime = TMAX(statbuf.st_mtime, syscron_stat.st_mtime);
	new_db.head = new_db.tail = NULL;

	if (syscron_stat.st_mtime) {
		process_crontab("root", SYS_NAME,
				SYSCRONTAB, &syscron_stat,
				&new_db, old_db);
	}

	/* we used to keep this dir open all the time, for the sake of
	 * efficiency.  however, we need to close it in every fork, and
	 * we fork a lot more often than the mtime of the dir changes.
	 */
	if (!(dir = opendir(SPOOL_DIR))) {
		log_it("CRON", getpid(), "OPENDIR FAILED", SPOOL_DIR);
		(void) exit(ERROR_EXIT);
	}

	while (NULL != (dp = readdir(dir))) {
		char	fname[MAXNAMLEN+1],
			tabname[MAXNAMLEN+1];

		/* avoid file names beginning with ".".  this is good
		 * because we would otherwise waste two guaranteed calls
		 * to getpwnam() for . and .., and also because user names
		 * starting with a period are just too nasty to consider.
		 */
		if (dp->d_name[0] == '.')
			continue;

		(void) strncpy(fname, dp->d_name, sizeof(fname));
		fname[sizeof(fname)-1] = '\0';
		(void) snprintf(tabname, sizeof tabname, CRON_TAB(fname));

		process_crontab(fname, fname, tabname,
				&statbuf, &new_db, old_db);
	}
	closedir(dir);

	/* if we don't do this, then when our children eventually call
	 * getpwnam() in do_command.c's child_process to verify MAILTO=,
	 * they will screw us up (and v-v).
	 */
	endpwent();

	/* whatever's left in the old database is now junk.
	 */
	Debug(DLOAD, ("unlinking old database:\n"))
	for (u = old_db->head;  u != NULL;  u = nu) {
		Debug(DLOAD, ("\t%s\n", u->name))
		nu = u->next;
		unlink_user(old_db, u);
		free_user(u);
	}

	/* overwrite the database control block with the new one.
	 */
	*old_db = new_db;
	Debug(DLOAD, ("load_database is done\n"))
}


void
link_user(db, u)
	cron_db	*db;
	user	*u;
{
	if (db->head == NULL)
		db->head = u;
	if (db->tail)
		db->tail->next = u;
	u->prev = db->tail;
	u->next = NULL;
	db->tail = u;
}


void
unlink_user(db, u)
	cron_db	*db;
	user	*u;
{
	if (u->prev == NULL)
		db->head = u->next;
	else
		u->prev->next = u->next;

	if (u->next == NULL)
		db->tail = u->prev;
	else
		u->next->prev = u->prev;
}


user *
find_user(db, name)
	cron_db	*db;
	char	*name;
{
	char	*env_get();
	user	*u;

	for (u = db->head;  u != NULL;  u = u->next)
		if (!strcmp(u->name, name))
			break;
	return u;
}


static void
process_crontab(uname, fname, tabname, statbuf, new_db, old_db)
	char		*uname;
	char		*fname;
	char		*tabname;
	struct stat	*statbuf;
	cron_db		*new_db;
	cron_db		*old_db;
{
	struct passwd	*pw = NULL;
	int		crontab_fd = OK - 1;
	user		*u;

	if (strcmp(fname, SYS_NAME) && !(pw = getpwnam(uname))) {
		/* file doesn't have a user in passwd file.
		 */
		log_it(fname, getpid(), "ORPHAN", "no passwd entry");
		goto next_crontab;
	}

	if ((crontab_fd = open(tabname, O_RDONLY, 0)) < OK) {
		/* crontab not accessible?
		 */
		log_it(fname, getpid(), "CAN'T OPEN", tabname);
		goto next_crontab;
	}

	if (fstat(crontab_fd, statbuf) < OK) {
		log_it(fname, getpid(), "FSTAT FAILED", tabname);
		goto next_crontab;
	}

	Debug(DLOAD, ("\t%s:", fname))
	u = find_user(old_db, fname);
	if (u != NULL) {
		/* if crontab has not changed since we last read it
		 * in, then we can just use our existing entry.
		 */
		if (u->mtime == statbuf->st_mtime) {
			Debug(DLOAD, (" [no change, using old data]"))
			unlink_user(old_db, u);
			link_user(new_db, u);
			goto next_crontab;
		}

		/* before we fall through to the code that will reload
		 * the user, let's deallocate and unlink the user in
		 * the old database.  This is more a point of memory
		 * efficiency than anything else, since all leftover
		 * users will be deleted from the old database when
		 * we finish with the crontab...
		 */
		Debug(DLOAD, (" [delete old data]"))
		unlink_user(old_db, u);
		free_user(u);
		log_it(fname, getpid(), "RELOAD", tabname);
	}
	u = load_user(crontab_fd, pw, fname);
	if (u != NULL) {
		u->mtime = statbuf->st_mtime;
		link_user(new_db, u);
	}

next_crontab:
	if (crontab_fd >= OK) {
		Debug(DLOAD, (" [done]\n"))
		close(crontab_fd);
	}
}