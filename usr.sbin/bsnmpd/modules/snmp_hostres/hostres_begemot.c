
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

#include <stdlib.h>

#include "hostres_snmp.h"
#include "hostres_oid.h"
#include "hostres_tree.h"

int
op_begemot(struct snmp_context *ctx, struct snmp_value *value,
    u_int sub, u_int iidx __unused, enum snmp_op op)
{

	switch (op) {

	  case SNMP_OP_GET:
		switch (value->var.subs[sub - 1]) {

		  case LEAF_begemotHrStorageUpdate:
			value->v.uint32 = storage_tbl_refresh;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrFSUpdate:
			value->v.uint32 = fs_tbl_refresh;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrDiskStorageUpdate:
			value->v.uint32 = disk_storage_tbl_refresh;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrNetworkUpdate:
			value->v.uint32 = network_tbl_refresh;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrSWInstalledUpdate:
			value->v.uint32 = swins_tbl_refresh;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrSWRunUpdate:
			value->v.uint32 = swrun_tbl_refresh;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrPkgDir:
			return (string_get(value, pkg_dir, -1));
		}
		abort();

	  case SNMP_OP_GETNEXT:
		abort();

	  case SNMP_OP_SET:
		switch (value->var.subs[sub - 1]) {

		  case LEAF_begemotHrStorageUpdate:
			ctx->scratch->int1 = storage_tbl_refresh;
			storage_tbl_refresh = value->v.uint32;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrFSUpdate:
			ctx->scratch->int1 = fs_tbl_refresh;
			fs_tbl_refresh = value->v.uint32;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrDiskStorageUpdate:
			ctx->scratch->int1 = disk_storage_tbl_refresh;
			disk_storage_tbl_refresh = value->v.uint32;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrNetworkUpdate:
			ctx->scratch->int1 = network_tbl_refresh;
			network_tbl_refresh = value->v.uint32;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrSWInstalledUpdate:
			ctx->scratch->int1 = swins_tbl_refresh;
			swins_tbl_refresh = value->v.uint32;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrSWRunUpdate:
			ctx->scratch->int1 = swrun_tbl_refresh;
			swrun_tbl_refresh = value->v.uint32;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrPkgDir:
			return (string_save(value, ctx, -1, &pkg_dir));
		}
		abort();

	  case SNMP_OP_COMMIT:
		switch (value->var.subs[sub - 1]) {

		  case LEAF_begemotHrStorageUpdate:
		  case LEAF_begemotHrFSUpdate:
		  case LEAF_begemotHrDiskStorageUpdate:
		  case LEAF_begemotHrNetworkUpdate:
		  case LEAF_begemotHrSWInstalledUpdate:
		  case LEAF_begemotHrSWRunUpdate:
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrPkgDir:
			string_commit(ctx);
			return (SNMP_ERR_NOERROR);
		}
		abort();

	  case SNMP_OP_ROLLBACK:
		switch (value->var.subs[sub - 1]) {

		  case LEAF_begemotHrStorageUpdate:
			storage_tbl_refresh = ctx->scratch->int1;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrFSUpdate:
			fs_tbl_refresh = ctx->scratch->int1;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrDiskStorageUpdate:
			disk_storage_tbl_refresh = ctx->scratch->int1;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrNetworkUpdate:
			network_tbl_refresh = ctx->scratch->int1;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrSWInstalledUpdate:
			swins_tbl_refresh = ctx->scratch->int1;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrSWRunUpdate:
			swrun_tbl_refresh = ctx->scratch->int1;
			return (SNMP_ERR_NOERROR);

		  case LEAF_begemotHrPkgDir:
			string_rollback(ctx, &pkg_dir);
			return (SNMP_ERR_NOERROR);
		}
		abort();
	}

	abort();
}