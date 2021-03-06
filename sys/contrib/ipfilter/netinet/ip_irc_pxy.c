
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

#define	IPF_IRC_PROXY

#define	IPF_IRCBUFSZ	96	/* This *MUST* be >= 64! */


int ippr_irc_init __P((void));
void ippr_irc_fini __P((void));
int ippr_irc_new __P((fr_info_t *, ap_session_t *, nat_t *));
int ippr_irc_out __P((fr_info_t *, ap_session_t *, nat_t *));
int ippr_irc_send __P((fr_info_t *, nat_t *));
int ippr_irc_complete __P((ircinfo_t *, char *, size_t));
u_short ipf_irc_atoi __P((char **));

static	frentry_t	ircnatfr;

int	irc_proxy_init = 0;


/*
 * Initialize local structures.
 */
int ippr_irc_init()
{
	bzero((char *)&ircnatfr, sizeof(ircnatfr));
	ircnatfr.fr_ref = 1;
	ircnatfr.fr_flags = FR_INQUE|FR_PASS|FR_QUICK|FR_KEEPSTATE;
	MUTEX_INIT(&ircnatfr.fr_lock, "IRC proxy rule lock");
	irc_proxy_init = 1;

	return 0;
}


void ippr_irc_fini()
{
	if (irc_proxy_init == 1) {
		MUTEX_DESTROY(&ircnatfr.fr_lock);
		irc_proxy_init = 0;
	}
}


const char *ippr_irc_dcctypes[] = {
	"CHAT ",	/* CHAT chat ipnumber portnumber */
	"SEND ",	/* SEND filename ipnumber portnumber */
	"MOVE ",
	"TSEND ",
	"SCHAT ",
	NULL,
};


/*
 * :A PRIVMSG B :^ADCC CHAT chat 0 0^A\r\n
 * PRIVMSG B ^ADCC CHAT chat 0 0^A\r\n
 */


int ippr_irc_complete(ircp, buf, len)
ircinfo_t *ircp;
char *buf;
size_t len;
{
	register char *s, c;
	register size_t i;
	u_32_t l;
	int j, k;

	ircp->irc_ipnum = 0;
	ircp->irc_port = 0;

	if (len < 31)
		return 0;
	s = buf;
	c = *s++;
	i = len - 1;

	if ((c != ':') && (c != 'P'))
		return 0;

	if (c == ':') {
		/*
		 * Loosely check that the source is a nickname of some sort
		 */
		s++;
		c = *s;
		ircp->irc_snick = s;
		if (!ISALPHA(c))
			return 0;
		i--;
		for (c = *s; !ISSPACE(c) && (i > 0); i--)
			c = *s++;
		if (i < 31)
			return 0;
		if (c != 'P')
			return 0;
	} else
		ircp->irc_snick = NULL;

	/*
	 * Check command string
	 */
	if (strncmp(s, "PRIVMSG ", 8))
		return 0;
	i -= 8;
	s += 8;
	c = *s;
	ircp->irc_dnick = s;

	/*
	 * Loosely check that the destination is a nickname of some sort
	 */
	if (!ISALPHA(c))
		return 0;
	for (; !ISSPACE(c) && (i > 0); i--)
		c = *s++;
	if (i < 20)
		return 0;
	s++,
	i--;

	/*
	 * Look for a ^A to start the DCC
	 */
	c = *s;
	if (c == ':') {
		s++;
		c = *s;
	}

	if (strncmp(s, "\001DCC ", 4))
		return 0;

	i -= 4;
	s += 4;

	/*
	 * Check for a recognised DCC command
	 */
	for (j = 0, k = 0; ippr_irc_dcctypes[j]; j++) {
		k = MIN(strlen(ippr_irc_dcctypes[j]), i);
		if (!strncmp(ippr_irc_dcctypes[j], s, k))
			break;
	}
	if (!ippr_irc_dcctypes[j])
		return 0;

	ircp->irc_type = s;
	i -= k;
	s += k;

	if (i < 11)
		return 0;

	/*
	 * Check for the arg
	 */
	c = *s;
	if (ISSPACE(c))
		return 0;
	ircp->irc_arg = s;
	for (; (c != ' ') && (c != '\001') && (i > 0); i--)
		c = *s++;

	if (c == '\001')	/* In reality a ^A can quote another ^A...*/
		return 0;

	if (i < 5)
		return 0;

	s++;
	i--;
	c = *s;
	if (!ISDIGIT(c))
		return 0;
	ircp->irc_addr = s;
	/*
	 * Get the IP#
	 */
	for (l = 0; ISDIGIT(c) && (i > 0); i--) {
		l *= 10;
		l += c - '0';
		c = *s++;
	}

	if (i < 4)
		return 0;

	if (c != ' ')
		return 0;

	ircp->irc_ipnum = l;
	s++;
	i--;
	c = *s;
	if (!ISDIGIT(c))
		return 0;
	/*
	 * Get the port#
	 */
	for (l = 0; ISDIGIT(c) && (i > 0); i--) {
		l *= 10;
		l += c - '0';
		c = *s++;
	}
	if (i < 3)
		return 0;
	if (strncmp(s, "\001\r\n", 3))
		return 0;
	s += 3;
	ircp->irc_len = s - buf;
	ircp->irc_port = l;
	return 1;
}


