
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
 * routines to convert on disk ext2 inodes into inodes and back
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/endian.h>
#include <sys/lock.h>
#include <sys/stat.h>
#include <sys/vnode.h>

#include <fs/ext2fs/inode.h>
#include <fs/ext2fs/ext2fs.h>
#include <fs/ext2fs/ext2_dinode.h>
#include <fs/ext2fs/ext2_extern.h>

#define XTIME_TO_NSEC(x)	((x & EXT3_NSEC_MASK) >> 2)
#define NSEC_TO_XTIME(t)	(le32toh(t << 2) & EXT3_NSEC_MASK)

void
ext2_print_inode(struct inode *in)
{
	int i;

	printf( "Inode: %5ju", (uintmax_t)in->i_number);
	printf( /* "Inode: %5d" */
		" Type: %10s Mode: 0x%o Flags: 0x%x  Version: %d\n",
		"n/a", in->i_mode, in->i_flags, in->i_gen);
	printf( "User: %5lu Group: %5lu  Size: %lu\n",
		(unsigned long)in->i_uid, (unsigned long)in->i_gid,
		(unsigned long)in->i_size);
	printf( "Links: %3d Blockcount: %d\n",
		in->i_nlink, in->i_blocks);
	printf( "ctime: 0x%x", in->i_ctime);
	printf( "atime: 0x%x", in->i_atime);
	printf( "mtime: 0x%x", in->i_mtime);
	printf( "BLOCKS: ");
	for(i=0; i < (in->i_blocks <= 24 ? ((in->i_blocks+1)/2): 12); i++)
		printf("%d ", in->i_db[i]);
	printf("\n");
}

/*
 *	raw ext2 inode to inode
 */
void
ext2_ei2i(struct ext2fs_dinode *ei, struct inode *ip)
{
        int i;

	ip->i_nlink = ei->e2di_nlink;
	/* Godmar thinks - if the link count is zero, then the inode is
	   unused - according to ext2 standards. Ufs marks this fact
	   by setting i_mode to zero - why ?
	   I can see that this might lead to problems in an undelete.
	*/
	ip->i_mode = ei->e2di_nlink ? ei->e2di_mode : 0;
	ip->i_size = ei->e2di_size;
	if (S_ISREG(ip->i_mode))
		ip->i_size |= ((u_int64_t)ei->e2di_size_high) << 32;
	ip->i_atime = ei->e2di_atime;
	ip->i_mtime = ei->e2di_mtime;
	ip->i_ctime = ei->e2di_ctime;
	if (E2DI_HAS_XTIME(ip)) {
		ip->i_atimensec = XTIME_TO_NSEC(ei->e2di_atime_extra);
		ip->i_mtimensec = XTIME_TO_NSEC(ei->e2di_mtime_extra);
		ip->i_ctimensec = XTIME_TO_NSEC(ei->e2di_ctime_extra);
		ip->i_birthtime = ei->e2di_crtime;
		ip->i_birthnsec = XTIME_TO_NSEC(ei->e2di_crtime_extra);
	}
	ip->i_flags = 0;
	ip->i_flags |= (ei->e2di_flags & EXT2_APPEND) ? SF_APPEND : 0;
	ip->i_flags |= (ei->e2di_flags & EXT2_IMMUTABLE) ? SF_IMMUTABLE : 0;
	ip->i_flags |= (ei->e2di_flags & EXT2_NODUMP) ? UF_NODUMP : 0;
	ip->i_blocks = ei->e2di_nblock;
	ip->i_gen = ei->e2di_gen;
	ip->i_uid = ei->e2di_uid;
	ip->i_gid = ei->e2di_gid;
	/* XXX use memcpy */
	for(i = 0; i < NDADDR; i++)
		ip->i_db[i] = ei->e2di_blocks[i];
	for(i = 0; i < NIADDR; i++)
		ip->i_ib[i] = ei->e2di_blocks[EXT2_NDIR_BLOCKS + i];
}

/*
 *	inode to raw ext2 inode
 */
void
ext2_i2ei(struct inode *ip, struct ext2fs_dinode *ei)
{
	int i;

	ei->e2di_mode = ip->i_mode;
        ei->e2di_nlink = ip->i_nlink;
	/* 
	   Godmar thinks: if dtime is nonzero, ext2 says this inode
	   has been deleted, this would correspond to a zero link count
	 */
	ei->e2di_dtime = ei->e2di_nlink ? 0 : ip->i_mtime;
	ei->e2di_size = ip->i_size;
	if (S_ISREG(ip->i_mode))
		ei->e2di_size_high = ip->i_size >> 32;
	ei->e2di_atime = ip->i_atime;
	ei->e2di_mtime = ip->i_mtime;
	ei->e2di_ctime = ip->i_ctime;
	if (E2DI_HAS_XTIME(ip)) {
		ei->e2di_ctime_extra = NSEC_TO_XTIME(ip->i_ctimensec);
		ei->e2di_mtime_extra = NSEC_TO_XTIME(ip->i_mtimensec);
		ei->e2di_atime_extra = NSEC_TO_XTIME(ip->i_atimensec);
		ei->e2di_crtime = ip->i_birthtime;
		ei->e2di_crtime_extra = NSEC_TO_XTIME(ip->i_birthnsec);
	}
	ei->e2di_flags = ip->i_flags;
	ei->e2di_flags = 0;
	ei->e2di_flags |= (ip->i_flags & SF_APPEND) ? EXT2_APPEND: 0;
	ei->e2di_flags |= (ip->i_flags & SF_IMMUTABLE) ? EXT2_IMMUTABLE: 0;
	ei->e2di_flags |= (ip->i_flags & UF_NODUMP) ? EXT2_NODUMP: 0;
	ei->e2di_nblock = ip->i_blocks;
	ei->e2di_gen = ip->i_gen;
	ei->e2di_uid = ip->i_uid;
	ei->e2di_gid = ip->i_gid;
	/* XXX use memcpy */
	for(i = 0; i < NDADDR; i++)
		ei->e2di_blocks[i] = ip->i_db[i];
	for(i = 0; i < NIADDR; i++)
		ei->e2di_blocks[EXT2_NDIR_BLOCKS + i] = ip->i_ib[i];
}