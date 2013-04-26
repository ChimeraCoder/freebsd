
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

#include "cryptlib.h"
#include <openssl/evp.h>
#include <openssl/conf.h>


/* Load all algorithms and configure OpenSSL.
 * This function is called automatically when
 * OPENSSL_LOAD_CONF is set.
 */

void OPENSSL_add_all_algorithms_conf(void)
	{
	OPENSSL_add_all_algorithms_noconf();
	OPENSSL_config(NULL);
	}