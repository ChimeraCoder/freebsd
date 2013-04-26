
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
 * This function fakes mmap() by reading `len' bytes from the file descriptor
 * `fd' and returning a pointer to that memory.  The "mapped" region can later
 * be deallocated with munmap().
 *
 * Note: ONLY reading is supported and only reading of the exact size of the
 * file will work.
 *
 * PUBLIC: #ifndef HAVE_MMAP
 * PUBLIC: char *mmap __P((char *, size_t, int, int, int, off_t));
 * PUBLIC: #endif
 */char *
mmap(addr, len, prot, flags, fd, off)
	char *addr;
	size_t len;
	int prot, flags, fd;
	off_t off;
{
	char *ptr;

	if ((ptr = (char *)malloc(len)) == 0)
		return ((char *)-1);
	if (read(fd, ptr, len) < 0) {
		free(ptr);
		return ((char *)-1);
	}
	return (ptr);
}

/*
 * PUBLIC: #ifndef HAVE_MMAP
 * PUBLIC: int munmap __P((char *, size_t));
 * PUBLIC: #endif
 */
int
munmap(addr, len)
	char *addr;
	size_t len;
{
	free(addr);
	return (0);
}