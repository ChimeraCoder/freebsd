
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


bool_t
xdr_time_type(XDR *xdrs, time_type *objp)
{
  if (!xdr_long(xdrs, (long *) objp)) {
    return (FALSE);
  }
  return (TRUE);
}


bool_t
xdr_amq_mount_tree(XDR *xdrs, amq_mount_tree *objp)
{

  if (!xdr_amq_string(xdrs, &objp->mt_mountinfo)) {
    return (FALSE);
  }

  if (!xdr_amq_string(xdrs, &objp->mt_directory)) {
    return (FALSE);
  }

  if (!xdr_amq_string(xdrs, &objp->mt_mountpoint)) {
    return (FALSE);
  }

  if (!xdr_amq_string(xdrs, &objp->mt_type)) {
    return (FALSE);
  }

  if (!xdr_time_type(xdrs, &objp->mt_mounttime)) {
    return (FALSE);
  }

  if (!xdr_u_short(xdrs, &objp->mt_mountuid)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->mt_getattr)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->mt_lookup)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->mt_readdir)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->mt_readlink)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->mt_statfs)) {
    return (FALSE);
  }

  if (!xdr_pointer(xdrs,
		   (char **) ((voidp) &objp->mt_next),
		   sizeof(amq_mount_tree),
		   (XDRPROC_T_TYPE) xdr_amq_mount_tree)) {
    return (FALSE);
  }

  if (!xdr_pointer(xdrs,
		   (char **) ((voidp) &objp->mt_child),
		   sizeof(amq_mount_tree),
		   (XDRPROC_T_TYPE) xdr_amq_mount_tree)) {
    return (FALSE);
  }

  return (TRUE);
}


bool_t
xdr_amq_mount_tree_p(XDR *xdrs, amq_mount_tree_p *objp)
{
  if (!xdr_pointer(xdrs,
		   (char **) objp,
		   sizeof(amq_mount_tree),
		   (XDRPROC_T_TYPE) xdr_amq_mount_tree)) {
    return (FALSE);
  }
  return (TRUE);
}


bool_t
xdr_amq_mount_info(XDR *xdrs, amq_mount_info *objp)
{

  if (!xdr_amq_string(xdrs, &objp->mi_type)) {
    return (FALSE);
  }

  if (!xdr_amq_string(xdrs, &objp->mi_mountpt)) {
    return (FALSE);
  }

  if (!xdr_amq_string(xdrs, &objp->mi_mountinfo)) {
    return (FALSE);
  }

  if (!xdr_amq_string(xdrs, &objp->mi_fserver)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->mi_error)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->mi_refc)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->mi_up)) {
    return (FALSE);
  }

  return (TRUE);
}


bool_t
xdr_amq_mount_info_list(XDR *xdrs, amq_mount_info_list *objp)
{
  if (!xdr_array(xdrs,
		 (char **) ((voidp) &objp->amq_mount_info_list_val),
		 (u_int *) &objp->amq_mount_info_list_len,
		 ~0,
		 sizeof(amq_mount_info),
		 (XDRPROC_T_TYPE) xdr_amq_mount_info)) {
    return (FALSE);
  }
  return (TRUE);
}


bool_t
xdr_amq_mount_tree_list(XDR *xdrs, amq_mount_tree_list *objp)
{
  if (!xdr_array(xdrs,
		 (char **) ((voidp) &objp->amq_mount_tree_list_val),
		 (u_int *) &objp->amq_mount_tree_list_len,
		 ~0,
		 sizeof(amq_mount_tree_p),
		 (XDRPROC_T_TYPE) xdr_amq_mount_tree_p)) {
    return (FALSE);
  }
  return (TRUE);
}


bool_t
xdr_amq_mount_stats(XDR *xdrs, amq_mount_stats *objp)
{

  if (!xdr_int(xdrs, &objp->as_drops)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->as_stale)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->as_mok)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->as_merr)) {
    return (FALSE);
  }

  if (!xdr_int(xdrs, &objp->as_uerr)) {
    return (FALSE);
  }

  return (TRUE);
}


bool_t
xdr_amq_opt(XDR *xdrs, amq_opt *objp)
{
  if (!xdr_enum(xdrs, (enum_t *) objp)) {
    return (FALSE);
  }
  return (TRUE);
}


bool_t
xdr_amq_setopt(XDR *xdrs, amq_setopt *objp)
{

  if (!xdr_amq_opt(xdrs, &objp->as_opt)) {
    return (FALSE);
  }

  if (!xdr_amq_string(xdrs, &objp->as_str)) {
    return (FALSE);
  }

  return (TRUE);
}


bool_t
xdr_pri_free(XDRPROC_T_TYPE xdr_args, caddr_t args_ptr)
{
  XDR xdr;

  xdr.x_op = XDR_FREE;
  return ((*xdr_args) (&xdr, (caddr_t *) args_ptr));
}