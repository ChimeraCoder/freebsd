
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
#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "acpidump.h"

int	dflag;	/* Disassemble AML using iasl(8) */
int	tflag;	/* Dump contents of SDT tables */
int	vflag;	/* Use verbose messages */

static void
usage(const char *progname)
{

	fprintf(stderr, "usage: %s [-d] [-t] [-h] [-v] [-f dsdt_input] "
			"[-o dsdt_output]\n", progname);
	fprintf(stderr, "To send ASL:\n\t%s -dt | gzip -c9 > foo.asl.gz\n",
	    progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	ACPI_TABLE_HEADER *rsdt, *sdt;
	char	c, *progname;
	char	*dsdt_input_file, *dsdt_output_file;

	dsdt_input_file = dsdt_output_file = NULL;
	progname = argv[0];

	if (argc < 2)
		usage(progname);

	while ((c = getopt(argc, argv, "dhtvf:o:")) != -1) {
		switch (c) {
		case 'd':
			dflag = 1;
			break;
		case 't':
			tflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		case 'f':
			dsdt_input_file = optarg;
			break;
		case 'o':
			dsdt_output_file = optarg;
			break;
		case 'h':
		default:
			usage(progname);
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	/* Get input either from file or /dev/mem */
	if (dsdt_input_file != NULL) {
		if (dflag == 0 && tflag == 0) {
			warnx("Need to specify -d or -t with DSDT input file");
			usage(progname);
		} else if (tflag != 0) {
			warnx("Can't use -t with DSDT input file");
			usage(progname);
		}
		if (vflag)
			warnx("loading DSDT file: %s", dsdt_input_file);
		rsdt = dsdt_load_file(dsdt_input_file);
	} else {
		if (vflag)
			warnx("loading RSD PTR from /dev/mem");
		rsdt = sdt_load_devmem();
	}

	/* Display misc. SDT tables (only available when using /dev/mem) */
	if (tflag) {
		if (vflag)
			warnx("printing various SDT tables");
		sdt_print_all(rsdt);
	}

	/* Translate RSDT to DSDT pointer */
	if (dsdt_input_file == NULL) {
		sdt = sdt_from_rsdt(rsdt, ACPI_SIG_FADT, NULL);
		sdt = dsdt_from_fadt((ACPI_TABLE_FADT *)sdt);
	} else {
		sdt = rsdt;
		rsdt = NULL;
	}

	/* Dump the DSDT and SSDTs to a file */
	if (dsdt_output_file != NULL) {
		if (vflag)
			warnx("saving DSDT file: %s", dsdt_output_file);
		dsdt_save_file(dsdt_output_file, rsdt, sdt);
	}

	/* Disassemble the DSDT into ASL */
	if (dflag) {
		if (vflag)
			warnx("disassembling DSDT, iasl messages follow");
		aml_disassemble(rsdt, sdt);
		if (vflag)
			warnx("iasl processing complete");
	}

	exit(0);
}