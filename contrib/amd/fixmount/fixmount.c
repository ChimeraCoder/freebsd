
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>

#define CREATE_TIMEOUT	2	/* seconds */
#define CALL_TIMEOUT	5	/* seconds */

/* Constant defs */
#define	ALL		1
#define	DIRS		2

#define	DODUMP		0x1
#define	DOEXPORTS	0x2
#define	DOREMOVE	0x4
#define	DOVERIFY	0x8
#define	DOREMALL	0x10

extern int fixmount_check_mount(char *host, struct in_addr hostaddr, char *path);

static char dir_path[NFS_MAXPATHLEN];
static char localhost[] = "localhost";
static char thishost[MAXHOSTNAMELEN + 1] = "";
static exports mntexports;
static int quiet = 0;
static int type = 0;
static jmp_buf before_rpc;
static mountlist mntdump;
static struct in_addr thisaddr;
static CLIENT *clnt_create_timeout(char *, struct timeval *);

RETSIGTYPE create_timeout(int);
int is_same_host(char *, char *, struct in_addr);
int main(int, char *[]);
int remove_all(CLIENT *, char *);
int remove_mount(CLIENT *, char *, mountlist, int);
void fix_rmtab(CLIENT *, char *, mountlist, int, int);
void print_dump(mountlist);
void usage(void);


void
usage(void)
{
  fprintf(stderr, "usage: fixmount [-adervAqf] [-h hostname] host ...\n");
  exit(1);
}


/*
 * Check hostname against other name and its IP address
 */
int
is_same_host(char *name1, char *name2, struct in_addr addr2)
{
  if (strcasecmp(name1, name2) == 0) {
    return 1;
  } else if (addr2.s_addr == INADDR_NONE) {
    return 0;
  } else {
    static char lasthost[MAXHOSTNAMELEN] = "";
    static struct in_addr addr1;
    struct hostent *he;

    /*
     * To save nameserver lookups, and because this function
     * is typically called repeatedly on the same names,
     * cache the last lookup result and reuse it if possible.
     */
    if (strcasecmp(name1, lasthost) == 0) {
      return (addr1.s_addr == addr2.s_addr);
    } else if (!(he = gethostbyname(name1))) {
      return 0;
    } else {
      xstrlcpy(lasthost, name1, MAXHOSTNAMELEN);
      memcpy(&addr1, he->h_addr, sizeof(addr1));
      return (addr1.s_addr == addr2.s_addr);
    }
  }
}


/*
 * Print the binary tree in inorder so that output is sorted.
 */
void
print_dump(mountlist mp)
{
  if (mp == NULL)
    return;
  if (is_same_host(mp->ml_hostname, thishost, thisaddr)) {
    switch (type) {
    case ALL:
      printf("%s:%s\n", mp->ml_hostname, mp->ml_directory);
      break;
    case DIRS:
      printf("%s\n", mp->ml_directory);
      break;
    default:
      printf("%s\n", mp->ml_hostname);
      break;
    };
  }
  if (mp->ml_next)
    print_dump(mp->ml_next);
}


/*
 * remove entry from remote rmtab
 */
int
remove_mount(CLIENT *client, char *host, mountlist ml, int fixit)
{
  enum clnt_stat estat;
  struct timeval tv;
  char *pathp = dir_path;

  xstrlcpy(dir_path, ml->ml_directory, sizeof(dir_path));

  if (!fixit) {
    printf("%s: bogus mount %s:%s\n", host, ml->ml_hostname, ml->ml_directory);
    fflush(stdout);
  } else {
    printf("%s: removing %s:%s\n", host, ml->ml_hostname, ml->ml_directory);
    fflush(stdout);

    tv.tv_sec = CALL_TIMEOUT;
    tv.tv_usec = 0;

    if ((estat = clnt_call(client,
			   MOUNTPROC_UMNT,
			   (XDRPROC_T_TYPE) xdr_dirpath,
			   (char *) &pathp,
			   (XDRPROC_T_TYPE) xdr_void,
			   (char *) 0,
			   tv)) != RPC_SUCCESS) {
      fprintf(stderr, "%s:%s MOUNTPROC_UMNT: ",
	      host, ml->ml_directory);
      clnt_perrno(estat);
      fflush(stderr);
      return -1;
    }
  }
  return 0;
}


