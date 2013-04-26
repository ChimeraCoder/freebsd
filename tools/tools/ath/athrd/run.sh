
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# $FreeBSD$
COUNTRIES=${@:-"
	DB NA AL DZ AR AM AU AT AZ BH BY BE BZ BO BR BN BG
	CA CL CN CO CR HR CY CZ DK DO EC EG SV EE FI FR GE
	DE GR GT HN HK HU IS IN ID IR IE IL IT JP J1 J2 J3
	J4 J5 JO KZ KP KR K2 KW LV LB LI LT LU MO MK MY MX
	MC MA NL NZ NO OM PK PA PE PH PL PT PR QA RO RU SA
	SG SK SI ZA ES SE CH SY TW TH TT TN TR UA AE GB US
	UY UZ VE VN YE ZW WOR0_WORLD WOR1_WORLD WOR2_WORLD WOR3_WORLD
	WOR4_WORLD, WOR5_ETSIC EU1_WORLD WOR01_WORLD WOR02_WORLD
"}

for i in $COUNTRIES
do
	./athrd -o $i
done