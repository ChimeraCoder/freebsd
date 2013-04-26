
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

#include <sys/param.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <archive.h>
#include <dialog.h>

static int extract_files(int nfiles, const char **files);

int
main(void)
{
	char *diststring;
	const char **dists;
	int i, retval, ndists = 0;

	if (getenv("DISTRIBUTIONS") == NULL) {
		fprintf(stderr, "DISTRIBUTIONS variable is not set\n");
		return (1);
	}

	diststring = strdup(getenv("DISTRIBUTIONS"));
	for (i = 0; diststring[i] != 0; i++)
		if (isspace(diststring[i]) && !isspace(diststring[i+1]))
			ndists++;
	ndists++; /* Last one */

	dists = calloc(ndists, sizeof(const char *));
	if (dists == NULL) {
		fprintf(stderr, "Out of memory!\n");
		free(diststring);
		return (1);
	}

	for (i = 0; i < ndists; i++)
		dists[i] = strsep(&diststring, " \t");

	init_dialog(stdin, stdout);
	dialog_vars.backtitle = __DECONST(char *, "FreeBSD Installer");
	dlg_put_backtitle();

	if (chdir(getenv("BSDINSTALL_CHROOT")) != 0) {
		char error[512];
		sprintf(error, "Could could change to directory %s: %s\n",
		    getenv("BSDINSTALL_DISTDIR"), strerror(errno));
		dialog_msgbox("Error", error, 0, 0, TRUE);
		end_dialog();
		return (1);
	}

	retval = extract_files(ndists, dists);

	end_dialog();

	free(diststring);
	free(dists);

	return (retval);
}

static int
count_files(const char *file)
{
	struct archive *archive;
	struct archive_entry *entry;
	static FILE *manifest = NULL;
	char path[MAXPATHLEN];
	char errormsg[512];
	int file_count, err;

	if (manifest == NULL) {
		sprintf(path, "%s/MANIFEST", getenv("BSDINSTALL_DISTDIR"));
		manifest = fopen(path, "r");
	}

	if (manifest != NULL) {
		char line[512];
		char *tok1, *tok2;

		rewind(manifest);
		while (fgets(line, sizeof(line), manifest) != NULL) {
			tok2 = line;
			tok1 = strsep(&tok2, "\t");
			if (tok1 == NULL || strcmp(tok1, file) != 0)
				continue;

			/*
			 * We're at the right manifest line. The file count is
			 * in the third element
			 */
			tok1 = strsep(&tok2, "\t");
			tok1 = strsep(&tok2, "\t");
			if (tok1 != NULL)
				return atoi(tok1);
		}
	}

	/* Either we didn't have a manifest, or this archive wasn't there */
	archive = archive_read_new();
	archive_read_support_format_all(archive);
	archive_read_support_filter_all(archive);
	sprintf(path, "%s/%s", getenv("BSDINSTALL_DISTDIR"), file);
	err = archive_read_open_filename(archive, path, 4096);
	if (err != ARCHIVE_OK) {
		snprintf(errormsg, sizeof(errormsg),
		    "Error while extracting %s: %s\n", file,
		    archive_error_string(archive));
		dialog_msgbox("Extract Error", errormsg, 0, 0, TRUE);
		return (-1);
	}

	file_count = 0;
	while (archive_read_next_header(archive, &entry) == ARCHIVE_OK)
		file_count++;
	archive_read_free(archive);

	return (file_count);
}

static int
extract_files(int nfiles, const char **files)
{
	const char *items[nfiles*2];
	char path[PATH_MAX];
	int archive_files[nfiles];
	int total_files, current_files, archive_file;
	struct archive *archive;
	struct archive_entry *entry;
	char errormsg[512];
	char status[8];
	int i, err, progress, last_progress;

	err = 0;
	progress = 0;
	
	/* Make the transfer list for dialog */
	for (i = 0; i < nfiles; i++) {
		items[i*2] = strrchr(files[i], '/');
		if (items[i*2] != NULL)
			items[i*2]++;
		else
			items[i*2] = files[i];
		items[i*2 + 1] = "Pending";
	}

	dialog_msgbox("",
	    "Checking distribution archives.\nPlease wait...", 0, 0, FALSE);

	/* Count all the files */
	total_files = 0;
	for (i = 0; i < nfiles; i++) {
		archive_files[i] = count_files(files[i]);
		if (archive_files[i] < 0)
			return (-1);
		total_files += archive_files[i];
	}

	current_files = 0;

	for (i = 0; i < nfiles; i++) {
		archive = archive_read_new();
		archive_read_support_format_all(archive);
		archive_read_support_filter_all(archive);
		sprintf(path, "%s/%s", getenv("BSDINSTALL_DISTDIR"), files[i]);
		err = archive_read_open_filename(archive, path, 4096);

		items[i*2 + 1] = "In Progress";
		archive_file = 0;

		while ((err = archive_read_next_header(archive, &entry)) ==
		    ARCHIVE_OK) {
			last_progress = progress;
			progress = (current_files*100)/total_files; 

			sprintf(status, "-%d",
			    (archive_file*100)/archive_files[i]);
			items[i*2 + 1] = status;

			if (progress > last_progress)
				dialog_mixedgauge("Archive Extraction",
				    "Extracting distribution files...", 0, 0,
				    progress, nfiles,
				    __DECONST(char **, items));

			err = archive_read_extract(archive, entry,
			    ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_OWNER |
			    ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL |
			    ARCHIVE_EXTRACT_XATTR | ARCHIVE_EXTRACT_FFLAGS);

			if (err != ARCHIVE_OK)
				break;

			archive_file++;
			current_files++;
		}

		items[i*2 + 1] = "Done";

		if (err != ARCHIVE_EOF) {
			snprintf(errormsg, sizeof(errormsg),
			    "Error while extracting %s: %s\n", items[i*2],
			    archive_error_string(archive));
			items[i*2 + 1] = "Failed";
			dialog_msgbox("Extract Error", errormsg, 0, 0,
			    TRUE);
			return (err);
		}

		archive_read_free(archive);
	}

	return (0);
}