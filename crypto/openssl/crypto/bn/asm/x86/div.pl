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
# x86 assember
sub bn_div_words
	{
	local($name)=@_;

	&function_begin($name,"");
	&mov("edx",&wparam(0));	#
	&mov("eax",&wparam(1));	#
	&mov("ebx",&wparam(2));	#
	&div("ebx");
	&function_end($name);
	}
1;