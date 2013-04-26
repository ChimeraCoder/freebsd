
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
#include <amd.h>

/* typedefs */
typedef char *(*amqsvcproc_t)(voidp, struct svc_req *);

#if defined(HAVE_TCPD_H) && defined(HAVE_LIBWRAP)
# ifdef NEED_LIBWRAP_SEVERITY_VARIABLES
/*
 * Some systems that define libwrap already define these two variables
 * in libwrap, while others don't: so I need to know precisely iff
 * to define these two severity variables.
 */
int allow_severity=0, deny_severity=0;
# endif /* NEED_LIBWRAP_SEVERITY_VARIABLES */

/*
 * check if remote amq is authorized to access this amd.
 * Returns: 1=allowed, 0=denied.
 */
static int
amqsvc_is_client_allowed(const struct sockaddr_in *addr, char *remote)
{
  struct hostent *h;
  char *name = NULL, **ad;
  int ret = 0;			/* default is 0==denied */

  /* Check IP address */
  if (hosts_ctl(AMD_SERVICE_NAME, "", remote, "")) {
    ret = 1;
    goto out;
  }
  /* Get address */
  if (!(h = gethostbyaddr((const char *)&(addr->sin_addr),
                          sizeof(addr->sin_addr),
                          AF_INET)))
    goto out;
  if (!(name = strdup(h->h_name)))
    goto out;
  /* Paranoia check */
  if (!(h = gethostbyname(name)))
    goto out;
  for (ad = h->h_addr_list; *ad; ad++)
    if (!memcmp(*ad, &(addr->sin_addr), h->h_length))
      break;
  if (!*ad)
    goto out;
  if (hosts_ctl(AMD_SERVICE_NAME, "", h->h_name, "")) {
    return 1;
    goto out;
  }
  /* Check aliases */
  for (ad = h->h_aliases; *ad; ad++)
    if (hosts_ctl(AMD_SERVICE_NAME, "", *ad, "")) {
      return 1;
      goto out;
    }

 out:
  if (name)
    XFREE(name);
  return ret;
}
#endif /* defined(HAVE_TCPD_H) && defined(HAVE_LIBWRAP) */


void
amq_program_1(struct svc_req *rqstp, SVCXPRT *transp)
{
  union {
    amq_string amqproc_mnttree_1_arg;
    amq_string amqproc_umnt_1_arg;
    amq_setopt amqproc_setopt_1_arg;
  } argument;
  char *result;
  xdrproc_t xdr_argument, xdr_result;
  amqsvcproc_t local;

#if defined(HAVE_TCPD_H) && defined(HAVE_LIBWRAP)
  if (gopt.flags & CFM_USE_TCPWRAPPERS) {
    struct sockaddr_in *remote_addr = svc_getcaller(rqstp->rq_xprt);
    char *remote_hostname = inet_ntoa(remote_addr->sin_addr);

    if (!amqsvc_is_client_allowed(remote_addr, remote_hostname)) {
      plog(XLOG_WARNING, "Amd denied remote amq service to %s", remote_hostname);
      svcerr_auth(transp, AUTH_FAILED);
      return;
    } else {
      dlog("Amd allowed remote amq service to %s", remote_hostname);
    }
  }
#endif /* defined(HAVE_TCPD_H) && defined(HAVE_LIBWRAP) */

  switch (rqstp->rq_proc) {

  case AMQPROC_NULL:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_void;
    local = (amqsvcproc_t) amqproc_null_1_svc;
    break;

  case AMQPROC_MNTTREE:
    xdr_argument = (xdrproc_t) xdr_amq_string;
    xdr_result = (xdrproc_t) xdr_amq_mount_tree_p;
    local = (amqsvcproc_t) amqproc_mnttree_1_svc;
    break;

  case AMQPROC_UMNT:
    xdr_argument = (xdrproc_t) xdr_amq_string;
    xdr_result = (xdrproc_t) xdr_void;
    local = (amqsvcproc_t) amqproc_umnt_1_svc;
    break;

  case AMQPROC_STATS:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_amq_mount_stats;
    local = (amqsvcproc_t) amqproc_stats_1_svc;
    break;

  case AMQPROC_EXPORT:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_amq_mount_tree_list;
    local = (amqsvcproc_t) amqproc_export_1_svc;
    break;

  case AMQPROC_SETOPT:
    xdr_argument = (xdrproc_t) xdr_amq_setopt;
    xdr_result = (xdrproc_t) xdr_int;
    local = (amqsvcproc_t) amqproc_setopt_1_svc;
    break;

  case AMQPROC_GETMNTFS:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_amq_mount_info_qelem;
    local = (amqsvcproc_t) amqproc_getmntfs_1_svc;
    break;

  case AMQPROC_GETVERS:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_amq_string;
    local = (amqsvcproc_t) amqproc_getvers_1_svc;
    break;

  case AMQPROC_GETPID:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_int;
    local = (amqsvcproc_t) amqproc_getpid_1_svc;
    break;

  case AMQPROC_PAWD:
    xdr_argument = (xdrproc_t) xdr_amq_string;
    xdr_result = (xdrproc_t) xdr_amq_string;
    local = (amqsvcproc_t) amqproc_pawd_1_svc;
    break;

  default:
    svcerr_noproc(transp);
    return;
  }

  memset((char *) &argument, 0, sizeof(argument));
  if (!svc_getargs(transp,
		   (XDRPROC_T_TYPE) xdr_argument,
		   (SVC_IN_ARG_TYPE) & argument)) {
    svcerr_decode(transp);
    return;
  }

  result = (*local) (&argument, rqstp);

  if (result != NULL && !svc_sendreply(transp,
				       (XDRPROC_T_TYPE) xdr_result,
				       result)) {
    svcerr_systemerr(transp);
  }

  if (!svc_freeargs(transp,
		    (XDRPROC_T_TYPE) xdr_argument,
		    (SVC_IN_ARG_TYPE) & argument)) {
    plog(XLOG_FATAL, "unable to free rpc arguments in amqprog_1");
    going_down(1);
  }
}