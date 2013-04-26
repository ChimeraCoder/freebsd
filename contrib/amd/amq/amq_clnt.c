
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
#include <amq.h>


static struct timeval TIMEOUT = {ALLOWED_MOUNT_TIME, 0};


voidp
amqproc_null_1(voidp argp, CLIENT *clnt)
{
  static char res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_NULL,
		(XDRPROC_T_TYPE) xdr_void, argp,
		(XDRPROC_T_TYPE) xdr_void, &res, TIMEOUT)
      != RPC_SUCCESS) {
    return (NULL);
  }
  return ((voidp) &res);
}


amq_mount_tree_p *
amqproc_mnttree_1(amq_string *argp, CLIENT *clnt)
{
  static amq_mount_tree_p res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_MNTTREE,
		(XDRPROC_T_TYPE) xdr_amq_string, (SVC_IN_ARG_TYPE) argp,
		(XDRPROC_T_TYPE) xdr_amq_mount_tree_p, (SVC_IN_ARG_TYPE) &res,
		TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}


voidp
amqproc_umnt_1(amq_string *argp, CLIENT *clnt)
{
  static char res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_UMNT,
		(XDRPROC_T_TYPE) xdr_amq_string, (SVC_IN_ARG_TYPE) argp,
		(XDRPROC_T_TYPE) xdr_void, &res,
		TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return ((voidp) &res);
}


amq_mount_stats *
amqproc_stats_1(voidp argp, CLIENT *clnt)
{
  static amq_mount_stats res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_STATS,
		(XDRPROC_T_TYPE) xdr_void, argp,
		(XDRPROC_T_TYPE) xdr_amq_mount_stats,
		(SVC_IN_ARG_TYPE) &res,
		TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}


amq_mount_tree_list *
amqproc_export_1(voidp argp, CLIENT *clnt)
{
  static amq_mount_tree_list res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_EXPORT,
		(XDRPROC_T_TYPE) xdr_void, argp,
		(XDRPROC_T_TYPE) xdr_amq_mount_tree_list,
		(SVC_IN_ARG_TYPE) &res, TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}


int *
amqproc_setopt_1(amq_setopt *argp, CLIENT *clnt)
{
  static int res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_SETOPT, (XDRPROC_T_TYPE) xdr_amq_setopt,
		(SVC_IN_ARG_TYPE) argp, (XDRPROC_T_TYPE) xdr_int,
		(SVC_IN_ARG_TYPE) &res, TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}


amq_mount_info_list *
amqproc_getmntfs_1(voidp argp, CLIENT *clnt)
{
  static amq_mount_info_list res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_GETMNTFS, (XDRPROC_T_TYPE) xdr_void, argp,
		(XDRPROC_T_TYPE) xdr_amq_mount_info_list,
		(SVC_IN_ARG_TYPE) &res, TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}


int *
amqproc_mount_1(voidp argp, CLIENT *clnt)
{
  static int res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_MOUNT, (XDRPROC_T_TYPE) xdr_amq_string, argp,
		(XDRPROC_T_TYPE) xdr_int, (SVC_IN_ARG_TYPE) &res,
		TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}


amq_string *
amqproc_getvers_1(voidp argp, CLIENT *clnt)
{
  static amq_string res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_GETVERS, (XDRPROC_T_TYPE) xdr_void, argp,
		(XDRPROC_T_TYPE) xdr_amq_string, (SVC_IN_ARG_TYPE) &res,
		TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}


int *
amqproc_getpid_1(voidp argp, CLIENT *clnt)
{
  static int res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_GETPID, (XDRPROC_T_TYPE) xdr_void, argp,
		(XDRPROC_T_TYPE) xdr_int, (SVC_IN_ARG_TYPE) &res,
		TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}


amq_string *
amqproc_pawd_1(amq_string *argp, CLIENT *clnt)
{
  static amq_string res;

  memset((char *) &res, 0, sizeof(res));
  if (clnt_call(clnt, AMQPROC_PAWD,
		(XDRPROC_T_TYPE) xdr_amq_string, (SVC_IN_ARG_TYPE) argp,
		(XDRPROC_T_TYPE) xdr_amq_string, (SVC_IN_ARG_TYPE) &res,
		TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&res);
}