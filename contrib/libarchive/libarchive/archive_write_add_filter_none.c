
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

#include "archive_platform.h"
__FBSDID("$FreeBSD: head/lib/libarchive/archive_write_set_compression_none.c 201080 2009-12-28 02:03:54Z kientzle $");

#include "archive.h"

int
archive_write_set_compression_none(struct archive *a)
{
	(void)a; /* UNUSED */
	return (ARCHIVE_OK);
}

int
archive_write_add_filter_none(struct archive *a)
{
	(void)a; /* UNUSED */
	return (ARCHIVE_OK);
}