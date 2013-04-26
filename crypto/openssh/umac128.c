
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
/* $FreeBSD$ */#define UMAC_OUTPUT_LEN		16
#undef umac_new
#define umac_new		ssh_umac128_new
#undef umac_update
#define umac_update		ssh_umac128_update
#undef umac_final
#define umac_final		ssh_umac128_final
#undef umac_delete
#define umac_delete		ssh_umac128_delete
#include "umac.c"