/*
 * fix mount list on remote host
 */
void
fix_rmtab(CLIENT *client, char *host, mountlist mp, int fixit, int force)
{
  mountlist p;
  struct hostent *he;
  struct in_addr hostaddr;

  /*
   * Obtain remote address for comparisons
   */
  if ((he = gethostbyname(host))) {
    memcpy(&hostaddr, he->h_addr, sizeof(hostaddr));
  } else {
    hostaddr.s_addr = INADDR_NONE;
  }

  for (p = mp; p; p = p->ml_next) {
    if (is_same_host(p->ml_hostname, thishost, thisaddr)) {
      if (force || !fixmount_check_mount(host, hostaddr, p->ml_directory))
	remove_mount(client, host, p, fixit);
    }
  }
}


/*
 * remove all entries from remote rmtab
 */
int
remove_all(CLIENT *client, char *host)
{
  enum clnt_stat estat;
  struct timeval tv;

  printf("%s: removing ALL\n", host);
  fflush(stdout);

  tv.tv_sec = CALL_TIMEOUT;
  tv.tv_usec = 0;

  if ((estat = clnt_call(client,
			 MOUNTPROC_UMNTALL,
			 (XDRPROC_T_TYPE) xdr_void,
			 (char *) 0,
			 (XDRPROC_T_TYPE) xdr_void,
			 (char *) 0,
			 tv)) != RPC_SUCCESS) {
    /*
     * RPC_SYSTEMERROR is returned even if all went well
     */
    if (estat != RPC_SYSTEMERROR) {
      fprintf(stderr, "%s MOUNTPROC_UMNTALL: ", host);
      clnt_perrno(estat);
      fflush(stderr);
      return -1;
    }
  }

  return 0;
}


/*
 * This command queries the NFS mount daemon for it's mount list and/or
 * it's exports list and prints them out.
 * See "NFS: Network File System Protocol Specification, RFC1094, Appendix A"
 * for detailed information on the protocol.
 */
