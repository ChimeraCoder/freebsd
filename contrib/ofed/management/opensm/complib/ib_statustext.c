
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
 * Abstract:
 *	Defines string to decode ib_api_status_t return values.
 *
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <complib/cl_types.h>

/* ib_api_status_t values above converted to text for easier printing. */
const char *ib_error_str[] = {
	"IB_SUCCESS",
	"IB_INSUFFICIENT_RESOURCES",
	"IB_INSUFFICIENT_MEMORY",
	"IB_INVALID_PARAMETER",
	"IB_INVALID_SETTING",
	"IB_NOT_FOUND",
	"IB_TIMEOUT",
	"IB_CANCELED",
	"IB_INTERRUPTED",
	"IB_INVALID_PERMISSION",
	"IB_UNSUPPORTED",
	"IB_OVERFLOW",
	"IB_MAX_MCAST_QPS_REACHED",
	"IB_INVALID_QP_STATE",
	"IB_INVALID_EEC_STATE",
	"IB_INVALID_APM_STATE",
	"IB_INVALID_PORT_STATE",
	"IB_INVALID_STATE",
	"IB_RESOURCE_BUSY",
	"IB_INVALID_PKEY",
	"IB_INVALID_LKEY",
	"IB_INVALID_RKEY",
	"IB_INVALID_MAX_WRS",
	"IB_INVALID_MAX_SGE",
	"IB_INVALID_CQ_SIZE",
	"IB_INVALID_SERVICE_TYPE",
	"IB_INVALID_GID",
	"IB_INVALID_LID",
	"IB_INVALID_GUID",
	"IB_INVALID_CA_HANDLE",
	"IB_INVALID_AV_HANDLE",
	"IB_INVALID_CQ_HANDLE",
	"IB_INVALID_EEC_HANDLE",
	"IB_INVALID_QP_HANDLE",
	"IB_INVALID_PD_HANDLE",
	"IB_INVALID_MR_HANDLE",
	"IB_INVALID_MW_HANDLE",
	"IB_INVALID_RDD_HANDLE",
	"IB_INVALID_MCAST_HANDLE",
	"IB_INVALID_CALLBACK",
	"IB_INVALID_AL_HANDLE",
	"IB_INVALID_HANDLE",
	"IB_ERROR",
	"IB_REMOTE_ERROR",	/* Infiniband Access Layer */
	"IB_VERBS_PROCESSING_DONE",
	"IB_INVALID_WR_TYPE",
	"IB_QP_IN_TIMEWAIT",
	"IB_EE_IN_TIMEWAIT",
	"IB_INVALID_PORT",
	"IB_NOT_DONE",
	"IB_UNKNOWN_ERROR"
};

/* ib_async_event_t values above converted to text for easier printing. */
const char *ib_async_event_str[] = {
	"IB_AE_SQ_ERROR",
	"IB_AE_SQ_DRAINED",
	"IB_AE_RQ_ERROR",
	"IB_AE_CQ_ERROR",
	"IB_AE_QP_FATAL",
	"IB_AE_QP_COMM",
	"IB_AE_QP_APM",
	"IB_AE_EEC_FATAL",
	"IB_AE_EEC_COMM",
	"IB_AE_EEC_APM",
	"IB_AE_LOCAL_FATAL",
	"IB_AE_PKEY_TRAP",
	"IB_AE_QKEY_TRAP",
	"IB_AE_MKEY_TRAP",
	"IB_AE_PORT_TRAP",
	"IB_AE_SYSIMG_GUID_TRAP",
	"IB_AE_BUF_OVERRUN",
	"IB_AE_LINK_INTEGRITY",
	"IB_AE_FLOW_CTRL_ERROR",
	"IB_AE_BKEY_TRAP",
	"IB_AE_QP_APM_ERROR",
	"IB_AE_EEC_APM_ERROR",
	"IB_AE_WQ_REQ_ERROR",
	"IB_AE_WQ_ACCESS_ERROR",
	"IB_AE_PORT_ACTIVE",	/* ACTIVE STATE */
	"IB_AE_PORT_DOWN",	/* INIT", ARMED", DOWN */
	"IB_AE_UNKNOWN"
};

const char *ib_wc_status_str[] = {
	"IB_WCS_SUCCESS",
	"IB_WCS_LOCAL_LEN_ERR",
	"IB_WCS_LOCAL_OP_ERR",
	"IB_WCS_LOCAL_EEC_OP_ERR",
	"IB_WCS_LOCAL_PROTECTION_ERR",
	"IB_WCS_WR_FLUSHED_ERR",
	"IB_WCS_MEM_WINDOW_BIND_ERR",
	"IB_WCS_REM_ACCESS_ERR",
	"IB_WCS_REM_OP_ERR",
	"IB_WCS_RNR_RETRY_ERR",
	"IB_WCS_TIMEOUT_RETRY_ERR",
	"IB_WCS_REM_INVALID_REQ_ERR",
	"IB_WCS_REM_INVALID_RD_REQ_ERR",
	"IB_WCS_INVALID_EECN",
	"IB_WCS_INVALID_EEC_STATE",
	"IB_WCS_UNMATCHED_RESPONSE",	/* InfiniBand Access Layer */
	"IB_WCS_CANCELED",	/* InfiniBand Access Layer */
	"IB_WCS_UNKNOWN"
};