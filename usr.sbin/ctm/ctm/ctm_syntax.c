
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

#include "ctm.h"

/* The fields... */
#define Name	CTM_F_Name
#define Uid	CTM_F_Uid
#define Gid	CTM_F_Gid
#define Mode	CTM_F_Mode
#define MD5	CTM_F_MD5
#define Count	CTM_F_Count
#define Bytes	CTM_F_Bytes

/* The qualifiers... */
#define File	CTM_Q_Name_File
#define Dir	CTM_Q_Name_Dir
#define New	CTM_Q_Name_New
#define Subst	CTM_Q_Name_Subst
#define After	CTM_Q_MD5_After
#define Before	CTM_Q_MD5_Before
#define Chunk	CTM_Q_MD5_Chunk
#define Force	CTM_Q_MD5_Force

static int ctmFM[] = /* File Make */
    { Name|File|New|Subst, Uid, Gid, Mode,
	MD5|After|Chunk, Count, Bytes,0 };

static int ctmFS[] = /* File Substitute */
    { Name|File|Subst, Uid, Gid, Mode,
	MD5|Before|Force, MD5|After|Chunk, Count, Bytes,0 };

static int ctmFE[] = /* File Edit */
    { Name|File|Subst, Uid, Gid, Mode,
	MD5|Before, MD5|After, Count, Bytes,0 };

static int ctmFR[] = /* File Remove */
    { Name|File|Subst, MD5|Before, 0 };

static int ctmAS[] = /* Attribute Substitute */
    { Name|Subst, Uid, Gid, Mode, 0 };

static int ctmDM[] = /* Directory Make */
    { Name|Dir|New , Uid, Gid, Mode, 0 };

static int ctmDR[] = /* Directory Remove */
    { Name|Dir, 0 };

struct CTM_Syntax Syntax[] = {
    { "FM",  	ctmFM },
    { "FS",  	ctmFS },
    { "FE",  	ctmFE },
    { "FN",  	ctmFE },
    { "FR",  	ctmFR },
    { "AS", 	ctmAS },
    { "DM",  	ctmDM },
    { "DR",  	ctmDR },
    { 0,    	0} };