int
main(int argc, char *argv[])
{
  AUTH *auth;
  CLIENT *client;
  char *host;
  enum clnt_stat estat;
  exports exp;
  extern char *optarg;
  extern int optind;
  groups grp;
  int ch;
  int force = 0;
  int morethanone;
  register int rpcs = 0;
  struct timeval tv;

  while ((ch = getopt(argc, argv, "adervAqfh:")) != -1)
    switch ((char) ch) {

    case 'a':
      if (type == 0) {
	type = ALL;
	rpcs |= DODUMP;
      } else
	usage();
      break;

    case 'd':
      if (type == 0) {
	type = DIRS;
	rpcs |= DODUMP;
      } else
	usage();
      break;

    case 'e':
      rpcs |= DOEXPORTS;
      break;

    case 'r':
      rpcs |= DOREMOVE;
      break;

    case 'A':
      rpcs |= DOREMALL;
      break;

    case 'v':
      rpcs |= DOVERIFY;
      break;

    case 'q':
      quiet = 1;
      break;

    case 'f':
      force = 1;
      break;

    case 'h':
      xstrlcpy(thishost, optarg, sizeof(thishost));
      break;

    case '?':
    default:
      usage();
    }

  if (optind == argc)
    usage();

  if (rpcs == 0)
    rpcs = DODUMP;

  if (!*thishost) {
    struct hostent *he;

    if (gethostname(thishost, sizeof(thishost)) < 0) {
      perror("gethostname");
      exit(1);
    }
    thishost[sizeof(thishost) - 1] = '\0';

    /*
     * We need the hostname as it appears to the other side's
     * mountd, so get our own hostname by reverse address
     * resolution.
     */
    if (!(he = gethostbyname(thishost))) {
      fprintf(stderr, "gethostbyname failed on %s\n",
	      thishost);
      exit(1);
    }
    memcpy(&thisaddr, he->h_addr, sizeof(thisaddr));
    if (!(he = gethostbyaddr((char *) &thisaddr, sizeof(thisaddr),
			     he->h_addrtype))) {
      fprintf(stderr, "gethostbyaddr failed on %s\n",
	      inet_ntoa(thisaddr));
      exit(1);
    }
    xstrlcpy(thishost, he->h_name, sizeof(thishost));
  } else {
    thisaddr.s_addr = INADDR_NONE;
  }

  if (!(auth = authunix_create_default())) {
    fprintf(stderr, "couldn't create authentication handle\n");
    exit(1);
  }
  morethanone = (optind + 1 < argc);

  for (; optind < argc; optind++) {

    host = argv[optind];
    tv.tv_sec = CREATE_TIMEOUT;
    tv.tv_usec = 0;

    if (!(client = clnt_create_timeout(host, &tv)))
        continue;

    client->cl_auth = auth;
    tv.tv_sec = CALL_TIMEOUT;
    tv.tv_usec = 0;

    if (rpcs & (DODUMP | DOREMOVE | DOVERIFY))
      if ((estat = clnt_call(client,
			     MOUNTPROC_DUMP,
			     (XDRPROC_T_TYPE) xdr_void,
			     (char *) 0,
			     (XDRPROC_T_TYPE) xdr_mountlist,
			     (char *) &mntdump,
			     tv)) != RPC_SUCCESS) {
	fprintf(stderr, "%s: MOUNTPROC_DUMP: ", host);
	clnt_perrno(estat);
	fflush(stderr);
	mntdump = NULL;
	goto next;
      }
    if (rpcs & DOEXPORTS)
      if ((estat = clnt_call(client,
			     MOUNTPROC_EXPORT,
			     (XDRPROC_T_TYPE) xdr_void,
			     (char *) 0,
			     (XDRPROC_T_TYPE) xdr_exports,
			     (char *) &mntexports,
			     tv)) != RPC_SUCCESS) {
	fprintf(stderr, "%s: MOUNTPROC_EXPORT: ", host);
	clnt_perrno(estat);
	fflush(stderr);
	mntexports = NULL;
	goto next;
      }

    /* Now just print out the results */
    if ((rpcs & (DODUMP | DOEXPORTS)) &&
	morethanone) {
      printf(">>> %s <<<\n", host);
      fflush(stdout);
    }

    if (rpcs & DODUMP) {
      print_dump(mntdump);
    }

    if (rpcs & DOEXPORTS) {
      exp = mntexports;
      while (exp) {
	printf("%-35s", exp->ex_dir);
	grp = exp->ex_groups;
	if (grp == NULL) {
	  printf("Everyone\n");
	} else {
	  while (grp) {
	    printf("%s ", grp->gr_name);
	    grp = grp->gr_next;
	  }
	  printf("\n");
	}
	exp = exp->ex_next;
      }
    }

    if (rpcs & DOVERIFY)
      fix_rmtab(client, host, mntdump, 0, force);

    if (rpcs & DOREMOVE)
      fix_rmtab(client, host, mntdump, 1, force);

    if (rpcs & DOREMALL)
      remove_all(client, host);

  next:
    if (mntdump)
      (void) clnt_freeres(client,
			  (XDRPROC_T_TYPE) xdr_mountlist,
			  (char *) &mntdump);
    if (mntexports)
      (void) clnt_freeres(client,
			  (XDRPROC_T_TYPE) xdr_exports,
			  (char *) &mntexports);

    clnt_destroy(client);
  }
  exit(0);
  return 0; /* should never reach here */
}


RETSIGTYPE
create_timeout(int sig)
{
  signal(SIGALRM, SIG_DFL);
  longjmp(before_rpc, 1);
}


