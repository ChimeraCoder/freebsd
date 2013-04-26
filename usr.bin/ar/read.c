
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

#include <sys/queue.h>
#include <sys/stat.h>
#include <archive.h>
#include <archive_entry.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

#include "ar.h"

static void read_archive(struct bsdar *bsdar, char mode);

void
ar_mode_p(struct bsdar *bsdar)
{

	read_archive(bsdar, 'p');
}

void
ar_mode_t(struct bsdar *bsdar)
{

	read_archive(bsdar, 't');
}

void
ar_mode_x(struct bsdar *bsdar)
{

	read_archive(bsdar, 'x');
}

/*
 * Handle read modes: 'x', 't' and 'p'.
 */
static void
read_archive(struct bsdar *bsdar, char mode)
{
	struct archive		 *a;
	struct archive_entry	 *entry;
	struct stat		  sb;
	struct tm		 *tp;
	const char		 *bname;
	const char		 *name;
	mode_t			  md;
	size_t			  size;
	time_t			  mtime;
	uid_t			  uid;
	gid_t			  gid;
	char			**av;
	char			  buf[25];
	char			  find;
	int			  flags, r, i;

	if ((a = archive_read_new()) == NULL)
		bsdar_errc(bsdar, EX_SOFTWARE, 0, "archive_read_new failed");
	archive_read_support_format_ar(a);
	AC(archive_read_open_filename(a, bsdar->filename, DEF_BLKSZ));

	for (;;) {
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_WARN || r == ARCHIVE_RETRY ||
		    r == ARCHIVE_FATAL)
			bsdar_warnc(bsdar, 0, "%s", archive_error_string(a));
		if (r == ARCHIVE_EOF || r == ARCHIVE_FATAL)
			break;
		if (r == ARCHIVE_RETRY) {
			bsdar_warnc(bsdar, 0, "Retrying...");
			continue;
		}

		name = archive_entry_pathname(entry);

		/* Skip pseudo members. */
		if (strcmp(name, "/") == 0 || strcmp(name, "//") == 0)
			continue;

		if (bsdar->argc > 0) {
			find = 0;
			for(i = 0; i < bsdar->argc; i++) {
				av = &bsdar->argv[i];
				if (*av == NULL)
					continue;
				if ((bname = basename(*av)) == NULL)
					bsdar_errc(bsdar, EX_SOFTWARE, errno,
					    "basename failed");
				if (strcmp(bname, name) != 0)
					continue;

				*av = NULL;
				find = 1;
				break;
			}
			if (!find)
				continue;
		}

		if (mode == 't') {
			if (bsdar->options & AR_V) {
				md = archive_entry_mode(entry);
				uid = archive_entry_uid(entry);
				gid = archive_entry_gid(entry);
				size = archive_entry_size(entry);
				mtime = archive_entry_mtime(entry);
				(void)strmode(md, buf);
				(void)fprintf(stdout, "%s %6d/%-6d %8ju ",
				    buf + 1, uid, gid, (uintmax_t)size);
				tp = localtime(&mtime);
				(void)strftime(buf, sizeof(buf),
				    "%b %e %H:%M %Y", tp);
				(void)fprintf(stdout, "%s %s", buf, name);
			} else
				(void)fprintf(stdout, "%s", name);
			r = archive_read_data_skip(a);
			if (r == ARCHIVE_WARN || r == ARCHIVE_RETRY ||
			    r == ARCHIVE_FATAL) {
				(void)fprintf(stdout, "\n");
				bsdar_warnc(bsdar, 0, "%s",
				    archive_error_string(a));
			}

			if (r == ARCHIVE_FATAL)
				break;

			(void)fprintf(stdout, "\n");
		} else {
			/* mode == 'x' || mode = 'p' */
			if (mode == 'p') {
				if (bsdar->options & AR_V) {
					(void)fprintf(stdout, "\n<%s>\n\n",
					    name);
					fflush(stdout);
				}
				r = archive_read_data_into_fd(a, 1);
			} else {
				/* mode == 'x' */
				if (stat(name, &sb) != 0) {
					if (errno != ENOENT) {
						bsdar_warnc(bsdar, 0,
						    "stat %s failed",
						    bsdar->filename);
						continue;
					}
				} else {
					/* stat success, file exist */
					if (bsdar->options & AR_CC)
						continue;
					if (bsdar->options & AR_U &&
					    archive_entry_mtime(entry) <=
					    sb.st_mtime)
						continue;
				}

				if (bsdar->options & AR_V)
					(void)fprintf(stdout, "x - %s\n", name);
				flags = 0;
				if (bsdar->options & AR_O)
					flags |= ARCHIVE_EXTRACT_TIME;

				r = archive_read_extract(a, entry, flags);
			}

			if (r)
				bsdar_warnc(bsdar, 0, "%s",
				    archive_error_string(a));
		}
	}
	AC(archive_read_close(a));
	AC(archive_read_free(a));
}