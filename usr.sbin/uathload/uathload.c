
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
 * Atheros AR5523 USB Station Firmware downloader.
 *
 *    uathload -d ugen-device [firmware-file]
 *
 * Intended to be called from devd on device discovery.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/endian.h>
#include <sys/mman.h>

#include <sys/ioctl.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

/* all fields are big endian */
struct uath_fwmsg {
	uint32_t	flags;
#define UATH_WRITE_BLOCK	(1 << 4)

	uint32_t	len;
#define UATH_MAX_FWBLOCK_SIZE	2048

	uint32_t	total;
	uint32_t	remain;
	uint32_t	rxtotal;
	uint32_t	pad[123];
} __packed;

#define UATH_DATA_TIMEOUT	10000
#define UATH_CMD_TIMEOUT	1000

#define	VERBOSE(_fmt, ...) do {			\
	if (verbose) {				\
		printf(_fmt, __VA_ARGS__);	\
		fflush(stdout);			\
	}					\
} while (0)

extern	uint8_t _binary_ar5523_bin_start;
extern	uint8_t _binary_ar5523_bin_end;

static int
getdevname(const char *devname, char *msgdev, char *datadev)
{
	char *bn, *dn;

	dn = dirname(devname);
	if (dn == NULL)
		return (-1);
	bn = basename(devname);
	if (bn == NULL || strncmp(bn, "ugen", 4))
		return (-1);
	bn += 4;

	/* NB: pipes are hardcoded */
	snprintf(msgdev, 256, "%s/usb/%s.1", dn, bn);
	snprintf(datadev, 256, "%s/usb/%s.2", dn, bn);
	return (0);
}

static void
usage(void)
{
	errx(-1, "usage: uathload [-v] -d devname [firmware]");
}

int
main(int argc, char *argv[])
{
	const char *fwname, *devname;
	char msgdev[256], datadev[256];
	struct uath_fwmsg txmsg, rxmsg;
	char *txdata;
	struct stat sb;
	int msg, data, fw, timeout, b, c;
	int bufsize = 512, verbose = 0;
	ssize_t len;

	devname = NULL;
	while ((c = getopt(argc, argv, "d:v")) != -1) {
		switch (c) {
		case 'd':
			devname = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	}
	argc -= optind;
	argv += optind;

	if (devname == NULL)
		errx(-1, "No device name; use -d to specify the ugen device");
	if (argc > 1)
		usage();

	if (argc == 1) {
		fwname = argv[0];
		fw = open(fwname, O_RDONLY, 0);
		if (fw < 0)
			err(-1, "open(%s)", fwname);
		if (fstat(fw, &sb) < 0)
			err(-1, "fstat(%s)", fwname);
		txdata = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fw, 0);
		if (txdata == MAP_FAILED)
			err(-1, "mmap(%s)", fwname);
		len = sb.st_size;
	} else {
		fwname = "ar5523.bin (builtin)";
		fw = -1;
		txdata = &_binary_ar5523_bin_start;
		len = &_binary_ar5523_bin_end - &_binary_ar5523_bin_start;
	}
	/* XXX verify device is an AR5005 part */
	if (getdevname(devname, msgdev, datadev))
		err(-1, "getdevname error");

	msg = open(msgdev, O_RDWR, 0);
	if (msg < 0)
		err(-1, "open(%s)", msgdev);
	timeout = UATH_DATA_TIMEOUT;
	if (ioctl(msg, USB_SET_RX_TIMEOUT, &timeout) < 0)
		err(-1, "%s: USB_SET_RX_TIMEOUT(%u)", msgdev, UATH_DATA_TIMEOUT);
	if (ioctl(msg, USB_SET_RX_BUFFER_SIZE, &bufsize) < 0)
		err(-1, "%s: USB_SET_RX_BUFFER_SIZE(%u)", msgdev, bufsize);

	data = open(datadev, O_WRONLY, 0);
	if (data < 0)
		err(-1, "open(%s)", datadev);
	timeout = UATH_DATA_TIMEOUT;
	if (ioctl(data, USB_SET_TX_TIMEOUT, &timeout) < 0)
		err(-1, "%s: USB_SET_TX_TIMEOUT(%u)", datadev,
		    UATH_DATA_TIMEOUT);

	VERBOSE("Load firmware %s to %s\n", fwname, devname);

	bzero(&txmsg, sizeof (struct uath_fwmsg));
	txmsg.flags = htobe32(UATH_WRITE_BLOCK);
	txmsg.total = htobe32(len);

	b = 0;
	while (len > 0) {
		int mlen;

		mlen = len;
		if (mlen > UATH_MAX_FWBLOCK_SIZE)
			mlen = UATH_MAX_FWBLOCK_SIZE;
		txmsg.remain = htobe32(len - mlen);
		txmsg.len = htobe32(mlen);

		/* send firmware block meta-data */
		VERBOSE("send block %2u: %zd bytes remaining", b, len - mlen);
		if (write(msg, &txmsg, sizeof(txmsg)) != sizeof(txmsg)) {
			VERBOSE("%s", "\n");
			err(-1, "error sending msg (%s)", msgdev);
			break;
		}

		/* send firmware block data */
		VERBOSE("%s", "\n             : data...");
		if (write(data, txdata, mlen) != mlen) {
			VERBOSE("%s", "\n");
			err(-1, "error sending data (%s)", datadev);
			break;
		}

		/* wait for ack from firmware */
		VERBOSE("%s", "\n             : wait for ack...");
		bzero(&rxmsg, sizeof(rxmsg));
		if (read(msg, &rxmsg, sizeof(rxmsg)) != sizeof(rxmsg)) {
			VERBOSE("%s", "\n");
			err(-1, "error reading msg (%s)", msgdev);
			break;
		}

		VERBOSE("flags=0x%x total=%d\n",
		    be32toh(rxmsg.flags), be32toh(rxmsg.rxtotal));
		len -= mlen;
		txdata += mlen;
		b++;
	}
	sleep(1);
	close(fw);
	close(msg);
	close(data);
	return 0;
}