#ifndef HAVE_TRANSPORT_TYPE_TLI
/*
 * inetresport creates a datagram socket and attempts to bind it to a
 * secure port.
 * returns: The bound socket, or -1 to indicate an error.
 */
static int
inetresport(int ty)
{
  int alport;
  struct sockaddr_in addr;
  int fd;

  memset(&addr, 0, sizeof(addr));
  /* as per POSIX, sin_len need not be set (used internally by kernel) */

  addr.sin_family = AF_INET;	/* use internet address family */
  addr.sin_addr.s_addr = INADDR_ANY;
  if ((fd = socket(AF_INET, ty, 0)) < 0)
    return -1;

  for (alport = IPPORT_RESERVED - 1; alport > IPPORT_RESERVED / 2 + 1; alport--) {
    addr.sin_port = htons((u_short) alport);
    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) >= 0)
      return fd;
    if (errno != EADDRINUSE) {
      close(fd);
      return -1;
    }
  }
  close(fd);
  errno = EAGAIN;
  return -1;
}


/*
 * Privsock() calls inetresport() to attempt to bind a socket to a secure
 * port.  If inetresport() fails, privsock returns a magic socket number which
 * indicates to RPC that it should make its own socket.
 * returns: A privileged socket # or RPC_ANYSOCK.
 */
static int
privsock(int ty)
{
  int sock = inetresport(ty);

  if (sock < 0) {
    errno = 0;
    /* Couldn't get a secure port, let RPC make an insecure one */
    sock = RPC_ANYSOCK;
  }
  return sock;
}
#endif /* not HAVE_TRANSPORT_TYPE_TLI */


static CLIENT *
clnt_create_timeout(char *host, struct timeval *tvp)
{
  CLIENT *clnt;
  struct sockaddr_in host_addr;
  struct hostent *hp;
#ifndef HAVE_TRANSPORT_TYPE_TLI
  int s;
#endif /* not HAVE_TRANSPORT_TYPE_TLI */

  if (setjmp(before_rpc)) {
    if (!quiet) {
      fprintf(stderr, "%s: ", host);
      clnt_perrno(RPC_TIMEDOUT);
      fprintf(stderr, "\n");
      fflush(stderr);
    }
    return NULL;
  }
  signal(SIGALRM, create_timeout);
  ualarm(tvp->tv_sec * 1000000 + tvp->tv_usec, 0);

  /*
   * Get address of host
   */
  if ((hp = gethostbyname(host)) == 0 && !STREQ(host, localhost)) {
    fprintf(stderr, "can't get address of %s\n", host);
    return NULL;
  }
  memset(&host_addr, 0, sizeof(host_addr));
  /* as per POSIX, sin_len need not be set (used internally by kernel) */
  host_addr.sin_family = AF_INET;
  if (hp) {
    memmove((voidp) &host_addr.sin_addr, (voidp) hp->h_addr,
	    sizeof(host_addr.sin_addr));
  } else {
    /* fake "localhost" */
    host_addr.sin_addr.s_addr = htonl(0x7f000001);
  }

#ifdef HAVE_TRANSPORT_TYPE_TLI
  /* try TCP first (in case return data is large), then UDP */
  clnt = clnt_create(host, MOUNTPROG, MOUNTVERS, "tcp");
  if (!clnt)
    clnt = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");
#else /* not HAVE_TRANSPORT_TYPE_TLI */
  s = RPC_ANYSOCK;
  clnt = clnttcp_create(&host_addr, MOUNTPROG, MOUNTVERS, &s, 0, 0);
  if (!clnt) {
    /* XXX: do we need to close(s) ? */
    s = privsock(SOCK_DGRAM);
    clnt = clntudp_create(&host_addr, MOUNTPROG, MOUNTVERS, *tvp, &s);
  }
#endif /* not HAVE_TRANSPORT_TYPE_TLI */

  if (!clnt) {
    ualarm(0, 0);
    if (!quiet) {
      clnt_pcreateerror(host);
      fflush(stderr);
    }
    return NULL;
  }

  ualarm(0, 0);
  return clnt;
}