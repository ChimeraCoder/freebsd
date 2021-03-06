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
sub bn_sub_words
	{
	local($name)=@_;

	&function_begin($name,"");

	&comment("");
	$a="esi";
	$b="edi";
	$c="eax";
	$r="ebx";
	$tmp1="ecx";
	$tmp2="edx";
	$num="ebp";

	&mov($r,&wparam(0));	# get r
	 &mov($a,&wparam(1));	# get a
	&mov($b,&wparam(2));	# get b
	 &mov($num,&wparam(3));	# get num
	&xor($c,$c);		# clear carry
	 &and($num,0xfffffff8);	# num / 8

	&jz(&label("aw_finish"));

	&set_label("aw_loop",0);
	for ($i=0; $i<8; $i++)
		{
		&comment("Round $i");

		&mov($tmp1,&DWP($i*4,$a,"",0)); 	# *a
		 &mov($tmp2,&DWP($i*4,$b,"",0)); 	# *b
		&sub($tmp1,$c);
		 &mov($c,0);
		&adc($c,$c);
		 &sub($tmp1,$tmp2);
		&adc($c,0);
		 &mov(&DWP($i*4,$r,"",0),$tmp1); 	# *r
		}

	&comment("");
	&add($a,32);
	 &add($b,32);
	&add($r,32);
	 &sub($num,8);
	&jnz(&label("aw_loop"));

	&set_label("aw_finish",0);
	&mov($num,&wparam(3));	# get num
	&and($num,7);
	 &jz(&label("aw_end"));

	for ($i=0; $i<7; $i++)
		{
		&comment("Tail Round $i");
		&mov($tmp1,&DWP($i*4,$a,"",0));	# *a
		 &mov($tmp2,&DWP($i*4,$b,"",0));# *b
		&sub($tmp1,$c);
		 &mov($c,0);
		&adc($c,$c);
		 &sub($tmp1,$tmp2);
		&adc($c,0);
		 &dec($num) if ($i != 6);
		&mov(&DWP($i*4,$r,"",0),$tmp1);	# *a
		 &jz(&label("aw_end")) if ($i != 6);
		}
	&set_label("aw_end",0);

#	&mov("eax",$c);		# $c is "eax"

	&function_end($name);
	}

1;