
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
 * CAM Target Layer command serialization table.
 *
 * Author: Kim Le
 */

/****************************************************************************/
/* TABLE       ctlSerTbl                                                    */
/*                                                                          */
/*  The matrix which drives the serialization algorithm. The major index    */
/*  (the first) into this table is the command being checked and the minor  */
/*  index is the command against which the first command is being checked.  */
/*  i.e., the major index (row) command is ahead of the minor index command */
/*  (column) in the queue.  This allows the code to optimize by capturing   */
/*  the result of the first indexing operation into a pointer.              */
/*                                                                          */
/*  Whenever a new value is added to the IDX_T type, this matrix must be    */
/*  expanded by one row AND one column -- Because of this, some effort      */
/*  should be made to re-use the indexes whenever possible.                 */
/*                                                                          */
/****************************************************************************/

#define	sK	CTL_SER_SKIP		/* Skip */
#define	pS	CTL_SER_PASS		/* pS */
#define	bK	CTL_SER_BLOCK		/* Blocked */
#define	xT	CTL_SER_EXTENT		/* Extent check */

static ctl_serialize_action
ctl_serialize_table[CTL_SERIDX_COUNT][CTL_SERIDX_COUNT] = {
/**>IDX_ :: 2nd:TUR RD  WRT  MDSN MDSL RQSN INQ  RDCP RES  REL LSNS FMT STR PRIN PROT MAININ*/
/*TUR     */{   pS, pS, pS,  bK,  bK,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*READ    */{   pS, pS, xT,  bK,  bK,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*WRITE   */{   pS, xT, xT,  bK,  bK,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*MD_SNS  */{   bK, bK, bK,  pS,  bK,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*MD_SEL  */{   bK, bK, bK,  bK,  bK,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*RQ_SNS  */{   pS, pS, pS,  pS,  pS,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*INQ     */{   pS, pS, pS,  pS,  pS,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*RD_CAP  */{   pS, pS, pS,  pS,  pS,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*RESV    */{   bK, bK, bK,  bK,  bK,  bK,  bK,  bK,  bK,  bK, bK,  bK, bK, bK,  bK,  bK},
/*REL     */{   bK, bK, bK,  bK,  bK,  bK,  bK,  bK,  bK,  bK, bK,  bK, bK, bK,  bK,  bK},
/*LOG_SNS */{   pS, pS, pS,  pS,  bK,  bK,  pS,  pS,  bK,  bK, pS,  bK, bK, bK,  bK,  bK},
/*FORMAT  */{   pS, bK, bK,  bK,  bK,  pS,  pS,  bK,  bK,  bK, bK,  bK, bK, bK,  bK,  bK},
/*START   */{   bK, bK, bK,  bK,  bK,  bK,  pS,  bK,  bK,  bK, bK,  bK, bK, bK,  bK,  bK},
/*PRES_IN */{   bK, bK, bK,  bK,  bK,  bK,  bK,  bK,  bK,  bK, bK,  bK, bK, bK,  bK,  bK},
/*PRES_OUT*/{   bK, bK, bK,  bK,  bK,  bK,  bK,  bK,  bK,  bK, bK,  bK, bK, bK,  bK,  bK},
/*MAIN_IN */{   bK, bK, bK,  bK,  bK,  bK,  pS,  bK,  bK,  bK, bK,  bK, bK, bK,  bK,  pS}
};