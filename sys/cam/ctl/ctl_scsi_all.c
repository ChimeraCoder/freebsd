
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

__FBSDID("$FreeBSD$");

#include <sys/types.h>
#ifdef _KERNEL
#include <sys/systm.h>
#include <sys/libkern.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>
#else
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#endif

#include <cam/cam.h>
#include <cam/cam_ccb.h>
#include <cam/cam_queue.h>
#include <cam/cam_xpt.h>
#include <cam/scsi/scsi_all.h>

#include <cam/ctl/ctl_io.h>
#include <cam/ctl/ctl_scsi_all.h>
#include <sys/sbuf.h>
#ifndef _KERNEL
#include <camlib.h>
#endif

const char *
ctl_scsi_status_string(struct ctl_scsiio *ctsio)
{
	switch(ctsio->scsi_status) {
	case SCSI_STATUS_OK:
		return("OK");
	case SCSI_STATUS_CHECK_COND:
		return("Check Condition");
	case SCSI_STATUS_BUSY:
		return("Busy");
	case SCSI_STATUS_INTERMED:
		return("Intermediate");
	case SCSI_STATUS_INTERMED_COND_MET:
		return("Intermediate-Condition Met");
	case SCSI_STATUS_RESERV_CONFLICT:
		return("Reservation Conflict");
	case SCSI_STATUS_CMD_TERMINATED:
		return("Command Terminated");
	case SCSI_STATUS_QUEUE_FULL:
		return("Queue Full");
	case SCSI_STATUS_ACA_ACTIVE:
		return("ACA Active");
	case SCSI_STATUS_TASK_ABORTED:
		return("Task Aborted");
	default: {
		static char unkstr[64];
		snprintf(unkstr, sizeof(unkstr), "Unknown %#x",
			 ctsio->scsi_status);
		return(unkstr);
	}
	}
}

/*
 * scsi_command_string() returns 0 for success and -1 for failure.
 */
int
ctl_scsi_command_string(struct ctl_scsiio *ctsio,
			struct scsi_inquiry_data *inq_data, struct sbuf *sb)
{
	char cdb_str[(SCSI_MAX_CDBLEN * 3) + 1];

	sbuf_printf(sb, "%s. CDB: %s",
		    scsi_op_desc(ctsio->cdb[0], inq_data),
		    scsi_cdb_string(ctsio->cdb, cdb_str, sizeof(cdb_str)));

	return(0);
}

void
ctl_scsi_path_string(union ctl_io *io, char *path_str, int len)
{
	if (io->io_hdr.nexus.targ_target.wwid[0] == 0) {
		snprintf(path_str, len, "(%ju:%d:%ju:%d): ",
			 (uintmax_t)io->io_hdr.nexus.initid.id,
			 io->io_hdr.nexus.targ_port,
			 (uintmax_t)io->io_hdr.nexus.targ_target.id,
			 io->io_hdr.nexus.targ_lun);
	} else {
		/*
		 * XXX KDM find a better way to display FC WWIDs.
		 */
#ifdef _KERNEL
		snprintf(path_str, len, "(%ju:%d:%#jx,%#jx:%d): ",
			 (uintmax_t)io->io_hdr.nexus.initid.id,
			 io->io_hdr.nexus.targ_port,
			 (intmax_t)io->io_hdr.nexus.targ_target.wwid[0],
			 (intmax_t)io->io_hdr.nexus.targ_target.wwid[1],
			 io->io_hdr.nexus.targ_lun);
#else /* _KERNEL */
		snprintf(path_str, len, "(%ju:%d:%#jx,%#jx:%d): ",
			 (uintmax_t)io->io_hdr.nexus.initid.id,
			 io->io_hdr.nexus.targ_port,
			 (intmax_t)io->io_hdr.nexus.targ_target.wwid[0],
			 (intmax_t)io->io_hdr.nexus.targ_target.wwid[1],
			 io->io_hdr.nexus.targ_lun);
#endif /* _KERNEL */
	}
}

/*
 * ctl_scsi_sense_sbuf() returns 0 for success and -1 for failure.
 */
int
ctl_scsi_sense_sbuf(struct ctl_scsiio *ctsio,
		    struct scsi_inquiry_data *inq_data, struct sbuf *sb,
		    scsi_sense_string_flags flags)
{
	char	  path_str[64];

	if ((ctsio == NULL) || (sb == NULL))
		return(-1);

	ctl_scsi_path_string((union ctl_io *)ctsio, path_str, sizeof(path_str));

	if (flags & SSS_FLAG_PRINT_COMMAND) {

		sbuf_cat(sb, path_str);

		ctl_scsi_command_string(ctsio, inq_data, sb);

		sbuf_printf(sb, "\n");
	}

	scsi_sense_only_sbuf(&ctsio->sense_data, ctsio->sense_len, sb,
			     path_str, inq_data, ctsio->cdb, ctsio->cdb_len);

	return(0);
}

char *
ctl_scsi_sense_string(struct ctl_scsiio *ctsio,
		      struct scsi_inquiry_data *inq_data, char *str,
		      int str_len)
{
	struct sbuf sb;

	sbuf_new(&sb, str, str_len, 0);

	ctl_scsi_sense_sbuf(ctsio, inq_data, &sb, SSS_FLAG_PRINT_COMMAND);

	sbuf_finish(&sb);

	return(sbuf_data(&sb));
}

#ifdef _KERNEL
void 
ctl_scsi_sense_print(struct ctl_scsiio *ctsio,
		     struct scsi_inquiry_data *inq_data)
{
	struct sbuf sb;
	char str[512];

	sbuf_new(&sb, str, sizeof(str), 0);

	ctl_scsi_sense_sbuf(ctsio, inq_data, &sb, SSS_FLAG_PRINT_COMMAND);

	sbuf_finish(&sb);

	printf("%s", sbuf_data(&sb));
}

#else /* _KERNEL */
void
ctl_scsi_sense_print(struct ctl_scsiio *ctsio,
		     struct scsi_inquiry_data *inq_data, FILE *ofile)
{
	struct sbuf sb;
	char str[512];

	if ((ctsio == NULL) || (ofile == NULL))
		return;

	sbuf_new(&sb, str, sizeof(str), 0);

	ctl_scsi_sense_sbuf(ctsio, inq_data, &sb, SSS_FLAG_PRINT_COMMAND);

	sbuf_finish(&sb);

	fprintf(ofile, "%s", sbuf_data(&sb));
}

#endif /* _KERNEL */