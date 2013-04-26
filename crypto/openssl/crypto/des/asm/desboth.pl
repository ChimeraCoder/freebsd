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

$L="edi";
$R="esi";

sub DES_encrypt3
	{
	local($name,$enc)=@_;

	&function_begin_B($name,"");
	&push("ebx");
	&mov("ebx",&wparam(0));

	&push("ebp");
	&push("esi");

	&push("edi");

	&comment("");
	&comment("Load the data words");
	&mov($L,&DWP(0,"ebx","",0));
	&mov($R,&DWP(4,"ebx","",0));
	&stack_push(3);

	&comment("");
	&comment("IP");
	&IP_new($L,$R,"edx",0);

	# put them back
	
	if ($enc)
		{
		&mov(&DWP(4,"ebx","",0),$R);
		 &mov("eax",&wparam(1));
		&mov(&DWP(0,"ebx","",0),"edx");
		 &mov("edi",&wparam(2));
		 &mov("esi",&wparam(3));
		}
	else
		{
		&mov(&DWP(4,"ebx","",0),$R);
		 &mov("esi",&wparam(1));
		&mov(&DWP(0,"ebx","",0),"edx");
		 &mov("edi",&wparam(2));
		 &mov("eax",&wparam(3));
		}
	&mov(&swtmp(2),	(DWC(($enc)?"1":"0")));
	&mov(&swtmp(1),	"eax");
	&mov(&swtmp(0),	"ebx");
	&call("DES_encrypt2");
	&mov(&swtmp(2),	(DWC(($enc)?"0":"1")));
	&mov(&swtmp(1),	"edi");
	&mov(&swtmp(0),	"ebx");
	&call("DES_encrypt2");
	&mov(&swtmp(2),	(DWC(($enc)?"1":"0")));
	&mov(&swtmp(1),	"esi");
	&mov(&swtmp(0),	"ebx");
	&call("DES_encrypt2");

	&stack_pop(3);
	&mov($L,&DWP(0,"ebx","",0));
	&mov($R,&DWP(4,"ebx","",0));

	&comment("");
	&comment("FP");
	&FP_new($L,$R,"eax",0);

	&mov(&DWP(0,"ebx","",0),"eax");
	&mov(&DWP(4,"ebx","",0),$R);

	&pop("edi");
	&pop("esi");
	&pop("ebp");
	&pop("ebx");
	&ret();
	&function_end_B($name);
	}