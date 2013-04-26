#!/usr/local/bin/perl
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

push(@INC,"perlasm","../../perlasm");
require "x86asm.pl";

require("x86/mul_add.pl");
require("x86/mul.pl");
require("x86/sqr.pl");
require("x86/div.pl");
require("x86/add.pl");
require("x86/sub.pl");
require("x86/comba.pl");

&asm_init($ARGV[0],$0);

&bn_mul_add_words("bn_mul_add_words");
&bn_mul_words("bn_mul_words");
&bn_sqr_words("bn_sqr_words");
&bn_div_words("bn_div_words");
&bn_add_words("bn_add_words");
&bn_sub_words("bn_sub_words");
&bn_mul_comba("bn_mul_comba8",8);
&bn_mul_comba("bn_mul_comba4",4);
&bn_sqr_comba("bn_sqr_comba8",8);
&bn_sqr_comba("bn_sqr_comba4",4);

&asm_finish();