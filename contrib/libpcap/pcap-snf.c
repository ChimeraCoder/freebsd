
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
		 * Has "pcap_breakloop()" been called?
		 */		if (p->break_loop) {
			if (n == 0) {
				p->break_loop = 0;
				return (-2);
			} else {
				return (n);
			}
		}

		err = snf_ring_recv(p->md.snf_ring, p->md.snf_timeout, &req);

		if (err) {
			if (err == EBUSY || err == EAGAIN)
				return (0);
			if (err == EINTR)
				continue;
			if (err != 0) {
				snprintf(p->errbuf, PCAP_ERRBUF_SIZE, "snf_read: %s",
				 	 pcap_strerror(err));
				return -1;
			}
		}

		caplen = req.length;
		if (caplen > p->snapshot)
			caplen = p->snapshot;

		if ((p->fcode.bf_insns == NULL) ||
		     bpf_filter(p->fcode.bf_insns, req.pkt_addr, req.length, caplen)) {
			hdr.ts = snf_timestamp_to_timeval(req.timestamp);
			hdr.caplen = caplen;
			hdr.len = req.length;
			callback(user, &hdr, req.pkt_addr);
		}
		n++;
	}
	return (n);
}

static int
snf_setfilter(pcap_t *p, struct bpf_program *fp)
{
	if (!p)
		return -1;
	if (!fp) {
		strncpy(p->errbuf, "setfilter: No filter specified",
			sizeof(p->errbuf));
		return -1;
	}

	/* Make our private copy of the filter */

	if (install_bpf_program(p, fp) < 0)
		return -1;

	p->md.use_bpf = 0;

	return (0);
}

static int
snf_inject(pcap_t *p, const void *buf _U_, size_t size _U_)
{
	strlcpy(p->errbuf, "Sending packets isn't supported with snf",
	    PCAP_ERRBUF_SIZE);
	return (-1);
}

static int
snf_activate(pcap_t* p)
{
	char *device = p->opt.source;
	const char *nr = NULL;
	int err;
	int flags = 0;

	if (device == NULL) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE,
			 "device is NULL: %s", pcap_strerror(errno));
		return -1;
	}

	/* In Libpcap, we set pshared by default if NUM_RINGS is set to > 1.
	 * Since libpcap isn't thread-safe */
	if ((nr = getenv("SNF_NUM_RINGS")) && *nr && atoi(nr) > 1)
		flags |= SNF_F_PSHARED;
	else
		nr = NULL;

	err = snf_open(p->md.snf_boardnum,
			0, /* let SNF API parse SNF_NUM_RINGS, if set */
			NULL, /* default RSS, or use SNF_RSS_FLAGS env */
			0, /* default to SNF_DATARING_SIZE from env */
			flags, /* may want pshared */
			&p->md.snf_handle);
	if (err != 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE,
			 "snf_open failed: %s", pcap_strerror(err));
		return -1;
	}

	err = snf_ring_open(p->md.snf_handle, &p->md.snf_ring);
	if (err != 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE,
			 "snf_ring_open failed: %s", pcap_strerror(err));
		return -1;
	}

	if (p->md.timeout <= 0)
		p->md.snf_timeout = -1;
	else
		p->md.snf_timeout = p->md.timeout;

	err = snf_start(p->md.snf_handle);
	if (err != 0) {
		snprintf(p->errbuf, PCAP_ERRBUF_SIZE,
			 "snf_start failed: %s", pcap_strerror(err));
		return -1;
	}

	/*
	 * "select()" and "poll()" don't work on snf descriptors.
	 */
	p->selectable_fd = -1;
	p->linktype = DLT_EN10MB;
	p->read_op = snf_read;
	p->inject_op = snf_inject;
	p->setfilter_op = snf_setfilter;
	p->setdirection_op = NULL; /* Not implemented.*/
	p->set_datalink_op = snf_set_datalink;
	p->getnonblock_op = snf_getnonblock;
	p->setnonblock_op = snf_setnonblock;
	p->stats_op = snf_pcap_stats;
	p->cleanup_op = snf_platform_cleanup;
	p->md.stat.ps_recv = 0;
	p->md.stat.ps_drop = 0;
	p->md.stat.ps_ifdrop = 0;
	return 0;
}

int
snf_platform_finddevs(pcap_if_t **devlistp, char *errbuf)
{
	/*
	 * There are no platform-specific devices since each device
	 * exists as a regular Ethernet device.
	 */
	return 0;
}

pcap_t *
snf_create(const char *device, char *ebuf)
{
	pcap_t *p;
	int boardnum = -1;
	struct snf_ifaddrs *ifaddrs, *ifa;
	size_t devlen;

	if (snf_init(SNF_VERSION_API))
		return NULL;

	/*
	 * Match a given interface name to our list of interface names, from
	 * which we can obtain the intended board number
	 */
	if (snf_getifaddrs(&ifaddrs) || ifaddrs == NULL)
		return NULL;
	devlen = strlen(device) + 1;
	ifa = ifaddrs;
	while (ifa) {
		if (!strncmp(device, ifa->snf_ifa_name, devlen)) {
			boardnum = ifa->snf_ifa_boardnum;
			break;
		}
		ifa = ifa->snf_ifa_next;
	}
	snf_freeifaddrs(ifaddrs);

	if (ifa == NULL) {
		/*
		 * If we can't find the device by name, support the name "snfX"
		 * and "snf10gX" where X is the board number.
		 */
		if (sscanf(device, "snf10g%d", &boardnum) != 1 &&
		    sscanf(device, "snf%d", &boardnum) != 1)
			return NULL;
	}

	p = pcap_create_common(device, ebuf);
	if (p == NULL)
		return NULL;

	p->activate_op = snf_activate;
	p->md.snf_boardnum = boardnum;
	return p;
}