int ippr_irc_new(fin, aps, nat)
fr_info_t *fin;
ap_session_t *aps;
nat_t *nat;
{
	ircinfo_t *irc;

	KMALLOC(irc, ircinfo_t *);
	if (irc == NULL)
		return -1;

	fin = fin;	/* LINT */
	nat = nat;	/* LINT */

	aps->aps_data = irc;
	aps->aps_psiz = sizeof(ircinfo_t);

	bzero((char *)irc, sizeof(*irc));
	return 0;
}


int ippr_irc_send(fin, nat)
fr_info_t *fin;
nat_t *nat;
{
	char ctcpbuf[IPF_IRCBUFSZ], newbuf[IPF_IRCBUFSZ];
	tcphdr_t *tcp, tcph, *tcp2 = &tcph;
	int off, inc = 0, i, dlen;
	size_t nlen = 0, olen;
	struct in_addr swip;
	u_short a5, sp;
	ircinfo_t *irc;
	fr_info_t fi;
	nat_t *nat2;
	u_int a1;
	ip_t *ip;
	mb_t *m;
#ifdef	MENTAT
	mb_t *m1;
#endif

	m = fin->fin_m;
	ip = fin->fin_ip;
	tcp = (tcphdr_t *)fin->fin_dp;
	bzero(ctcpbuf, sizeof(ctcpbuf));
	off = (char *)tcp - (char *)ip + (TCP_OFF(tcp) << 2) + fin->fin_ipoff;

#ifdef __sgi
	dlen = fin->fin_plen - off;
#else
	dlen = MSGDSIZE(m) - off;
#endif
	if (dlen <= 0)
		return 0;
	COPYDATA(m, off, MIN(sizeof(ctcpbuf), dlen), ctcpbuf);

	if (dlen <= 0)
		return 0;
	ctcpbuf[sizeof(ctcpbuf) - 1] = '\0';
	*newbuf = '\0';

	irc = nat->nat_aps->aps_data;
	if (ippr_irc_complete(irc, ctcpbuf, dlen) == 0)
		return 0;

	/*
	 * check that IP address in the PORT/PASV reply is the same as the
	 * sender of the command - prevents using PORT for port scanning.
	 */
	if (irc->irc_ipnum != ntohl(nat->nat_inip.s_addr))
		return 0;

	a5 = irc->irc_port;

	/*
	 * Calculate new address parts for the DCC command
	 */
	a1 = ntohl(ip->ip_src.s_addr);
	olen = irc->irc_len;
	i = irc->irc_addr - ctcpbuf;
	i++;
	(void) strncpy(newbuf, ctcpbuf, i);
	/* DO NOT change these! */
#if defined(SNPRINTF) && defined(KERNEL)
	SNPRINTF(newbuf, sizeof(newbuf) - i, "%u %u\001\r\n", a1, a5);
#else
	(void) sprintf(newbuf, "%u %u\001\r\n", a1, a5);
#endif

	nlen = strlen(newbuf);
	inc = nlen - olen;

	if ((inc + ip->ip_len) > 65535)
		return 0;

#ifdef	MENTAT
	for (m1 = m; m1->b_cont; m1 = m1->b_cont)
		;
	if ((inc > 0) && (m1->b_datap->db_lim - m1->b_wptr < inc)) {
		mblk_t *nm;

		/* alloc enough to keep same trailer space for lower driver */
		nm = allocb(nlen, BPRI_MED);
		PANIC((!nm),("ippr_irc_out: allocb failed"));

		nm->b_band = m1->b_band;
		nm->b_wptr += nlen;

		m1->b_wptr -= olen;
		PANIC((m1->b_wptr < m1->b_rptr),
		      ("ippr_irc_out: cannot handle fragmented data block"));

		linkb(m1, nm);
	} else {
# if SOLARIS && defined(ICK_VALID)
		if (m1->b_datap->db_struiolim == m1->b_wptr)
			m1->b_datap->db_struiolim += inc;
		m1->b_datap->db_struioflag &= ~STRUIO_IP;
# endif
		m1->b_wptr += inc;
	}
#else
	if (inc < 0)
		m_adj(m, inc);
	/* the mbuf chain will be extended if necessary by m_copyback() */
#endif
	COPYBACK(m, off, nlen, newbuf);

	if (inc != 0) {
#if defined(MENTAT) || defined(__sgi)
		register u_32_t	sum1, sum2;

		sum1 = ip->ip_len;
		sum2 = ip->ip_len + inc;

		/* Because ~1 == -2, We really need ~1 == -1 */
		if (sum1 > sum2)
			sum2--;
		sum2 -= sum1;
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);

		fix_outcksum(fin, &ip->ip_sum, sum2);
#endif
		ip->ip_len += inc;
	}

	/*
	 * Add skeleton NAT entry for connection which will come back the
	 * other way.
	 */
	sp = htons(a5);
	/*
	 * Don't allow the PORT command to specify a port < 1024 due to
	 * security crap.
	 */
	if (ntohs(sp) < 1024)
		return 0;

	/*
	 * The server may not make the connection back from port 20, but
	 * it is the most likely so use it here to check for a conflicting
	 * mapping.
	 */
	bcopy((caddr_t)fin, (caddr_t)&fi, sizeof(fi));
	fi.fin_data[0] = sp;
	fi.fin_data[1] = fin->fin_data[1];
	nat2 = nat_outlookup(fin, IPN_TCP, nat->nat_p, nat->nat_inip,
			     ip->ip_dst);
	if (nat2 == NULL) {
		bcopy((caddr_t)fin, (caddr_t)&fi, sizeof(fi));
		bzero((char *)tcp2, sizeof(*tcp2));
		tcp2->th_win = htons(8192);
		tcp2->th_sport = sp;
		tcp2->th_dport = 0; /* XXX - don't specify remote port */
		fi.fin_state = NULL;
		fi.fin_nat = NULL;
		fi.fin_data[0] = ntohs(sp);
		fi.fin_data[1] = 0;
		fi.fin_dp = (char *)tcp2;
		fi.fin_fr = &ircnatfr;
		fi.fin_dlen = sizeof(*tcp2);
		fi.fin_plen = fi.fin_hlen + sizeof(*tcp2);
		swip = ip->ip_src;
		ip->ip_src = nat->nat_inip;
		nat2 = nat_new(&fi, nat->nat_ptr, NULL,
			       NAT_SLAVE|IPN_TCP|SI_W_DPORT, NAT_OUTBOUND);
		if (nat2 != NULL) {
			(void) nat_proto(&fi, nat2, 0);
			nat_update(&fi, nat2, nat2->nat_ptr);

			(void) fr_addstate(&fi, NULL, SI_W_DPORT);
			if (fi.fin_state != NULL)
				fr_statederef((ipstate_t **)&fi.fin_state);
		}
		ip->ip_src = swip;
	}
	return inc;
}


int ippr_irc_out(fin, aps, nat)
fr_info_t *fin;
ap_session_t *aps;
nat_t *nat;
{
	aps = aps;	/* LINT */
	return ippr_irc_send(fin, nat);
}