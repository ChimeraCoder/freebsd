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
sub bn_sqr_words
	{
	local($name)=@_;

	&function_begin($name,"");

	&comment("");
	$r="esi";
	$a="edi";
	$num="ebx";

	&mov($r,&wparam(0));	#
	&mov($a,&wparam(1));	#
	&mov($num,&wparam(2));	#

	&and($num,0xfffffff8);	# num / 8
	&jz(&label("sw_finish"));

	&set_label("sw_loop",0);
	for ($i=0; $i<32; $i+=4)
		{
		&comment("Round $i");
		&mov("eax",&DWP($i,$a,"",0)); 	# *a
		 # XXX
		&mul("eax");			# *a * *a
		&mov(&DWP($i*2,$r,"",0),"eax");	#
		 &mov(&DWP($i*2+4,$r,"",0),"edx");#
		}

	&comment("");
	&add($a,32);
	&add($r,64);
	&sub($num,8);
	&jnz(&label("sw_loop"));

	&set_label("sw_finish",0);
	&mov($num,&wparam(2));	# get num
	&and($num,7);
	&jz(&label("sw_end"));

	for ($i=0; $i<7; $i++)
		{
		&comment("Tail Round $i");
		&mov("eax",&DWP($i*4,$a,"",0));	# *a
		 # XXX
		&mul("eax");			# *a * *a
		&mov(&DWP($i*8,$r,"",0),"eax");	#
		 &dec($num) if ($i != 7-1);
		&mov(&DWP($i*8+4,$r,"",0),"edx");
		 &jz(&label("sw_end")) if ($i != 7-1);
		}
	&set_label("sw_end",0);

	&function_end($name);
	}

1;