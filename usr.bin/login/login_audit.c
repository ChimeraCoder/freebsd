
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <bsm/libbsm.h>
#include <bsm/audit_uevents.h>

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#include "login.h"

/*
 * Audit data
 */
static au_tid_t tid;

/*
 * The following tokens are included in the audit record for a successful
 * login: header, subject, return.
 */
void
au_login_success(void)
{
	token_t *tok;
	int aufd;
	au_mask_t aumask;
	auditinfo_t auinfo;
	uid_t uid = pwd->pw_uid;
	gid_t gid = pwd->pw_gid;
	pid_t pid = getpid();
	int au_cond;

	/* If we are not auditing, don't cut an audit record; just return. */
 	if (auditon(A_GETCOND, &au_cond, sizeof(au_cond)) < 0) {
		if (errno == ENOSYS)
			return;
		errx(1, "login: Could not determine audit condition");
	}
	if (au_cond == AUC_NOAUDIT)
		return;

	/* Compute and set the user's preselection mask. */
	if (au_user_mask(pwd->pw_name, &aumask) == -1)
		errx(1, "login: Could not set audit mask\n");

	/* Set the audit info for the user. */
	auinfo.ai_auid = uid;
	auinfo.ai_asid = pid;
	bcopy(&tid, &auinfo.ai_termid, sizeof(auinfo.ai_termid));
	bcopy(&aumask, &auinfo.ai_mask, sizeof(auinfo.ai_mask));
	if (setaudit(&auinfo) != 0)
		err(1, "login: setaudit failed");

	if ((aufd = au_open()) == -1)
		errx(1,"login: Audit Error: au_open() failed");

	if ((tok = au_to_subject32(uid, geteuid(), getegid(), uid, gid, pid,
	    pid, &tid)) == NULL)
		errx(1, "login: Audit Error: au_to_subject32() failed");
	au_write(aufd, tok);

	if ((tok = au_to_return32(0, 0)) == NULL)
		errx(1, "login: Audit Error: au_to_return32() failed");
	au_write(aufd, tok);

	if (au_close(aufd, 1, AUE_login) == -1)
		errx(1, "login: Audit Record was not committed.");
}

/*
 * The following tokens are included in the audit record for failed
 * login attempts: header, subject, text, return.
 */
void
au_login_fail(const char *errmsg, int na)
{
	token_t *tok;
	int aufd;
	int au_cond;
	uid_t uid;
	gid_t gid;
	pid_t pid = getpid();

	/* If we are not auditing, don't cut an audit record; just return. */
 	if (auditon(A_GETCOND, &au_cond, sizeof(au_cond)) < 0) {
		if (errno == ENOSYS)
			return;
		errx(1, "login: Could not determine audit condition");
	}
	if (au_cond == AUC_NOAUDIT)
		return;

	if ((aufd = au_open()) == -1)
		errx(1, "login: Audit Error: au_open() failed");

	if (na) {
		/*
		 * Non attributable event.  Assuming that login is not called
		 * within a user's session => auid,asid == -1.
		 */
		if ((tok = au_to_subject32(-1, geteuid(), getegid(), -1, -1,
		    pid, -1, &tid)) == NULL)
			errx(1, "login: Audit Error: au_to_subject32() failed");
	} else {
		/* We know the subject -- so use its value instead. */
		uid = pwd->pw_uid;
		gid = pwd->pw_gid;
		if ((tok = au_to_subject32(uid, geteuid(), getegid(), uid,
		    gid, pid, pid, &tid)) == NULL)
			errx(1, "login: Audit Error: au_to_subject32() failed");
	}
	au_write(aufd, tok);

	/* Include the error message. */
	if ((tok = au_to_text(errmsg)) == NULL)
		errx(1, "login: Audit Error: au_to_text() failed");
	au_write(aufd, tok);

	if ((tok = au_to_return32(1, errno)) == NULL)
		errx(1, "login: Audit Error: au_to_return32() failed");
	au_write(aufd, tok);

	if (au_close(aufd, 1, AUE_login) == -1)
		errx(1, "login: Audit Error: au_close() was not committed");
}

/*
 * The following tokens are included in the audit record for a logout:
 * header, subject, return.
 */
void
audit_logout(void)
{
	token_t *tok;
	int aufd;
	uid_t uid = pwd->pw_uid;
	gid_t gid = pwd->pw_gid;
	pid_t pid = getpid();
	int au_cond;

	/* If we are not auditing, don't cut an audit record; just return. */
 	if (auditon(A_GETCOND, &au_cond, sizeof(int)) < 0) {
		if (errno == ENOSYS)
			return;
		errx(1, "login: Could not determine audit condition");
	}
	if (au_cond == AUC_NOAUDIT)
		return;

	if ((aufd = au_open()) == -1)
		errx(1, "login: Audit Error: au_open() failed");

	/* The subject that is created (euid, egid of the current process). */
	if ((tok = au_to_subject32(uid, geteuid(), getegid(), uid, gid, pid,
	    pid, &tid)) == NULL)
		errx(1, "login: Audit Error: au_to_subject32() failed");
	au_write(aufd, tok);

	if ((tok = au_to_return32(0, 0)) == NULL)
		errx(1, "login: Audit Error: au_to_return32() failed");
	au_write(aufd, tok);

	if (au_close(aufd, 1, AUE_logout) == -1)
		errx(1, "login: Audit Record was not committed.");
}