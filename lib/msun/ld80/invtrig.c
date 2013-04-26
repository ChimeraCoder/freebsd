
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "invtrig.h"

/*
 * asinl() and acosl()
 */
const long double
pS0 =  1.66666666666666666631e-01L,
pS1 = -4.16313987993683104320e-01L,
pS2 =  3.69068046323246813704e-01L,
pS3 = -1.36213932016738603108e-01L,
pS4 =  1.78324189708471965733e-02L,
pS5 = -2.19216428382605211588e-04L,
pS6 = -7.10526623669075243183e-06L,
qS1 = -2.94788392796209867269e+00L,
qS2 =  3.27309890266528636716e+00L,
qS3 = -1.68285799854822427013e+00L,
qS4 =  3.90699412641738801874e-01L,
qS5 = -3.14365703596053263322e-02L;

/*
 * atanl()
 */
const long double atanhi[] = {
	 4.63647609000806116202e-01L,
	 7.85398163397448309628e-01L,
	 9.82793723247329067960e-01L,
	 1.57079632679489661926e+00L,
};

const long double atanlo[] = {
	 1.18469937025062860669e-20L,
	-1.25413940316708300586e-20L,
	 2.55232234165405176172e-20L,
	-2.50827880633416601173e-20L,
};

const long double aT[] = {
	 3.33333333333333333017e-01L,
	-1.99999999999999632011e-01L,
	 1.42857142857046531280e-01L,
	-1.11111111100562372733e-01L,
	 9.09090902935647302252e-02L,
	-7.69230552476207730353e-02L,
	 6.66661718042406260546e-02L,
	-5.88158892835030888692e-02L,
	 5.25499891539726639379e-02L,
	-4.70119845393155721494e-02L,
	 4.03539201366454414072e-02L,
	-2.91303858419364158725e-02L,
	 1.24822046299269234080e-02L,
};

const long double pi_lo = -5.01655761266833202345e-20L;