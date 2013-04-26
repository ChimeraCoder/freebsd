
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
__RCSID("$FreeBSD$");

#include <sys/param.h>
#include <sys/errno.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mptutil.h"

static CONFIG_PAGE_LOG_0 *
mpt_get_events(int fd, U16 *IOCStatus)
{

	return (mpt_read_extended_config_page(fd, MPI_CONFIG_EXTPAGETYPE_LOG,
	    0, 0, 0, IOCStatus));
}

/*
 *          1         2         3         4         5         6         7
 * 1234567890123456789012345678901234567890123456789012345678901234567890
 * < ID> < time > <ty> <X XX XX XX XX XX XX XX XX XX XX XX XX XX |..............|
 *  ID     Time   Type Log Data
 */
static void
mpt_print_event(MPI_LOG_0_ENTRY *entry, int verbose)
{
	int i;

	printf("%5d %7ds %4x ", entry->LogSequence, entry->TimeStamp,
	    entry->LogEntryQualifier);
	for (i = 0; i < 14; i++)
		printf("%02x ", entry->LogData[i]);
	printf("|");
	for (i = 0; i < 14; i++)
		printf("%c", isprint(entry->LogData[i]) ? entry->LogData[i] :
		    '.');
	printf("|\n");
	printf("                    ");
	for (i = 0; i < 14; i++)
		printf("%02x ", entry->LogData[i + 14]);
	printf("|");
	for (i = 0; i < 14; i++)
		printf("%c", isprint(entry->LogData[i + 14]) ?
		    entry->LogData[i + 14] : '.');
	printf("|\n");
}

static int
event_compare(const void *first, const void *second)
{
	MPI_LOG_0_ENTRY * const *one;
	MPI_LOG_0_ENTRY * const *two;

	one = first;
	two = second;
	return ((*one)->LogSequence - ((*two)->LogSequence));
}

static int
show_events(int ac, char **av)
{
	CONFIG_PAGE_LOG_0 *log;
	MPI_LOG_0_ENTRY **entries;
	int ch, error, fd, i, num_events, verbose;

	fd = mpt_open(mpt_unit);
	if (fd < 0) {
		error = errno;
		warn("mpt_open");
		return (error);
	}

	log = mpt_get_events(fd, NULL);
	if (log == NULL) {
		error = errno;
		warn("Failed to get event log info");
		return (error);
	}

	/* Default settings. */
	verbose = 0;

	/* Parse any options. */
	optind = 1;
	while ((ch = getopt(ac, av, "v")) != -1) {
		switch (ch) {
		case 'v':
			verbose = 1;
			break;
		case '?':
		default:
			return (EINVAL);
		}
	}
	ac -= optind;
	av += optind;

	/* Build a list of valid entries and sort them by sequence. */
	entries = malloc(sizeof(MPI_LOG_0_ENTRY *) * log->NumLogEntries);
	if (entries == NULL)
		return (ENOMEM);
	num_events = 0;
	for (i = 0; i < log->NumLogEntries; i++) {
		if (log->LogEntry[i].LogEntryQualifier ==
		    MPI_LOG_0_ENTRY_QUAL_ENTRY_UNUSED)
			continue;
		entries[num_events] = &log->LogEntry[i];
		num_events++;
	}

	qsort(entries, num_events, sizeof(MPI_LOG_0_ENTRY *), event_compare);

	if (num_events == 0)
		printf("Event log is empty\n");
	else {
		printf(" ID     Time   Type Log Data\n");
		for (i = 0; i < num_events; i++)
			mpt_print_event(entries[i], verbose);
	}
	
	free(entries);
	close(fd);

	return (0);
}
MPT_COMMAND(show, events, show_events);