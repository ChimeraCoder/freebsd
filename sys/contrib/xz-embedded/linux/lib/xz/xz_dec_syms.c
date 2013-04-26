
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
 * XZ decoder module information
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */
#include <linux/module.h>
#include <linux/xz.h>

EXPORT_SYMBOL(xz_dec_init);
EXPORT_SYMBOL(xz_dec_reset);
EXPORT_SYMBOL(xz_dec_run);
EXPORT_SYMBOL(xz_dec_end);

MODULE_DESCRIPTION("XZ decompressor");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Lasse Collin <lasse.collin@tukaani.org> and Igor Pavlov");

/*
 * This code is in the public domain, but in Linux it's simplest to just
 * say it's GPL and consider the authors as the copyright holders.
 */
MODULE_LICENSE("GPL");