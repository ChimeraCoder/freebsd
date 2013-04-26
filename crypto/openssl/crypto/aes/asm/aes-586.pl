#!/usr/bin/env perl
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

$0 =~ m/(.*[\/\\])[^\/\\]+$/; $dir=$1;
push(@INC,"${dir}","${dir}../../perlasm");
require "x86asm.pl";

&asm_init($ARGV[0],"aes-586.pl",$x86only = $ARGV[$#ARGV] eq "386");
&static_label("AES_Te");
&static_label("AES_Td");

$s0="eax";
$s1="ebx";
$s2="ecx";
$s3="edx";
$key="edi";
$acc="esi";
$tbl="ebp";

# stack frame layout in _[x86|sse]_AES_* routines, frame is allocated
# by caller
$__ra=&DWP(0,"esp");	# return address
$__s0=&DWP(4,"esp");	# s0 backing store
$__s1=&DWP(8,"esp");	# s1 backing store
$__s2=&DWP(12,"esp");	# s2 backing store
$__s3=&DWP(16,"esp");	# s3 backing store
$__key=&DWP(20,"esp");	# pointer to key schedule
$__end=&DWP(24,"esp");	# pointer to end of key schedule
$__tbl=&DWP(28,"esp");	# %ebp backing store

# stack frame layout in AES_[en|crypt] routines, which differs from
# above by 4 and overlaps by %ebp backing store
$_tbl=&DWP(24,"esp");
$_esp=&DWP(28,"esp");

sub _data_word() { my $i; while(defined($i=shift)) { &data_word($i,$i); } }

$speed_limit=512;	# chunks smaller than $speed_limit are
			# processed with compact routine in CBC mode
$small_footprint=1;	# $small_footprint=1 code is ~5% slower [on
			# recent �-archs], but ~5 times smaller!
			# I favor compact code to minimize cache
			# contention and in hope to "collect" 5% back
			# in real-life applications...

$vertical_spin=0;	# shift "verticaly" defaults to 0, because of
			# its proof-of-concept status...
# Note that there is no decvert(), as well as last encryption round is
# performed with "horizontal" shifts. This is because this "vertical"
# implementation [one which groups shifts on a given $s[i] to form a
# "column," unlike "horizontal" one, which groups shifts on different
# $s[i] to form a "row"] is work in progress. It was observed to run
# few percents faster on Intel cores, but not AMD. On AMD K8 core it's
# whole 12% slower:-( So we face a trade-off... Shall it be resolved
# some day? Till then the code is considered experimental and by
# default remains dormant...

sub encvert()
{ my ($te,@s) = @_;
  my $v0 = $acc, $v1 = $key;

	&mov	($v0,$s[3]);				# copy s3
	&mov	(&DWP(4,"esp"),$s[2]);			# save s2
	&mov	($v1,$s[0]);				# copy s0
	&mov	(&DWP(8,"esp"),$s[1]);			# save s1

	&movz	($s[2],&HB($s[0]));
	&and	($s[0],0xFF);
	&mov	($s[0],&DWP(0,$te,$s[0],8));		# s0>>0
	&shr	($v1,16);
	&mov	($s[3],&DWP(3,$te,$s[2],8));		# s0>>8
	&movz	($s[1],&HB($v1));
	&and	($v1,0xFF);
	&mov	($s[2],&DWP(2,$te,$v1,8));		# s0>>16
	 &mov	($v1,$v0);
	&mov	($s[1],&DWP(1,$te,$s[1],8));		# s0>>24

	&and	($v0,0xFF);
	&xor	($s[3],&DWP(0,$te,$v0,8));		# s3>>0
	&movz	($v0,&HB($v1));
	&shr	($v1,16);
	&xor	($s[2],&DWP(3,$te,$v0,8));		# s3>>8
	&movz	($v0,&HB($v1));
	&and	($v1,0xFF);
	&xor	($s[1],&DWP(2,$te,$v1,8));		# s3>>16
	 &mov	($v1,&DWP(4,"esp"));			# restore s2
	&xor	($s[0],&DWP(1,$te,$v0,8));		# s3>>24

	&mov	($v0,$v1);
	&and	($v1,0xFF);
	&xor	($s[2],&DWP(0,$te,$v1,8));		# s2>>0
	&movz	($v1,&HB($v0));
	&shr	($v0,16);
	&xor	($s[1],&DWP(3,$te,$v1,8));		# s2>>8
	&movz	($v1,&HB($v0));
	&and	($v0,0xFF);
	&xor	($s[0],&DWP(2,$te,$v0,8));		# s2>>16
	 &mov	($v0,&DWP(8,"esp"));			# restore s1
	&xor	($s[3],&DWP(1,$te,$v1,8));		# s2>>24

	&mov	($v1,$v0);
	&and	($v0,0xFF);
	&xor	($s[1],&DWP(0,$te,$v0,8));		# s1>>0
	&movz	($v0,&HB($v1));
	&shr	($v1,16);
	&xor	($s[0],&DWP(3,$te,$v0,8));		# s1>>8
	&movz	($v0,&HB($v1));
	&and	($v1,0xFF);
	&xor	($s[3],&DWP(2,$te,$v1,8));		# s1>>16
	 &mov	($key,$__key);				# reincarnate v1 as key
	&xor	($s[2],&DWP(1,$te,$v0,8));		# s1>>24
}

# Another experimental routine, which features "horizontal spin," but
# eliminates one reference to stack. Strangely enough runs slower...
sub enchoriz()
{ my $v0 = $key, $v1 = $acc;

	&movz	($v0,&LB($s0));			#  3, 2, 1, 0*
	&rotr	($s2,8);			#  8,11,10, 9
	&mov	($v1,&DWP(0,$te,$v0,8));	#  0
	&movz	($v0,&HB($s1));			#  7, 6, 5*, 4
	&rotr	($s3,16);			# 13,12,15,14
	&xor	($v1,&DWP(3,$te,$v0,8));	#  5
	&movz	($v0,&HB($s2));			#  8,11,10*, 9
	&rotr	($s0,16);			#  1, 0, 3, 2
	&xor	($v1,&DWP(2,$te,$v0,8));	# 10
	&movz	($v0,&HB($s3));			# 13,12,15*,14
	&xor	($v1,&DWP(1,$te,$v0,8));	# 15, t[0] collected
	&mov	($__s0,$v1);			# t[0] saved

	&movz	($v0,&LB($s1));			#  7, 6, 5, 4*
	&shr	($s1,16);			#  -, -, 7, 6
	&mov	($v1,&DWP(0,$te,$v0,8));	#  4
	&movz	($v0,&LB($s3));			# 13,12,15,14*
	&xor	($v1,&DWP(2,$te,$v0,8));	# 14
	&movz	($v0,&HB($s0));			#  1, 0, 3*, 2
	&and	($s3,0xffff0000);		# 13,12, -, -
	&xor	($v1,&DWP(1,$te,$v0,8));	#  3
	&movz	($v0,&LB($s2));			#  8,11,10, 9*
	&or	($s3,$s1);			# 13,12, 7, 6
	&xor	($v1,&DWP(3,$te,$v0,8));	#  9, t[1] collected
	&mov	($s1,$v1);			#  s[1]=t[1]

	&movz	($v0,&LB($s0));			#  1, 0, 3, 2*
	&shr	($s2,16);			#  -, -, 8,11
	&mov	($v1,&DWP(2,$te,$v0,8));	#  2
	&movz	($v0,&HB($s3));			# 13,12, 7*, 6
	&xor	($v1,&DWP(1,$te,$v0,8));	#  7
	&movz	($v0,&HB($s2));			#  -, -, 8*,11
	&xor	($v1,&DWP(0,$te,$v0,8));	#  8
	&mov	($v0,$s3);
	&shr	($v0,24);			# 13
	&xor	($v1,&DWP(3,$te,$v0,8));	# 13, t[2] collected

	&movz	($v0,&LB($s2));			#  -, -, 8,11*
	&shr	($s0,24);			#  1*
	&mov	($s2,&DWP(1,$te,$v0,8));	# 11
	&xor	($s2,&DWP(3,$te,$s0,8));	#  1
	&mov	($s0,$__s0);			# s[0]=t[0]
	&movz	($v0,&LB($s3));			# 13,12, 7, 6*
	&shr	($s3,16);			#   ,  ,13,12
	&xor	($s2,&DWP(2,$te,$v0,8));	#  6
	&mov	($key,$__key);			# reincarnate v0 as key
	&and	($s3,0xff);			#   ,  ,13,12*
	&mov	($s3,&DWP(0,$te,$s3,8));	# 12
	&xor	($s3,$s2);			# s[2]=t[3] collected
	&mov	($s2,$v1);			# s[2]=t[2]
}

# More experimental code... SSE one... Even though this one eliminates
# *all* references to stack, it's not faster...
sub sse_encbody()
{
	&movz	($acc,&LB("eax"));		#  0
	&mov	("ecx",&DWP(0,$tbl,$acc,8));	#  0
	&pshufw	("mm2","mm0",0x0d);		#  7, 6, 3, 2
	&movz	("edx",&HB("eax"));		#  1
	&mov	("edx",&DWP(3,$tbl,"edx",8));	#  1
	&shr	("eax",16);			#  5, 4

	&movz	($acc,&LB("ebx"));		# 10
	&xor	("ecx",&DWP(2,$tbl,$acc,8));	# 10
	&pshufw	("mm6","mm4",0x08);		# 13,12, 9, 8
	&movz	($acc,&HB("ebx"));		# 11
	&xor	("edx",&DWP(1,$tbl,$acc,8));	# 11
	&shr	("ebx",16);			# 15,14

	&movz	($acc,&HB("eax"));		#  5
	&xor	("ecx",&DWP(3,$tbl,$acc,8));	#  5
	&movq	("mm3",QWP(16,$key));
	&movz	($acc,&HB("ebx"));		# 15
	&xor	("ecx",&DWP(1,$tbl,$acc,8));	# 15
	&movd	("mm0","ecx");			# t[0] collected

	&movz	($acc,&LB("eax"));		#  4
	&mov	("ecx",&DWP(0,$tbl,$acc,8));	#  4
	&movd	("eax","mm2");			#  7, 6, 3, 2
	&movz	($acc,&LB("ebx"));		# 14
	&xor	("ecx",&DWP(2,$tbl,$acc,8));	# 14
	&movd	("ebx","mm6");			# 13,12, 9, 8

	&movz	($acc,&HB("eax"));		#  3
	&xor	("ecx",&DWP(1,$tbl,$acc,8));	#  3
	&movz	($acc,&HB("ebx"));		#  9
	&xor	("ecx",&DWP(3,$tbl,$acc,8));	#  9
	&movd	("mm1","ecx");			# t[1] collected

	&movz	($acc,&LB("eax"));		#  2
	&mov	("ecx",&DWP(2,$tbl,$acc,8));	#  2
	&shr	("eax",16);			#  7, 6
	&punpckldq	("mm0","mm1");		# t[0,1] collected
	&movz	($acc,&LB("ebx"));		#  8
	&xor	("ecx",&DWP(0,$tbl,$acc,8));	#  8
	&shr	("ebx",16);			# 13,12

	&movz	($acc,&HB("eax"));		#  7
	&xor	("ecx",&DWP(1,$tbl,$acc,8));	#  7
	&pxor	("mm0","mm3");
	&movz	("eax",&LB("eax"));		#  6
	&xor	("edx",&DWP(2,$tbl,"eax",8));	#  6
	&pshufw	("mm1","mm0",0x08);		#  5, 4, 1, 0
	&movz	($acc,&HB("ebx"));		# 13
	&xor	("ecx",&DWP(3,$tbl,$acc,8));	# 13
	&xor	("ecx",&DWP(24,$key));		# t[2]
	&movd	("mm4","ecx");			# t[2] collected
	&movz	("ebx",&LB("ebx"));		# 12
	&xor	("edx",&DWP(0,$tbl,"ebx",8));	# 12
	&shr	("ecx",16);
	&movd	("eax","mm1");			#  5, 4, 1, 0
	&mov	("ebx",&DWP(28,$key));		# t[3]
	&xor	("ebx","edx");
	&movd	("mm5","ebx");			# t[3] collected
	&and	("ebx",0xffff0000);
	&or	("ebx","ecx");

	&punpckldq	("mm4","mm5");		# t[2,3] collected
}

######################################################################
# "Compact" block function
######################################################################

sub enccompact()
{ my $Fn = mov;
  while ($#_>5) { pop(@_); $Fn=sub{}; }
  my ($i,$te,@s)=@_;
  my $tmp = $key;
  my $out = $i==3?$s[0]:$acc;

	# $Fn is used in first compact round and its purpose is to
	# void restoration of some values from stack, so that after
	# 4xenccompact with extra argument $key value is left there...
	if ($i==3)  {	&$Fn	($key,$__key);			}##%edx
	else        {	&mov	($out,$s[0]);			}
			&and	($out,0xFF);
	if ($i==1)  {	&shr	($s[0],16);			}#%ebx[1]
	if ($i==2)  {	&shr	($s[0],24);			}#%ecx[2]
			&movz	($out,&BP(-128,$te,$out,1));

	if ($i==3)  {	$tmp=$s[1];				}##%eax
			&movz	($tmp,&HB($s[1]));
			&movz	($tmp,&BP(-128,$te,$tmp,1));
			&shl	($tmp,8);
			&xor	($out,$tmp);

	if ($i==3)  {	$tmp=$s[2]; &mov ($s[1],$__s0);		}##%ebx
	else        {	&mov	($tmp,$s[2]);
			&shr	($tmp,16);			}
	if ($i==2)  {	&and	($s[1],0xFF);			}#%edx[2]
			&and	($tmp,0xFF);
			&movz	($tmp,&BP(-128,$te,$tmp,1));
			&shl	($tmp,16);
			&xor	($out,$tmp);

	if ($i==3)  {	$tmp=$s[3]; &mov ($s[2],$__s1);		}##%ecx
	elsif($i==2){	&movz	($tmp,&HB($s[3]));		}#%ebx[2]
	else        {	&mov	($tmp,$s[3]);
			&shr	($tmp,24);			}
			&movz	($tmp,&BP(-128,$te,$tmp,1));
			&shl	($tmp,24);
			&xor	($out,$tmp);
	if ($i<2)   {	&mov	(&DWP(4+4*$i,"esp"),$out);	}
	if ($i==3)  {	&mov	($s[3],$acc);			}
	&comment();
}

sub enctransform()
{ my @s = ($s0,$s1,$s2,$s3);
  my $i = shift;
  my $tmp = $tbl;
  my $r2  = $key ;

	&mov	($acc,$s[$i]);
	&and	($acc,0x80808080);
	&mov	($tmp,$acc);
	&shr	($tmp,7);
	&lea	($r2,&DWP(0,$s[$i],$s[$i]));
	&sub	($acc,$tmp);
	&and	($r2,0xfefefefe);
	&and	($acc,0x1b1b1b1b);
	&mov	($tmp,$s[$i]);
	&xor	($acc,$r2);	# r2

	&xor	($s[$i],$acc);	# r0 ^ r2
	&rotl	($s[$i],24);
	&xor	($s[$i],$acc)	# ROTATE(r2^r0,24) ^ r2
	&rotr	($tmp,16);
	&xor	($s[$i],$tmp);
	&rotr	($tmp,8);
	&xor	($s[$i],$tmp);
}

&function_begin_B("_x86_AES_encrypt_compact");
	# note that caller is expected to allocate stack frame for me!
	&mov	($__key,$key);			# save key

	&xor	($s0,&DWP(0,$key));		# xor with key
	&xor	($s1,&DWP(4,$key));
	&xor	($s2,&DWP(8,$key));
	&xor	($s3,&DWP(12,$key));

	&mov	($acc,&DWP(240,$key));		# load key->rounds
	&lea	($acc,&DWP(-2,$acc,$acc));
	&lea	($acc,&DWP(0,$key,$acc,8));
	&mov	($__end,$acc);			# end of key schedule

	# prefetch Te4
	&mov	($key,&DWP(0-128,$tbl));
	&mov	($acc,&DWP(32-128,$tbl));
	&mov	($key,&DWP(64-128,$tbl));
	&mov	($acc,&DWP(96-128,$tbl));
	&mov	($key,&DWP(128-128,$tbl));
	&mov	($acc,&DWP(160-128,$tbl));
	&mov	($key,&DWP(192-128,$tbl));
	&mov	($acc,&DWP(224-128,$tbl));

	&set_label("loop",16);

		&enccompact(0,$tbl,$s0,$s1,$s2,$s3,1);
		&enccompact(1,$tbl,$s1,$s2,$s3,$s0,1);
		&enccompact(2,$tbl,$s2,$s3,$s0,$s1,1);
		&enccompact(3,$tbl,$s3,$s0,$s1,$s2,1);
		&enctransform(2);
		&enctransform(3);
		&enctransform(0);
		&enctransform(1);
		&mov 	($key,$__key);
		&mov	($tbl,$__tbl);
		&add	($key,16);		# advance rd_key
		&xor	($s0,&DWP(0,$key));
		&xor	($s1,&DWP(4,$key));
		&xor	($s2,&DWP(8,$key));
		&xor	($s3,&DWP(12,$key));

	&cmp	($key,$__end);
	&mov	($__key,$key);
	&jb	(&label("loop"));

	&enccompact(0,$tbl,$s0,$s1,$s2,$s3);
	&enccompact(1,$tbl,$s1,$s2,$s3,$s0);
	&enccompact(2,$tbl,$s2,$s3,$s0,$s1);
	&enccompact(3,$tbl,$s3,$s0,$s1,$s2);

	&xor	($s0,&DWP(16,$key));
	&xor	($s1,&DWP(20,$key));
	&xor	($s2,&DWP(24,$key));
	&xor	($s3,&DWP(28,$key));

	&ret	();
&function_end_B("_x86_AES_encrypt_compact");

######################################################################
# "Compact" SSE block function.
######################################################################
#
# Performance is not actually extraordinary in comparison to pure
# x86 code. In particular encrypt performance is virtually the same.
# Decrypt performance on the other hand is 15-20% better on newer
# �-archs [but we're thankful for *any* improvement here], and ~50%
# better on PIII:-) And additionally on the pros side this code
# eliminates redundant references to stack and thus relieves/
# minimizes the pressure on the memory bus.
#
# MMX register layout                           lsb
# +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
# |          mm4          |          mm0          |
# +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
# |     s3    |     s2    |     s1    |     s0    |    
# +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
# |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
# +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
#
# Indexes translate as s[N/4]>>(8*(N%4)), e.g. 5 means s1>>8.
# In this terms encryption and decryption "compact" permutation
# matrices can be depicted as following:
#
# encryption              lsb	# decryption              lsb
# +----++----+----+----+----+	# +----++----+----+----+----+
# | t0 || 15 | 10 |  5 |  0 |	# | t0 ||  7 | 10 | 13 |  0 |
# +----++----+----+----+----+	# +----++----+----+----+----+
# | t1 ||  3 | 14 |  9 |  4 |	# | t1 || 11 | 14 |  1 |  4 |
# +----++----+----+----+----+	# +----++----+----+----+----+
# | t2 ||  7 |  2 | 13 |  8 |	# | t2 || 15 |  2 |  5 |  8 |
# +----++----+----+----+----+	# +----++----+----+----+----+
# | t3 || 11 |  6 |  1 | 12 |	# | t3 ||  3 |  6 |  9 | 12 |
# +----++----+----+----+----+	# +----++----+----+----+----+
#
######################################################################
# Why not xmm registers? Short answer. It was actually tested and
# was not any faster, but *contrary*, most notably on Intel CPUs.
# Longer answer. Main advantage of using mm registers is that movd
# latency is lower, especially on Intel P4. While arithmetic
# instructions are twice as many, they can be scheduled every cycle
# and not every second one when they are operating on xmm register,
# so that "arithmetic throughput" remains virtually the same. And
# finally the code can be executed even on elder SSE-only CPUs:-)

sub sse_enccompact()
{
	&pshufw	("mm1","mm0",0x08);		#  5, 4, 1, 0
	&pshufw	("mm5","mm4",0x0d);		# 15,14,11,10
	&movd	("eax","mm1");			#  5, 4, 1, 0
	&movd	("ebx","mm5");			# 15,14,11,10

	&movz	($acc,&LB("eax"));		#  0
	&movz	("ecx",&BP(-128,$tbl,$acc,1));	#  0
	&pshufw	("mm2","mm0",0x0d);		#  7, 6, 3, 2
	&movz	("edx",&HB("eax"));		#  1
	&movz	("edx",&BP(-128,$tbl,"edx",1));	#  1
	&shl	("edx",8);			#  1
	&shr	("eax",16);			#  5, 4

	&movz	($acc,&LB("ebx"));		# 10
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 10
	&shl	($acc,16);			# 10
	&or	("ecx",$acc);			# 10
	&pshufw	("mm6","mm4",0x08);		# 13,12, 9, 8
	&movz	($acc,&HB("ebx"));		# 11
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 11
	&shl	($acc,24);			# 11
	&or	("edx",$acc);			# 11
	&shr	("ebx",16);			# 15,14

	&movz	($acc,&HB("eax"));		#  5
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  5
	&shl	($acc,8);			#  5
	&or	("ecx",$acc);			#  5
	&movz	($acc,&HB("ebx"));		# 15
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 15
	&shl	($acc,24);			# 15
	&or	("ecx",$acc);			# 15
	&movd	("mm0","ecx");			# t[0] collected

	&movz	($acc,&LB("eax"));		#  4
	&movz	("ecx",&BP(-128,$tbl,$acc,1));	#  4
	&movd	("eax","mm2");			#  7, 6, 3, 2
	&movz	($acc,&LB("ebx"));		# 14
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 14
	&shl	($acc,16);			# 14
	&or	("ecx",$acc);			# 14

	&movd	("ebx","mm6");			# 13,12, 9, 8
	&movz	($acc,&HB("eax"));		#  3
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  3
	&shl	($acc,24);			#  3
	&or	("ecx",$acc);			#  3
	&movz	($acc,&HB("ebx"));		#  9
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  9
	&shl	($acc,8);			#  9
	&or	("ecx",$acc);			#  9
	&movd	("mm1","ecx");			# t[1] collected

	&movz	($acc,&LB("ebx"));		#  8
	&movz	("ecx",&BP(-128,$tbl,$acc,1));	#  8
	&shr	("ebx",16);			# 13,12
	&movz	($acc,&LB("eax"));		#  2
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  2
	&shl	($acc,16);			#  2
	&or	("ecx",$acc);			#  2
	&shr	("eax",16);			#  7, 6

	&punpckldq	("mm0","mm1");		# t[0,1] collected

	&movz	($acc,&HB("eax"));		#  7
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  7
	&shl	($acc,24);			#  7
	&or	("ecx",$acc);			#  7
	&and	("eax",0xff);			#  6
	&movz	("eax",&BP(-128,$tbl,"eax",1));	#  6
	&shl	("eax",16);			#  6
	&or	("edx","eax");			#  6
	&movz	($acc,&HB("ebx"));		# 13
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 13
	&shl	($acc,8);			# 13
	&or	("ecx",$acc);			# 13
	&movd	("mm4","ecx");			# t[2] collected
	&and	("ebx",0xff);			# 12
	&movz	("ebx",&BP(-128,$tbl,"ebx",1));	# 12
	&or	("edx","ebx");			# 12
	&movd	("mm5","edx");			# t[3] collected

	&punpckldq	("mm4","mm5");		# t[2,3] collected
}

					if (!$x86only) {
&function_begin_B("_sse_AES_encrypt_compact");
	&pxor	("mm0",&QWP(0,$key));	#  7, 6, 5, 4, 3, 2, 1, 0
	&pxor	("mm4",&QWP(8,$key));	# 15,14,13,12,11,10, 9, 8

	# note that caller is expected to allocate stack frame for me!
	&mov	($acc,&DWP(240,$key));		# load key->rounds
	&lea	($acc,&DWP(-2,$acc,$acc));
	&lea	($acc,&DWP(0,$key,$acc,8));
	&mov	($__end,$acc);			# end of key schedule

	&mov	($s0,0x1b1b1b1b);		# magic constant
	&mov	(&DWP(8,"esp"),$s0);
	&mov	(&DWP(12,"esp"),$s0);

	# prefetch Te4
	&mov	($s0,&DWP(0-128,$tbl));
	&mov	($s1,&DWP(32-128,$tbl));
	&mov	($s2,&DWP(64-128,$tbl));
	&mov	($s3,&DWP(96-128,$tbl));
	&mov	($s0,&DWP(128-128,$tbl));
	&mov	($s1,&DWP(160-128,$tbl));
	&mov	($s2,&DWP(192-128,$tbl));
	&mov	($s3,&DWP(224-128,$tbl));

	&set_label("loop",16);
		&sse_enccompact();
		&add	($key,16);
		&cmp	($key,$__end);
		&ja	(&label("out"));

		&movq	("mm2",&QWP(8,"esp"));
		&pxor	("mm3","mm3");		&pxor	("mm7","mm7");
		&movq	("mm1","mm0");		&movq	("mm5","mm4");	# r0
		&pcmpgtb("mm3","mm0");		&pcmpgtb("mm7","mm4");
		&pand	("mm3","mm2");		&pand	("mm7","mm2");
		&pshufw	("mm2","mm0",0xb1);	&pshufw	("mm6","mm4",0xb1);# ROTATE(r0,16)
		&paddb	("mm0","mm0");		&paddb	("mm4","mm4");
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# = r2
		&pshufw	("mm3","mm2",0xb1);	&pshufw	("mm7","mm6",0xb1);# r0
		&pxor	("mm1","mm0");		&pxor	("mm5","mm4");	# r0^r2
		&pxor	("mm0","mm2");		&pxor	("mm4","mm6");	# ^= ROTATE(r0,16)

		&movq	("mm2","mm3");		&movq	("mm6","mm7");
		&pslld	("mm3",8);		&pslld	("mm7",8);
		&psrld	("mm2",24);		&psrld	("mm6",24);
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# ^= r0<<8
		&pxor	("mm0","mm2");		&pxor	("mm4","mm6");	# ^= r0>>24

		&movq	("mm3","mm1");		&movq	("mm7","mm5");
		&movq	("mm2",&QWP(0,$key));	&movq	("mm6",&QWP(8,$key));
		&psrld	("mm1",8);		&psrld	("mm5",8);
		&mov	($s0,&DWP(0-128,$tbl));
		&pslld	("mm3",24);		&pslld	("mm7",24);
		&mov	($s1,&DWP(64-128,$tbl));
		&pxor	("mm0","mm1");		&pxor	("mm4","mm5");	# ^= (r2^r0)<<8
		&mov	($s2,&DWP(128-128,$tbl));
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# ^= (r2^r0)>>24
		&mov	($s3,&DWP(192-128,$tbl));

		&pxor	("mm0","mm2");		&pxor	("mm4","mm6");
	&jmp	(&label("loop"));

	&set_label("out",16);
	&pxor	("mm0",&QWP(0,$key));
	&pxor	("mm4",&QWP(8,$key));

	&ret	();
&function_end_B("_sse_AES_encrypt_compact");
					}

######################################################################
# Vanilla block function.
######################################################################

sub encstep()
{ my ($i,$te,@s) = @_;
  my $tmp = $key;
  my $out = $i==3?$s[0]:$acc;

	# lines marked with #%e?x[i] denote "reordered" instructions...
	if ($i==3)  {	&mov	($key,$__key);			}##%edx
	else        {	&mov	($out,$s[0]);
			&and	($out,0xFF);			}
	if ($i==1)  {	&shr	($s[0],16);			}#%ebx[1]
	if ($i==2)  {	&shr	($s[0],24);			}#%ecx[2]
			&mov	($out,&DWP(0,$te,$out,8));

	if ($i==3)  {	$tmp=$s[1];				}##%eax
			&movz	($tmp,&HB($s[1]));
			&xor	($out,&DWP(3,$te,$tmp,8));

	if ($i==3)  {	$tmp=$s[2]; &mov ($s[1],$__s0);		}##%ebx
	else        {	&mov	($tmp,$s[2]);
			&shr	($tmp,16);			}
	if ($i==2)  {	&and	($s[1],0xFF);			}#%edx[2]
			&and	($tmp,0xFF);
			&xor	($out,&DWP(2,$te,$tmp,8));

	if ($i==3)  {	$tmp=$s[3]; &mov ($s[2],$__s1);		}##%ecx
	elsif($i==2){	&movz	($tmp,&HB($s[3]));		}#%ebx[2]
	else        {	&mov	($tmp,$s[3]); 
			&shr	($tmp,24)			}
			&xor	($out,&DWP(1,$te,$tmp,8));
	if ($i<2)   {	&mov	(&DWP(4+4*$i,"esp"),$out);	}
	if ($i==3)  {	&mov	($s[3],$acc);			}
			&comment();
}

sub enclast()
{ my ($i,$te,@s)=@_;
  my $tmp = $key;
  my $out = $i==3?$s[0]:$acc;

	if ($i==3)  {	&mov	($key,$__key);			}##%edx
	else        {	&mov	($out,$s[0]);			}
			&and	($out,0xFF);
	if ($i==1)  {	&shr	($s[0],16);			}#%ebx[1]
	if ($i==2)  {	&shr	($s[0],24);			}#%ecx[2]
			&mov	($out,&DWP(2,$te,$out,8));
			&and	($out,0x000000ff);

	if ($i==3)  {	$tmp=$s[1];				}##%eax
			&movz	($tmp,&HB($s[1]));
			&mov	($tmp,&DWP(0,$te,$tmp,8));
			&and	($tmp,0x0000ff00);
			&xor	($out,$tmp);

	if ($i==3)  {	$tmp=$s[2]; &mov ($s[1],$__s0);		}##%ebx
	else        {	&mov	($tmp,$s[2]);
			&shr	($tmp,16);			}
	if ($i==2)  {	&and	($s[1],0xFF);			}#%edx[2]
			&and	($tmp,0xFF);
			&mov	($tmp,&DWP(0,$te,$tmp,8));
			&and	($tmp,0x00ff0000);
			&xor	($out,$tmp);

	if ($i==3)  {	$tmp=$s[3]; &mov ($s[2],$__s1);		}##%ecx
	elsif($i==2){	&movz	($tmp,&HB($s[3]));		}#%ebx[2]
	else        {	&mov	($tmp,$s[3]);
			&shr	($tmp,24);			}
			&mov	($tmp,&DWP(2,$te,$tmp,8));
			&and	($tmp,0xff000000);
			&xor	($out,$tmp);
	if ($i<2)   {	&mov	(&DWP(4+4*$i,"esp"),$out);	}
	if ($i==3)  {	&mov	($s[3],$acc);			}
}

&function_begin_B("_x86_AES_encrypt");
	if ($vertical_spin) {
		# I need high parts of volatile registers to be accessible...
		&exch	($s1="edi",$key="ebx");
		&mov	($s2="esi",$acc="ecx");
	}

	# note that caller is expected to allocate stack frame for me!
	&mov	($__key,$key);			# save key

	&xor	($s0,&DWP(0,$key));		# xor with key
	&xor	($s1,&DWP(4,$key));
	&xor	($s2,&DWP(8,$key));
	&xor	($s3,&DWP(12,$key));

	&mov	($acc,&DWP(240,$key));		# load key->rounds

	if ($small_footprint) {
	    &lea	($acc,&DWP(-2,$acc,$acc));
	    &lea	($acc,&DWP(0,$key,$acc,8));
	    &mov	($__end,$acc);		# end of key schedule

	    &set_label("loop",16);
		if ($vertical_spin) {
		    &encvert($tbl,$s0,$s1,$s2,$s3);
		} else {
		    &encstep(0,$tbl,$s0,$s1,$s2,$s3);
		    &encstep(1,$tbl,$s1,$s2,$s3,$s0);
		    &encstep(2,$tbl,$s2,$s3,$s0,$s1);
		    &encstep(3,$tbl,$s3,$s0,$s1,$s2);
		}
		&add	($key,16);		# advance rd_key
		&xor	($s0,&DWP(0,$key));
		&xor	($s1,&DWP(4,$key));
		&xor	($s2,&DWP(8,$key));
		&xor	($s3,&DWP(12,$key));
	    &cmp	($key,$__end);
	    &mov	($__key,$key);
	    &jb		(&label("loop"));
	}
	else {
	    &cmp	($acc,10);
	    &jle	(&label("10rounds"));
	    &cmp	($acc,12);
	    &jle	(&label("12rounds"));

	&set_label("14rounds",4);
	    for ($i=1;$i<3;$i++) {
		if ($vertical_spin) {
		    &encvert($tbl,$s0,$s1,$s2,$s3);
		} else {
		    &encstep(0,$tbl,$s0,$s1,$s2,$s3);
		    &encstep(1,$tbl,$s1,$s2,$s3,$s0);
		    &encstep(2,$tbl,$s2,$s3,$s0,$s1);
		    &encstep(3,$tbl,$s3,$s0,$s1,$s2);
		}
		&xor	($s0,&DWP(16*$i+0,$key));
		&xor	($s1,&DWP(16*$i+4,$key));
		&xor	($s2,&DWP(16*$i+8,$key));
		&xor	($s3,&DWP(16*$i+12,$key));
	    }
	    &add	($key,32);
	    &mov	($__key,$key);		# advance rd_key
	&set_label("12rounds",4);
	    for ($i=1;$i<3;$i++) {
		if ($vertical_spin) {
		    &encvert($tbl,$s0,$s1,$s2,$s3);
		} else {
		    &encstep(0,$tbl,$s0,$s1,$s2,$s3);
		    &encstep(1,$tbl,$s1,$s2,$s3,$s0);
		    &encstep(2,$tbl,$s2,$s3,$s0,$s1);
		    &encstep(3,$tbl,$s3,$s0,$s1,$s2);
		}
		&xor	($s0,&DWP(16*$i+0,$key));
		&xor	($s1,&DWP(16*$i+4,$key));
		&xor	($s2,&DWP(16*$i+8,$key));
		&xor	($s3,&DWP(16*$i+12,$key));
	    }
	    &add	($key,32);
	    &mov	($__key,$key);		# advance rd_key
	&set_label("10rounds",4);
	    for ($i=1;$i<10;$i++) {
		if ($vertical_spin) {
		    &encvert($tbl,$s0,$s1,$s2,$s3);
		} else {
		    &encstep(0,$tbl,$s0,$s1,$s2,$s3);
		    &encstep(1,$tbl,$s1,$s2,$s3,$s0);
		    &encstep(2,$tbl,$s2,$s3,$s0,$s1);
		    &encstep(3,$tbl,$s3,$s0,$s1,$s2);
		}
		&xor	($s0,&DWP(16*$i+0,$key));
		&xor	($s1,&DWP(16*$i+4,$key));
		&xor	($s2,&DWP(16*$i+8,$key));
		&xor	($s3,&DWP(16*$i+12,$key));
	    }
	}

	if ($vertical_spin) {
	    # "reincarnate" some registers for "horizontal" spin...
	    &mov	($s1="ebx",$key="edi");
	    &mov	($s2="ecx",$acc="esi");
	}
	&enclast(0,$tbl,$s0,$s1,$s2,$s3);
	&enclast(1,$tbl,$s1,$s2,$s3,$s0);
	&enclast(2,$tbl,$s2,$s3,$s0,$s1);
	&enclast(3,$tbl,$s3,$s0,$s1,$s2);

	&add	($key,$small_footprint?16:160);
	&xor	($s0,&DWP(0,$key));
	&xor	($s1,&DWP(4,$key));
	&xor	($s2,&DWP(8,$key));
	&xor	($s3,&DWP(12,$key));

	&ret	();

&set_label("AES_Te",64);	# Yes! I keep it in the code segment!
	&_data_word(0xa56363c6, 0x847c7cf8, 0x997777ee, 0x8d7b7bf6);
	&_data_word(0x0df2f2ff, 0xbd6b6bd6, 0xb16f6fde, 0x54c5c591);
	&_data_word(0x50303060, 0x03010102, 0xa96767ce, 0x7d2b2b56);
	&_data_word(0x19fefee7, 0x62d7d7b5, 0xe6abab4d, 0x9a7676ec);
	&_data_word(0x45caca8f, 0x9d82821f, 0x40c9c989, 0x877d7dfa);
	&_data_word(0x15fafaef, 0xeb5959b2, 0xc947478e, 0x0bf0f0fb);
	&_data_word(0xecadad41, 0x67d4d4b3, 0xfda2a25f, 0xeaafaf45);
	&_data_word(0xbf9c9c23, 0xf7a4a453, 0x967272e4, 0x5bc0c09b);
	&_data_word(0xc2b7b775, 0x1cfdfde1, 0xae93933d, 0x6a26264c);
	&_data_word(0x5a36366c, 0x413f3f7e, 0x02f7f7f5, 0x4fcccc83);
	&_data_word(0x5c343468, 0xf4a5a551, 0x34e5e5d1, 0x08f1f1f9);
	&_data_word(0x937171e2, 0x73d8d8ab, 0x53313162, 0x3f15152a);
	&_data_word(0x0c040408, 0x52c7c795, 0x65232346, 0x5ec3c39d);
	&_data_word(0x28181830, 0xa1969637, 0x0f05050a, 0xb59a9a2f);
	&_data_word(0x0907070e, 0x36121224, 0x9b80801b, 0x3de2e2df);
	&_data_word(0x26ebebcd, 0x6927274e, 0xcdb2b27f, 0x9f7575ea);
	&_data_word(0x1b090912, 0x9e83831d, 0x742c2c58, 0x2e1a1a34);
	&_data_word(0x2d1b1b36, 0xb26e6edc, 0xee5a5ab4, 0xfba0a05b);
	&_data_word(0xf65252a4, 0x4d3b3b76, 0x61d6d6b7, 0xceb3b37d);
	&_data_word(0x7b292952, 0x3ee3e3dd, 0x712f2f5e, 0x97848413);
	&_data_word(0xf55353a6, 0x68d1d1b9, 0x00000000, 0x2cededc1);
	&_data_word(0x60202040, 0x1ffcfce3, 0xc8b1b179, 0xed5b5bb6);
	&_data_word(0xbe6a6ad4, 0x46cbcb8d, 0xd9bebe67, 0x4b393972);
	&_data_word(0xde4a4a94, 0xd44c4c98, 0xe85858b0, 0x4acfcf85);
	&_data_word(0x6bd0d0bb, 0x2aefefc5, 0xe5aaaa4f, 0x16fbfbed);
	&_data_word(0xc5434386, 0xd74d4d9a, 0x55333366, 0x94858511);
	&_data_word(0xcf45458a, 0x10f9f9e9, 0x06020204, 0x817f7ffe);
	&_data_word(0xf05050a0, 0x443c3c78, 0xba9f9f25, 0xe3a8a84b);
	&_data_word(0xf35151a2, 0xfea3a35d, 0xc0404080, 0x8a8f8f05);
	&_data_word(0xad92923f, 0xbc9d9d21, 0x48383870, 0x04f5f5f1);
	&_data_word(0xdfbcbc63, 0xc1b6b677, 0x75dadaaf, 0x63212142);
	&_data_word(0x30101020, 0x1affffe5, 0x0ef3f3fd, 0x6dd2d2bf);
	&_data_word(0x4ccdcd81, 0x140c0c18, 0x35131326, 0x2fececc3);
	&_data_word(0xe15f5fbe, 0xa2979735, 0xcc444488, 0x3917172e);
	&_data_word(0x57c4c493, 0xf2a7a755, 0x827e7efc, 0x473d3d7a);
	&_data_word(0xac6464c8, 0xe75d5dba, 0x2b191932, 0x957373e6);
	&_data_word(0xa06060c0, 0x98818119, 0xd14f4f9e, 0x7fdcdca3);
	&_data_word(0x66222244, 0x7e2a2a54, 0xab90903b, 0x8388880b);
	&_data_word(0xca46468c, 0x29eeeec7, 0xd3b8b86b, 0x3c141428);
	&_data_word(0x79dedea7, 0xe25e5ebc, 0x1d0b0b16, 0x76dbdbad);
	&_data_word(0x3be0e0db, 0x56323264, 0x4e3a3a74, 0x1e0a0a14);
	&_data_word(0xdb494992, 0x0a06060c, 0x6c242448, 0xe45c5cb8);
	&_data_word(0x5dc2c29f, 0x6ed3d3bd, 0xefacac43, 0xa66262c4);
	&_data_word(0xa8919139, 0xa4959531, 0x37e4e4d3, 0x8b7979f2);
	&_data_word(0x32e7e7d5, 0x43c8c88b, 0x5937376e, 0xb76d6dda);
	&_data_word(0x8c8d8d01, 0x64d5d5b1, 0xd24e4e9c, 0xe0a9a949);
	&_data_word(0xb46c6cd8, 0xfa5656ac, 0x07f4f4f3, 0x25eaeacf);
	&_data_word(0xaf6565ca, 0x8e7a7af4, 0xe9aeae47, 0x18080810);
	&_data_word(0xd5baba6f, 0x887878f0, 0x6f25254a, 0x722e2e5c);
	&_data_word(0x241c1c38, 0xf1a6a657, 0xc7b4b473, 0x51c6c697);
	&_data_word(0x23e8e8cb, 0x7cdddda1, 0x9c7474e8, 0x211f1f3e);
	&_data_word(0xdd4b4b96, 0xdcbdbd61, 0x868b8b0d, 0x858a8a0f);
	&_data_word(0x907070e0, 0x423e3e7c, 0xc4b5b571, 0xaa6666cc);
	&_data_word(0xd8484890, 0x05030306, 0x01f6f6f7, 0x120e0e1c);
	&_data_word(0xa36161c2, 0x5f35356a, 0xf95757ae, 0xd0b9b969);
	&_data_word(0x91868617, 0x58c1c199, 0x271d1d3a, 0xb99e9e27);
	&_data_word(0x38e1e1d9, 0x13f8f8eb, 0xb398982b, 0x33111122);
	&_data_word(0xbb6969d2, 0x70d9d9a9, 0x898e8e07, 0xa7949433);
	&_data_word(0xb69b9b2d, 0x221e1e3c, 0x92878715, 0x20e9e9c9);
	&_data_word(0x49cece87, 0xff5555aa, 0x78282850, 0x7adfdfa5);
	&_data_word(0x8f8c8c03, 0xf8a1a159, 0x80898909, 0x170d0d1a);
	&_data_word(0xdabfbf65, 0x31e6e6d7, 0xc6424284, 0xb86868d0);
	&_data_word(0xc3414182, 0xb0999929, 0x772d2d5a, 0x110f0f1e);
	&_data_word(0xcbb0b07b, 0xfc5454a8, 0xd6bbbb6d, 0x3a16162c);

#Te4	# four copies of Te4 to choose from to avoid L1 aliasing
	&data_byte(0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5);
	&data_byte(0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76);
	&data_byte(0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0);
	&data_byte(0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0);
	&data_byte(0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc);
	&data_byte(0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15);
	&data_byte(0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a);
	&data_byte(0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75);
	&data_byte(0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0);
	&data_byte(0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84);
	&data_byte(0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b);
	&data_byte(0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf);
	&data_byte(0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85);
	&data_byte(0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8);
	&data_byte(0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5);
	&data_byte(0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2);
	&data_byte(0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17);
	&data_byte(0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73);
	&data_byte(0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88);
	&data_byte(0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb);
	&data_byte(0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c);
	&data_byte(0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79);
	&data_byte(0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9);
	&data_byte(0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08);
	&data_byte(0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6);
	&data_byte(0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a);
	&data_byte(0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e);
	&data_byte(0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e);
	&data_byte(0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94);
	&data_byte(0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf);
	&data_byte(0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68);
	&data_byte(0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16);

	&data_byte(0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5);
	&data_byte(0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76);
	&data_byte(0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0);
	&data_byte(0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0);
	&data_byte(0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc);
	&data_byte(0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15);
	&data_byte(0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a);
	&data_byte(0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75);
	&data_byte(0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0);
	&data_byte(0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84);
	&data_byte(0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b);
	&data_byte(0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf);
	&data_byte(0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85);
	&data_byte(0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8);
	&data_byte(0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5);
	&data_byte(0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2);
	&data_byte(0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17);
	&data_byte(0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73);
	&data_byte(0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88);
	&data_byte(0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb);
	&data_byte(0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c);
	&data_byte(0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79);
	&data_byte(0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9);
	&data_byte(0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08);
	&data_byte(0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6);
	&data_byte(0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a);
	&data_byte(0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e);
	&data_byte(0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e);
	&data_byte(0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94);
	&data_byte(0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf);
	&data_byte(0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68);
	&data_byte(0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16);

	&data_byte(0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5);
	&data_byte(0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76);
	&data_byte(0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0);
	&data_byte(0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0);
	&data_byte(0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc);
	&data_byte(0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15);
	&data_byte(0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a);
	&data_byte(0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75);
	&data_byte(0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0);
	&data_byte(0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84);
	&data_byte(0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b);
	&data_byte(0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf);
	&data_byte(0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85);
	&data_byte(0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8);
	&data_byte(0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5);
	&data_byte(0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2);
	&data_byte(0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17);
	&data_byte(0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73);
	&data_byte(0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88);
	&data_byte(0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb);
	&data_byte(0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c);
	&data_byte(0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79);
	&data_byte(0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9);
	&data_byte(0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08);
	&data_byte(0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6);
	&data_byte(0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a);
	&data_byte(0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e);
	&data_byte(0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e);
	&data_byte(0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94);
	&data_byte(0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf);
	&data_byte(0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68);
	&data_byte(0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16);

	&data_byte(0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5);
	&data_byte(0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76);
	&data_byte(0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0);
	&data_byte(0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0);
	&data_byte(0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc);
	&data_byte(0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15);
	&data_byte(0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a);
	&data_byte(0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75);
	&data_byte(0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0);
	&data_byte(0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84);
	&data_byte(0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b);
	&data_byte(0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf);
	&data_byte(0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85);
	&data_byte(0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8);
	&data_byte(0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5);
	&data_byte(0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2);
	&data_byte(0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17);
	&data_byte(0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73);
	&data_byte(0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88);
	&data_byte(0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb);
	&data_byte(0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c);
	&data_byte(0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79);
	&data_byte(0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9);
	&data_byte(0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08);
	&data_byte(0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6);
	&data_byte(0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a);
	&data_byte(0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e);
	&data_byte(0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e);
	&data_byte(0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94);
	&data_byte(0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf);
	&data_byte(0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68);
	&data_byte(0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16);
#rcon:
	&data_word(0x00000001, 0x00000002, 0x00000004, 0x00000008);
	&data_word(0x00000010, 0x00000020, 0x00000040, 0x00000080);
	&data_word(0x0000001b, 0x00000036, 0x00000000, 0x00000000);
	&data_word(0x00000000, 0x00000000, 0x00000000, 0x00000000);
&function_end_B("_x86_AES_encrypt");

# void AES_encrypt (const void *inp,void *out,const AES_KEY *key);
&function_begin("AES_encrypt");
	&mov	($acc,&wparam(0));		# load inp
	&mov	($key,&wparam(2));		# load key

	&mov	($s0,"esp");
	&sub	("esp",36);
	&and	("esp",-64);			# align to cache-line

	# place stack frame just "above" the key schedule
	&lea	($s1,&DWP(-64-63,$key));
	&sub	($s1,"esp");
	&neg	($s1);
	&and	($s1,0x3C0);	# modulo 1024, but aligned to cache-line
	&sub	("esp",$s1);
	&add	("esp",4);	# 4 is reserved for caller's return address
	&mov	($_esp,$s0);			# save stack pointer

	&call   (&label("pic_point"));          # make it PIC!
	&set_label("pic_point");
	&blindpop($tbl);
	&picmeup($s0,"OPENSSL_ia32cap_P",$tbl,&label("pic_point")) if (!$x86only);
	&lea    ($tbl,&DWP(&label("AES_Te")."-".&label("pic_point"),$tbl));

	# pick Te4 copy which can't "overlap" with stack frame or key schedule
	&lea	($s1,&DWP(768-4,"esp"));
	&sub	($s1,$tbl);
	&and	($s1,0x300);
	&lea	($tbl,&DWP(2048+128,$tbl,$s1));

					if (!$x86only) {
	&bt	(&DWP(0,$s0),25);	# check for SSE bit
	&jnc	(&label("x86"));

	&movq	("mm0",&QWP(0,$acc));
	&movq	("mm4",&QWP(8,$acc));
	&call	("_sse_AES_encrypt_compact");
	&mov	("esp",$_esp);			# restore stack pointer
	&mov	($acc,&wparam(1));		# load out
	&movq	(&QWP(0,$acc),"mm0");		# write output data
	&movq	(&QWP(8,$acc),"mm4");
	&emms	();
	&function_end_A();
					}
	&set_label("x86",16);
	&mov	($_tbl,$tbl);
	&mov	($s0,&DWP(0,$acc));		# load input data
	&mov	($s1,&DWP(4,$acc));
	&mov	($s2,&DWP(8,$acc));
	&mov	($s3,&DWP(12,$acc));
	&call	("_x86_AES_encrypt_compact");
	&mov	("esp",$_esp);			# restore stack pointer
	&mov	($acc,&wparam(1));		# load out
	&mov	(&DWP(0,$acc),$s0);		# write output data
	&mov	(&DWP(4,$acc),$s1);
	&mov	(&DWP(8,$acc),$s2);
	&mov	(&DWP(12,$acc),$s3);
&function_end("AES_encrypt");

#--------------------------------------------------------------------#

######################################################################
# "Compact" block function
######################################################################

sub deccompact()
{ my $Fn = mov;
  while ($#_>5) { pop(@_); $Fn=sub{}; }
  my ($i,$td,@s)=@_;
  my $tmp = $key;
  my $out = $i==3?$s[0]:$acc;

	# $Fn is used in first compact round and its purpose is to
	# void restoration of some values from stack, so that after
	# 4xdeccompact with extra argument $key, $s0 and $s1 values
	# are left there...
	if($i==3)   {	&$Fn	($key,$__key);			}
	else        {	&mov	($out,$s[0]);			}
			&and	($out,0xFF);
			&movz	($out,&BP(-128,$td,$out,1));

	if ($i==3)  {	$tmp=$s[1];				}
			&movz	($tmp,&HB($s[1]));
			&movz	($tmp,&BP(-128,$td,$tmp,1));
			&shl	($tmp,8);
			&xor	($out,$tmp);

	if ($i==3)  {	$tmp=$s[2]; &mov ($s[1],$acc);		}
	else        {	mov	($tmp,$s[2]);			}
			&shr	($tmp,16);
			&and	($tmp,0xFF);
			&movz	($tmp,&BP(-128,$td,$tmp,1));
			&shl	($tmp,16);
			&xor	($out,$tmp);

	if ($i==3)  {	$tmp=$s[3]; &$Fn ($s[2],$__s1);		}
	else        {	&mov	($tmp,$s[3]);			}
			&shr	($tmp,24);
			&movz	($tmp,&BP(-128,$td,$tmp,1));
			&shl	($tmp,24);
			&xor	($out,$tmp);
	if ($i<2)   {	&mov	(&DWP(4+4*$i,"esp"),$out);	}
	if ($i==3)  {	&$Fn	($s[3],$__s0);			}
}

# must be called with 2,3,0,1 as argument sequence!!!
sub dectransform()
{ my @s = ($s0,$s1,$s2,$s3);
  my $i = shift;
  my $tmp = $key;
  my $tp2 = @s[($i+2)%4]; $tp2 = @s[2] if ($i==1);
  my $tp4 = @s[($i+3)%4]; $tp4 = @s[3] if ($i==1);
  my $tp8 = $tbl;

	&mov	($acc,$s[$i]);
	&and	($acc,0x80808080);
	&mov	($tmp,$acc);
	&shr	($tmp,7);
	&lea	($tp2,&DWP(0,$s[$i],$s[$i]));
	&sub	($acc,$tmp);
	&and	($tp2,0xfefefefe);
	&and	($acc,0x1b1b1b1b);
	&xor	($acc,$tp2);
	&mov	($tp2,$acc);

	&and	($acc,0x80808080);
	&mov	($tmp,$acc);
	&shr	($tmp,7);
	&lea	($tp4,&DWP(0,$tp2,$tp2));
	&sub	($acc,$tmp);
	&and	($tp4,0xfefefefe);
	&and	($acc,0x1b1b1b1b);
	 &xor	($tp2,$s[$i]);	# tp2^tp1
	&xor	($acc,$tp4);
	&mov	($tp4,$acc);

	&and	($acc,0x80808080);
	&mov	($tmp,$acc);
	&shr	($tmp,7);
	&lea	($tp8,&DWP(0,$tp4,$tp4));
	&sub	($acc,$tmp);
	&and	($tp8,0xfefefefe);
	&and	($acc,0x1b1b1b1b);
	 &xor	($tp4,$s[$i]);	# tp4^tp1
	 &rotl	($s[$i],8);	# = ROTATE(tp1,8)
	&xor	($tp8,$acc);

	&xor	($s[$i],$tp2);
	&xor	($tp2,$tp8);
	&rotl	($tp2,24);
	&xor	($s[$i],$tp4);
	&xor	($tp4,$tp8);
	&rotl	($tp4,16);
	&xor	($s[$i],$tp8);	# ^= tp8^(tp4^tp1)^(tp2^tp1)
	&rotl	($tp8,8);
	&xor	($s[$i],$tp2);	# ^= ROTATE(tp8^tp2^tp1,24)
	&xor	($s[$i],$tp4);	# ^= ROTATE(tp8^tp4^tp1,16)
	 &mov	($s[0],$__s0)			if($i==2); #prefetch $s0
	 &mov	($s[1],$__s1)			if($i==3); #prefetch $s1
	 &mov	($s[2],$__s2)			if($i==1);
	&xor	($s[$i],$tp8);	# ^= ROTATE(tp8,8)

	&mov	($s[3],$__s3)			if($i==1);
	&mov	(&DWP(4+4*$i,"esp"),$s[$i])	if($i>=2);
}

&function_begin_B("_x86_AES_decrypt_compact");
	# note that caller is expected to allocate stack frame for me!
	&mov	($__key,$key);			# save key

	&xor	($s0,&DWP(0,$key));		# xor with key
	&xor	($s1,&DWP(4,$key));
	&xor	($s2,&DWP(8,$key));
	&xor	($s3,&DWP(12,$key));

	&mov	($acc,&DWP(240,$key));		# load key->rounds

	&lea	($acc,&DWP(-2,$acc,$acc));
	&lea	($acc,&DWP(0,$key,$acc,8));
	&mov	($__end,$acc);			# end of key schedule

	# prefetch Td4
	&mov	($key,&DWP(0-128,$tbl));
	&mov	($acc,&DWP(32-128,$tbl));
	&mov	($key,&DWP(64-128,$tbl));
	&mov	($acc,&DWP(96-128,$tbl));
	&mov	($key,&DWP(128-128,$tbl));
	&mov	($acc,&DWP(160-128,$tbl));
	&mov	($key,&DWP(192-128,$tbl));
	&mov	($acc,&DWP(224-128,$tbl));

	&set_label("loop",16);

		&deccompact(0,$tbl,$s0,$s3,$s2,$s1,1);
		&deccompact(1,$tbl,$s1,$s0,$s3,$s2,1);
		&deccompact(2,$tbl,$s2,$s1,$s0,$s3,1);
		&deccompact(3,$tbl,$s3,$s2,$s1,$s0,1);
		&dectransform(2);
		&dectransform(3);
		&dectransform(0);
		&dectransform(1);
		&mov 	($key,$__key);
		&mov	($tbl,$__tbl);
		&add	($key,16);		# advance rd_key
		&xor	($s0,&DWP(0,$key));
		&xor	($s1,&DWP(4,$key));
		&xor	($s2,&DWP(8,$key));
		&xor	($s3,&DWP(12,$key));

	&cmp	($key,$__end);
	&mov	($__key,$key);
	&jb	(&label("loop"));

	&deccompact(0,$tbl,$s0,$s3,$s2,$s1);
	&deccompact(1,$tbl,$s1,$s0,$s3,$s2);
	&deccompact(2,$tbl,$s2,$s1,$s0,$s3);
	&deccompact(3,$tbl,$s3,$s2,$s1,$s0);

	&xor	($s0,&DWP(16,$key));
	&xor	($s1,&DWP(20,$key));
	&xor	($s2,&DWP(24,$key));
	&xor	($s3,&DWP(28,$key));

	&ret	();
&function_end_B("_x86_AES_decrypt_compact");

######################################################################
# "Compact" SSE block function.
######################################################################

sub sse_deccompact()
{
	&pshufw	("mm1","mm0",0x0c);		#  7, 6, 1, 0
	&movd	("eax","mm1");			#  7, 6, 1, 0

	&pshufw	("mm5","mm4",0x09);		# 13,12,11,10
	&movz	($acc,&LB("eax"));		#  0
	&movz	("ecx",&BP(-128,$tbl,$acc,1));	#  0
	&movd	("ebx","mm5");			# 13,12,11,10
	&movz	("edx",&HB("eax"));		#  1
	&movz	("edx",&BP(-128,$tbl,"edx",1));	#  1
	&shl	("edx",8);			#  1

	&pshufw	("mm2","mm0",0x06);		#  3, 2, 5, 4
	&movz	($acc,&LB("ebx"));		# 10
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 10
	&shl	($acc,16);			# 10
	&or	("ecx",$acc);			# 10
	&shr	("eax",16);			#  7, 6
	&movz	($acc,&HB("ebx"));		# 11
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 11
	&shl	($acc,24);			# 11
	&or	("edx",$acc);			# 11
	&shr	("ebx",16);			# 13,12

	&pshufw	("mm6","mm4",0x03);		# 9, 8,15,14
	&movz	($acc,&HB("eax"));		#  7
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  7
	&shl	($acc,24);			#  7
	&or	("ecx",$acc);			#  7
	&movz	($acc,&HB("ebx"));		# 13
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 13
	&shl	($acc,8);			# 13
	&or	("ecx",$acc);			# 13
	&movd	("mm0","ecx");			# t[0] collected

	&movz	($acc,&LB("eax"));		#  6
	&movd	("eax","mm2");			#  3, 2, 5, 4
	&movz	("ecx",&BP(-128,$tbl,$acc,1));	#  6
	&shl	("ecx",16);			#  6
	&movz	($acc,&LB("ebx"));		# 12
	&movd	("ebx","mm6");			#  9, 8,15,14
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 12
	&or	("ecx",$acc);			# 12

	&movz	($acc,&LB("eax"));		#  4
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  4
	&or	("edx",$acc);			#  4
	&movz	($acc,&LB("ebx"));		# 14
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 14
	&shl	($acc,16);			# 14
	&or	("edx",$acc);			# 14
	&movd	("mm1","edx");			# t[1] collected

	&movz	($acc,&HB("eax"));		#  5
	&movz	("edx",&BP(-128,$tbl,$acc,1));	#  5
	&shl	("edx",8);			#  5
	&movz	($acc,&HB("ebx"));		# 15
	&shr	("eax",16);			#  3, 2
	&movz	($acc,&BP(-128,$tbl,$acc,1));	# 15
	&shl	($acc,24);			# 15
	&or	("edx",$acc);			# 15
	&shr	("ebx",16);			#  9, 8

	&punpckldq	("mm0","mm1");		# t[0,1] collected

	&movz	($acc,&HB("ebx"));		#  9
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  9
	&shl	($acc,8);			#  9
	&or	("ecx",$acc);			#  9
	&and	("ebx",0xff);			#  8
	&movz	("ebx",&BP(-128,$tbl,"ebx",1));	#  8
	&or	("edx","ebx");			#  8
	&movz	($acc,&LB("eax"));		#  2
	&movz	($acc,&BP(-128,$tbl,$acc,1));	#  2
	&shl	($acc,16);			#  2
	&or	("edx",$acc);			#  2
	&movd	("mm4","edx");			# t[2] collected
	&movz	("eax",&HB("eax"));		#  3
	&movz	("eax",&BP(-128,$tbl,"eax",1));	#  3
	&shl	("eax",24);			#  3
	&or	("ecx","eax");			#  3
	&movd	("mm5","ecx");			# t[3] collected

	&punpckldq	("mm4","mm5");		# t[2,3] collected
}

					if (!$x86only) {
&function_begin_B("_sse_AES_decrypt_compact");
	&pxor	("mm0",&QWP(0,$key));	#  7, 6, 5, 4, 3, 2, 1, 0
	&pxor	("mm4",&QWP(8,$key));	# 15,14,13,12,11,10, 9, 8

	# note that caller is expected to allocate stack frame for me!
	&mov	($acc,&DWP(240,$key));		# load key->rounds
	&lea	($acc,&DWP(-2,$acc,$acc));
	&lea	($acc,&DWP(0,$key,$acc,8));
	&mov	($__end,$acc);			# end of key schedule

	&mov	($s0,0x1b1b1b1b);		# magic constant
	&mov	(&DWP(8,"esp"),$s0);
	&mov	(&DWP(12,"esp"),$s0);

	# prefetch Td4
	&mov	($s0,&DWP(0-128,$tbl));
	&mov	($s1,&DWP(32-128,$tbl));
	&mov	($s2,&DWP(64-128,$tbl));
	&mov	($s3,&DWP(96-128,$tbl));
	&mov	($s0,&DWP(128-128,$tbl));
	&mov	($s1,&DWP(160-128,$tbl));
	&mov	($s2,&DWP(192-128,$tbl));
	&mov	($s3,&DWP(224-128,$tbl));

	&set_label("loop",16);
		&sse_deccompact();
		&add	($key,16);
		&cmp	($key,$__end);
		&ja	(&label("out"));

		# ROTATE(x^y,N) == ROTATE(x,N)^ROTATE(y,N)
		&movq	("mm3","mm0");		&movq	("mm7","mm4");
		&movq	("mm2","mm0",1);	&movq	("mm6","mm4",1);
		&movq	("mm1","mm0");		&movq	("mm5","mm4");
		&pshufw	("mm0","mm0",0xb1);	&pshufw	("mm4","mm4",0xb1);# = ROTATE(tp0,16)
		&pslld	("mm2",8);		&pslld	("mm6",8);
		&psrld	("mm3",8);		&psrld	("mm7",8);
		&pxor	("mm0","mm2");		&pxor	("mm4","mm6");	# ^= tp0<<8
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# ^= tp0>>8
		&pslld	("mm2",16);		&pslld	("mm6",16);
		&psrld	("mm3",16);		&psrld	("mm7",16);
		&pxor	("mm0","mm2");		&pxor	("mm4","mm6");	# ^= tp0<<24
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# ^= tp0>>24

		&movq	("mm3",&QWP(8,"esp"));
		&pxor	("mm2","mm2");		&pxor	("mm6","mm6");
		&pcmpgtb("mm2","mm1");		&pcmpgtb("mm6","mm5");
		&pand	("mm2","mm3");		&pand	("mm6","mm3");
		&paddb	("mm1","mm1");		&paddb	("mm5","mm5");
		&pxor	("mm1","mm2");		&pxor	("mm5","mm6");	# tp2
		&movq	("mm3","mm1");		&movq	("mm7","mm5");
		&movq	("mm2","mm1");		&movq	("mm6","mm5");
		&pxor	("mm0","mm1");		&pxor	("mm4","mm5");	# ^= tp2
		&pslld	("mm3",24);		&pslld	("mm7",24);
		&psrld	("mm2",8);		&psrld	("mm6",8);
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# ^= tp2<<24
		&pxor	("mm0","mm2");		&pxor	("mm4","mm6");	# ^= tp2>>8

		&movq	("mm2",&QWP(8,"esp"));
		&pxor	("mm3","mm3");		&pxor	("mm7","mm7");
		&pcmpgtb("mm3","mm1");		&pcmpgtb("mm7","mm5");
		&pand	("mm3","mm2");		&pand	("mm7","mm2");
		&paddb	("mm1","mm1");		&paddb	("mm5","mm5");
		&pxor	("mm1","mm3");		&pxor	("mm5","mm7");	# tp4
		&pshufw	("mm3","mm1",0xb1);	&pshufw	("mm7","mm5",0xb1);
		&pxor	("mm0","mm1");		&pxor	("mm4","mm5");	# ^= tp4
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# ^= ROTATE(tp4,16)	

		&pxor	("mm3","mm3");		&pxor	("mm7","mm7");
		&pcmpgtb("mm3","mm1");		&pcmpgtb("mm7","mm5");
		&pand	("mm3","mm2");		&pand	("mm7","mm2");
		&paddb	("mm1","mm1");		&paddb	("mm5","mm5");
		&pxor	("mm1","mm3");		&pxor	("mm5","mm7");	# tp8
		&pxor	("mm0","mm1");		&pxor	("mm4","mm5");	# ^= tp8
		&movq	("mm3","mm1");		&movq	("mm7","mm5");
		&pshufw	("mm2","mm1",0xb1);	&pshufw	("mm6","mm5",0xb1);
		&pxor	("mm0","mm2");		&pxor	("mm4","mm6");	# ^= ROTATE(tp8,16)
		&pslld	("mm1",8);		&pslld	("mm5",8);
		&psrld	("mm3",8);		&psrld	("mm7",8);
		&movq	("mm2",&QWP(0,$key));	&movq	("mm6",&QWP(8,$key));
		&pxor	("mm0","mm1");		&pxor	("mm4","mm5");	# ^= tp8<<8
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# ^= tp8>>8
		&mov	($s0,&DWP(0-128,$tbl));
		&pslld	("mm1",16);		&pslld	("mm5",16);
		&mov	($s1,&DWP(64-128,$tbl));
		&psrld	("mm3",16);		&psrld	("mm7",16);
		&mov	($s2,&DWP(128-128,$tbl));
		&pxor	("mm0","mm1");		&pxor	("mm4","mm5");	# ^= tp8<<24
		&mov	($s3,&DWP(192-128,$tbl));
		&pxor	("mm0","mm3");		&pxor	("mm4","mm7");	# ^= tp8>>24

		&pxor	("mm0","mm2");		&pxor	("mm4","mm6");
	&jmp	(&label("loop"));

	&set_label("out",16);
	&pxor	("mm0",&QWP(0,$key));
	&pxor	("mm4",&QWP(8,$key));

	&ret	();
&function_end_B("_sse_AES_decrypt_compact");
					}

######################################################################
# Vanilla block function.
######################################################################

sub decstep()
{ my ($i,$td,@s) = @_;
  my $tmp = $key;
  my $out = $i==3?$s[0]:$acc;

	# no instructions are reordered, as performance appears
	# optimal... or rather that all attempts to reorder didn't
	# result in better performance [which by the way is not a
	# bit lower than ecryption].
	if($i==3)   {	&mov	($key,$__key);			}
	else        {	&mov	($out,$s[0]);			}
			&and	($out,0xFF);
			&mov	($out,&DWP(0,$td,$out,8));

	if ($i==3)  {	$tmp=$s[1];				}
			&movz	($tmp,&HB($s[1]));
			&xor	($out,&DWP(3,$td,$tmp,8));

	if ($i==3)  {	$tmp=$s[2]; &mov ($s[1],$acc);		}
	else        {	&mov	($tmp,$s[2]);			}
			&shr	($tmp,16);
			&and	($tmp,0xFF);
			&xor	($out,&DWP(2,$td,$tmp,8));

	if ($i==3)  {	$tmp=$s[3]; &mov ($s[2],$__s1);		}
	else        {	&mov	($tmp,$s[3]);			}
			&shr	($tmp,24);
			&xor	($out,&DWP(1,$td,$tmp,8));
	if ($i<2)   {	&mov	(&DWP(4+4*$i,"esp"),$out);	}
	if ($i==3)  {	&mov	($s[3],$__s0);			}
			&comment();
}

sub declast()
{ my ($i,$td,@s)=@_;
  my $tmp = $key;
  my $out = $i==3?$s[0]:$acc;

	if($i==0)   {	&lea	($td,&DWP(2048+128,$td));
			&mov	($tmp,&DWP(0-128,$td));
			&mov	($acc,&DWP(32-128,$td));
			&mov	($tmp,&DWP(64-128,$td));
			&mov	($acc,&DWP(96-128,$td));
			&mov	($tmp,&DWP(128-128,$td));
			&mov	($acc,&DWP(160-128,$td));
			&mov	($tmp,&DWP(192-128,$td));
			&mov	($acc,&DWP(224-128,$td));
			&lea	($td,&DWP(-128,$td));		}
	if($i==3)   {	&mov	($key,$__key);			}
	else        {	&mov	($out,$s[0]);			}
			&and	($out,0xFF);
			&movz	($out,&BP(0,$td,$out,1));

	if ($i==3)  {	$tmp=$s[1];				}
			&movz	($tmp,&HB($s[1]));
			&movz	($tmp,&BP(0,$td,$tmp,1));
			&shl	($tmp,8);
			&xor	($out,$tmp);

	if ($i==3)  {	$tmp=$s[2]; &mov ($s[1],$acc);		}
	else        {	mov	($tmp,$s[2]);			}
			&shr	($tmp,16);
			&and	($tmp,0xFF);
			&movz	($tmp,&BP(0,$td,$tmp,1));
			&shl	($tmp,16);
			&xor	($out,$tmp);

	if ($i==3)  {	$tmp=$s[3]; &mov ($s[2],$__s1);		}
	else        {	&mov	($tmp,$s[3]);			}
			&shr	($tmp,24);
			&movz	($tmp,&BP(0,$td,$tmp,1));
			&shl	($tmp,24);
			&xor	($out,$tmp);
	if ($i<2)   {	&mov	(&DWP(4+4*$i,"esp"),$out);	}
	if ($i==3)  {	&mov	($s[3],$__s0);
			&lea	($td,&DWP(-2048,$td));		}
}

&function_begin_B("_x86_AES_decrypt");
	# note that caller is expected to allocate stack frame for me!
	&mov	($__key,$key);			# save key

	&xor	($s0,&DWP(0,$key));		# xor with key
	&xor	($s1,&DWP(4,$key));
	&xor	($s2,&DWP(8,$key));
	&xor	($s3,&DWP(12,$key));

	&mov	($acc,&DWP(240,$key));		# load key->rounds

	if ($small_footprint) {
	    &lea	($acc,&DWP(-2,$acc,$acc));
	    &lea	($acc,&DWP(0,$key,$acc,8));
	    &mov	($__end,$acc);		# end of key schedule
	    &set_label("loop",16);
		&decstep(0,$tbl,$s0,$s3,$s2,$s1);
		&decstep(1,$tbl,$s1,$s0,$s3,$s2);
		&decstep(2,$tbl,$s2,$s1,$s0,$s3);
		&decstep(3,$tbl,$s3,$s2,$s1,$s0);
		&add	($key,16);		# advance rd_key
		&xor	($s0,&DWP(0,$key));
		&xor	($s1,&DWP(4,$key));
		&xor	($s2,&DWP(8,$key));
		&xor	($s3,&DWP(12,$key));
	    &cmp	($key,$__end);
	    &mov	($__key,$key);
	    &jb		(&label("loop"));
	}
	else {
	    &cmp	($acc,10);
	    &jle	(&label("10rounds"));
	    &cmp	($acc,12);
	    &jle	(&label("12rounds"));

	&set_label("14rounds",4);
	    for ($i=1;$i<3;$i++) {
		&decstep(0,$tbl,$s0,$s3,$s2,$s1);
		&decstep(1,$tbl,$s1,$s0,$s3,$s2);
		&decstep(2,$tbl,$s2,$s1,$s0,$s3);
		&decstep(3,$tbl,$s3,$s2,$s1,$s0);
		&xor	($s0,&DWP(16*$i+0,$key));
		&xor	($s1,&DWP(16*$i+4,$key));
		&xor	($s2,&DWP(16*$i+8,$key));
		&xor	($s3,&DWP(16*$i+12,$key));
	    }
	    &add	($key,32);
	    &mov	($__key,$key);		# advance rd_key
	&set_label("12rounds",4);
	    for ($i=1;$i<3;$i++) {
		&decstep(0,$tbl,$s0,$s3,$s2,$s1);
		&decstep(1,$tbl,$s1,$s0,$s3,$s2);
		&decstep(2,$tbl,$s2,$s1,$s0,$s3);
		&decstep(3,$tbl,$s3,$s2,$s1,$s0);
		&xor	($s0,&DWP(16*$i+0,$key));
		&xor	($s1,&DWP(16*$i+4,$key));
		&xor	($s2,&DWP(16*$i+8,$key));
		&xor	($s3,&DWP(16*$i+12,$key));
	    }
	    &add	($key,32);
	    &mov	($__key,$key);		# advance rd_key
	&set_label("10rounds",4);
	    for ($i=1;$i<10;$i++) {
		&decstep(0,$tbl,$s0,$s3,$s2,$s1);
		&decstep(1,$tbl,$s1,$s0,$s3,$s2);
		&decstep(2,$tbl,$s2,$s1,$s0,$s3);
		&decstep(3,$tbl,$s3,$s2,$s1,$s0);
		&xor	($s0,&DWP(16*$i+0,$key));
		&xor	($s1,&DWP(16*$i+4,$key));
		&xor	($s2,&DWP(16*$i+8,$key));
		&xor	($s3,&DWP(16*$i+12,$key));
	    }
	}

	&declast(0,$tbl,$s0,$s3,$s2,$s1);
	&declast(1,$tbl,$s1,$s0,$s3,$s2);
	&declast(2,$tbl,$s2,$s1,$s0,$s3);
	&declast(3,$tbl,$s3,$s2,$s1,$s0);

	&add	($key,$small_footprint?16:160);
	&xor	($s0,&DWP(0,$key));
	&xor	($s1,&DWP(4,$key));
	&xor	($s2,&DWP(8,$key));
	&xor	($s3,&DWP(12,$key));

	&ret	();

&set_label("AES_Td",64);	# Yes! I keep it in the code segment!
	&_data_word(0x50a7f451, 0x5365417e, 0xc3a4171a, 0x965e273a);
	&_data_word(0xcb6bab3b, 0xf1459d1f, 0xab58faac, 0x9303e34b);
	&_data_word(0x55fa3020, 0xf66d76ad, 0x9176cc88, 0x254c02f5);
	&_data_word(0xfcd7e54f, 0xd7cb2ac5, 0x80443526, 0x8fa362b5);
	&_data_word(0x495ab1de, 0x671bba25, 0x980eea45, 0xe1c0fe5d);
	&_data_word(0x02752fc3, 0x12f04c81, 0xa397468d, 0xc6f9d36b);
	&_data_word(0xe75f8f03, 0x959c9215, 0xeb7a6dbf, 0xda595295);
	&_data_word(0x2d83bed4, 0xd3217458, 0x2969e049, 0x44c8c98e);
	&_data_word(0x6a89c275, 0x78798ef4, 0x6b3e5899, 0xdd71b927);
	&_data_word(0xb64fe1be, 0x17ad88f0, 0x66ac20c9, 0xb43ace7d);
	&_data_word(0x184adf63, 0x82311ae5, 0x60335197, 0x457f5362);
	&_data_word(0xe07764b1, 0x84ae6bbb, 0x1ca081fe, 0x942b08f9);
	&_data_word(0x58684870, 0x19fd458f, 0x876cde94, 0xb7f87b52);
	&_data_word(0x23d373ab, 0xe2024b72, 0x578f1fe3, 0x2aab5566);
	&_data_word(0x0728ebb2, 0x03c2b52f, 0x9a7bc586, 0xa50837d3);
	&_data_word(0xf2872830, 0xb2a5bf23, 0xba6a0302, 0x5c8216ed);
	&_data_word(0x2b1ccf8a, 0x92b479a7, 0xf0f207f3, 0xa1e2694e);
	&_data_word(0xcdf4da65, 0xd5be0506, 0x1f6234d1, 0x8afea6c4);
	&_data_word(0x9d532e34, 0xa055f3a2, 0x32e18a05, 0x75ebf6a4);
	&_data_word(0x39ec830b, 0xaaef6040, 0x069f715e, 0x51106ebd);
	&_data_word(0xf98a213e, 0x3d06dd96, 0xae053edd, 0x46bde64d);
	&_data_word(0xb58d5491, 0x055dc471, 0x6fd40604, 0xff155060);
	&_data_word(0x24fb9819, 0x97e9bdd6, 0xcc434089, 0x779ed967);
	&_data_word(0xbd42e8b0, 0x888b8907, 0x385b19e7, 0xdbeec879);
	&_data_word(0x470a7ca1, 0xe90f427c, 0xc91e84f8, 0x00000000);
	&_data_word(0x83868009, 0x48ed2b32, 0xac70111e, 0x4e725a6c);
	&_data_word(0xfbff0efd, 0x5638850f, 0x1ed5ae3d, 0x27392d36);
	&_data_word(0x64d90f0a, 0x21a65c68, 0xd1545b9b, 0x3a2e3624);
	&_data_word(0xb1670a0c, 0x0fe75793, 0xd296eeb4, 0x9e919b1b);
	&_data_word(0x4fc5c080, 0xa220dc61, 0x694b775a, 0x161a121c);
	&_data_word(0x0aba93e2, 0xe52aa0c0, 0x43e0223c, 0x1d171b12);
	&_data_word(0x0b0d090e, 0xadc78bf2, 0xb9a8b62d, 0xc8a91e14);
	&_data_word(0x8519f157, 0x4c0775af, 0xbbdd99ee, 0xfd607fa3);
	&_data_word(0x9f2601f7, 0xbcf5725c, 0xc53b6644, 0x347efb5b);
	&_data_word(0x7629438b, 0xdcc623cb, 0x68fcedb6, 0x63f1e4b8);
	&_data_word(0xcadc31d7, 0x10856342, 0x40229713, 0x2011c684);
	&_data_word(0x7d244a85, 0xf83dbbd2, 0x1132f9ae, 0x6da129c7);
	&_data_word(0x4b2f9e1d, 0xf330b2dc, 0xec52860d, 0xd0e3c177);
	&_data_word(0x6c16b32b, 0x99b970a9, 0xfa489411, 0x2264e947);
	&_data_word(0xc48cfca8, 0x1a3ff0a0, 0xd82c7d56, 0xef903322);
	&_data_word(0xc74e4987, 0xc1d138d9, 0xfea2ca8c, 0x360bd498);
	&_data_word(0xcf81f5a6, 0x28de7aa5, 0x268eb7da, 0xa4bfad3f);
	&_data_word(0xe49d3a2c, 0x0d927850, 0x9bcc5f6a, 0x62467e54);
	&_data_word(0xc2138df6, 0xe8b8d890, 0x5ef7392e, 0xf5afc382);
	&_data_word(0xbe805d9f, 0x7c93d069, 0xa92dd56f, 0xb31225cf);
	&_data_word(0x3b99acc8, 0xa77d1810, 0x6e639ce8, 0x7bbb3bdb);
	&_data_word(0x097826cd, 0xf418596e, 0x01b79aec, 0xa89a4f83);
	&_data_word(0x656e95e6, 0x7ee6ffaa, 0x08cfbc21, 0xe6e815ef);
	&_data_word(0xd99be7ba, 0xce366f4a, 0xd4099fea, 0xd67cb029);
	&_data_word(0xafb2a431, 0x31233f2a, 0x3094a5c6, 0xc066a235);
	&_data_word(0x37bc4e74, 0xa6ca82fc, 0xb0d090e0, 0x15d8a733);
	&_data_word(0x4a9804f1, 0xf7daec41, 0x0e50cd7f, 0x2ff69117);
	&_data_word(0x8dd64d76, 0x4db0ef43, 0x544daacc, 0xdf0496e4);
	&_data_word(0xe3b5d19e, 0x1b886a4c, 0xb81f2cc1, 0x7f516546);
	&_data_word(0x04ea5e9d, 0x5d358c01, 0x737487fa, 0x2e410bfb);
	&_data_word(0x5a1d67b3, 0x52d2db92, 0x335610e9, 0x1347d66d);
	&_data_word(0x8c61d79a, 0x7a0ca137, 0x8e14f859, 0x893c13eb);
	&_data_word(0xee27a9ce, 0x35c961b7, 0xede51ce1, 0x3cb1477a);
	&_data_word(0x59dfd29c, 0x3f73f255, 0x79ce1418, 0xbf37c773);
	&_data_word(0xeacdf753, 0x5baafd5f, 0x146f3ddf, 0x86db4478);
	&_data_word(0x81f3afca, 0x3ec468b9, 0x2c342438, 0x5f40a3c2);
	&_data_word(0x72c31d16, 0x0c25e2bc, 0x8b493c28, 0x41950dff);
	&_data_word(0x7101a839, 0xdeb30c08, 0x9ce4b4d8, 0x90c15664);
	&_data_word(0x6184cb7b, 0x70b632d5, 0x745c6c48, 0x4257b8d0);

#Td4:	# four copies of Td4 to choose from to avoid L1 aliasing
	&data_byte(0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38);
	&data_byte(0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb);
	&data_byte(0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87);
	&data_byte(0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb);
	&data_byte(0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d);
	&data_byte(0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e);
	&data_byte(0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2);
	&data_byte(0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25);
	&data_byte(0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16);
	&data_byte(0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92);
	&data_byte(0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda);
	&data_byte(0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84);
	&data_byte(0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a);
	&data_byte(0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06);
	&data_byte(0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02);
	&data_byte(0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b);
	&data_byte(0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea);
	&data_byte(0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73);
	&data_byte(0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85);
	&data_byte(0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e);
	&data_byte(0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89);
	&data_byte(0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b);
	&data_byte(0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20);
	&data_byte(0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4);
	&data_byte(0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31);
	&data_byte(0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f);
	&data_byte(0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d);
	&data_byte(0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef);
	&data_byte(0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0);
	&data_byte(0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61);
	&data_byte(0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26);
	&data_byte(0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d);

	&data_byte(0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38);
	&data_byte(0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb);
	&data_byte(0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87);
	&data_byte(0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb);
	&data_byte(0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d);
	&data_byte(0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e);
	&data_byte(0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2);
	&data_byte(0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25);
	&data_byte(0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16);
	&data_byte(0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92);
	&data_byte(0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda);
	&data_byte(0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84);
	&data_byte(0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a);
	&data_byte(0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06);
	&data_byte(0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02);
	&data_byte(0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b);
	&data_byte(0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea);
	&data_byte(0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73);
	&data_byte(0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85);
	&data_byte(0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e);
	&data_byte(0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89);
	&data_byte(0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b);
	&data_byte(0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20);
	&data_byte(0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4);
	&data_byte(0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31);
	&data_byte(0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f);
	&data_byte(0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d);
	&data_byte(0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef);
	&data_byte(0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0);
	&data_byte(0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61);
	&data_byte(0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26);
	&data_byte(0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d);

	&data_byte(0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38);
	&data_byte(0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb);
	&data_byte(0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87);
	&data_byte(0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb);
	&data_byte(0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d);
	&data_byte(0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e);
	&data_byte(0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2);
	&data_byte(0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25);
	&data_byte(0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16);
	&data_byte(0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92);
	&data_byte(0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda);
	&data_byte(0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84);
	&data_byte(0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a);
	&data_byte(0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06);
	&data_byte(0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02);
	&data_byte(0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b);
	&data_byte(0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea);
	&data_byte(0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73);
	&data_byte(0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85);
	&data_byte(0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e);
	&data_byte(0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89);
	&data_byte(0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b);
	&data_byte(0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20);
	&data_byte(0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4);
	&data_byte(0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31);
	&data_byte(0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f);
	&data_byte(0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d);
	&data_byte(0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef);
	&data_byte(0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0);
	&data_byte(0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61);
	&data_byte(0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26);
	&data_byte(0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d);

	&data_byte(0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38);
	&data_byte(0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb);
	&data_byte(0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87);
	&data_byte(0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb);
	&data_byte(0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d);
	&data_byte(0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e);
	&data_byte(0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2);
	&data_byte(0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25);
	&data_byte(0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16);
	&data_byte(0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92);
	&data_byte(0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda);
	&data_byte(0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84);
	&data_byte(0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a);
	&data_byte(0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06);
	&data_byte(0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02);
	&data_byte(0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b);
	&data_byte(0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea);
	&data_byte(0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73);
	&data_byte(0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85);
	&data_byte(0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e);
	&data_byte(0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89);
	&data_byte(0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b);
	&data_byte(0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20);
	&data_byte(0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4);
	&data_byte(0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31);
	&data_byte(0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f);
	&data_byte(0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d);
	&data_byte(0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef);
	&data_byte(0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0);
	&data_byte(0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61);
	&data_byte(0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26);
	&data_byte(0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d);
&function_end_B("_x86_AES_decrypt");

# void AES_decrypt (const void *inp,void *out,const AES_KEY *key);
&function_begin("AES_decrypt");
	&mov	($acc,&wparam(0));		# load inp
	&mov	($key,&wparam(2));		# load key

	&mov	($s0,"esp");
	&sub	("esp",36);
	&and	("esp",-64);			# align to cache-line

	# place stack frame just "above" the key schedule
	&lea	($s1,&DWP(-64-63,$key));
	&sub	($s1,"esp");
	&neg	($s1);
	&and	($s1,0x3C0);	# modulo 1024, but aligned to cache-line
	&sub	("esp",$s1);
	&add	("esp",4);	# 4 is reserved for caller's return address
	&mov	($_esp,$s0);	# save stack pointer

	&call   (&label("pic_point"));          # make it PIC!
	&set_label("pic_point");
	&blindpop($tbl);
	&picmeup($s0,"OPENSSL_ia32cap_P",$tbl,&label("pic_point")) if(!$x86only);
	&lea    ($tbl,&DWP(&label("AES_Td")."-".&label("pic_point"),$tbl));

	# pick Td4 copy which can't "overlap" with stack frame or key schedule
	&lea	($s1,&DWP(768-4,"esp"));
	&sub	($s1,$tbl);
	&and	($s1,0x300);
	&lea	($tbl,&DWP(2048+128,$tbl,$s1));

					if (!$x86only) {
	&bt	(&DWP(0,$s0),25);	# check for SSE bit
	&jnc	(&label("x86"));

	&movq	("mm0",&QWP(0,$acc));
	&movq	("mm4",&QWP(8,$acc));
	&call	("_sse_AES_decrypt_compact");
	&mov	("esp",$_esp);			# restore stack pointer
	&mov	($acc,&wparam(1));		# load out
	&movq	(&QWP(0,$acc),"mm0");		# write output data
	&movq	(&QWP(8,$acc),"mm4");
	&emms	();
	&function_end_A();
					}
	&set_label("x86",16);
	&mov	($_tbl,$tbl);
	&mov	($s0,&DWP(0,$acc));		# load input data
	&mov	($s1,&DWP(4,$acc));
	&mov	($s2,&DWP(8,$acc));
	&mov	($s3,&DWP(12,$acc));
	&call	("_x86_AES_decrypt_compact");
	&mov	("esp",$_esp);			# restore stack pointer
	&mov	($acc,&wparam(1));		# load out
	&mov	(&DWP(0,$acc),$s0);		# write output data
	&mov	(&DWP(4,$acc),$s1);
	&mov	(&DWP(8,$acc),$s2);
	&mov	(&DWP(12,$acc),$s3);
&function_end("AES_decrypt");

# void AES_cbc_encrypt (const void char *inp, unsigned char *out,
#			size_t length, const AES_KEY *key,
#			unsigned char *ivp,const int enc);
{
# stack frame layout
#             -4(%esp)		# return address	 0(%esp)
#              0(%esp)		# s0 backing store	 4(%esp)	
#              4(%esp)		# s1 backing store	 8(%esp)
#              8(%esp)		# s2 backing store	12(%esp)
#             12(%esp)		# s3 backing store	16(%esp)
#             16(%esp)		# key backup		20(%esp)
#             20(%esp)		# end of key schedule	24(%esp)
#             24(%esp)		# %ebp backup		28(%esp)
#             28(%esp)		# %esp backup
my $_inp=&DWP(32,"esp");	# copy of wparam(0)
my $_out=&DWP(36,"esp");	# copy of wparam(1)
my $_len=&DWP(40,"esp");	# copy of wparam(2)
my $_key=&DWP(44,"esp");	# copy of wparam(3)
my $_ivp=&DWP(48,"esp");	# copy of wparam(4)
my $_tmp=&DWP(52,"esp");	# volatile variable
#
my $ivec=&DWP(60,"esp");	# ivec[16]
my $aes_key=&DWP(76,"esp");	# copy of aes_key
my $mark=&DWP(76+240,"esp");	# copy of aes_key->rounds

&function_begin("AES_cbc_encrypt");
	&mov	($s2 eq "ecx"? $s2 : "",&wparam(2));	# load len
	&cmp	($s2,0);
	&je	(&label("drop_out"));

	&call   (&label("pic_point"));		# make it PIC!
	&set_label("pic_point");
	&blindpop($tbl);
	&picmeup($s0,"OPENSSL_ia32cap_P",$tbl,&label("pic_point")) if(!$x86only);

	&cmp	(&wparam(5),0);
	&lea    ($tbl,&DWP(&label("AES_Te")."-".&label("pic_point"),$tbl));
	&jne	(&label("picked_te"));
	&lea	($tbl,&DWP(&label("AES_Td")."-".&label("AES_Te"),$tbl));
	&set_label("picked_te");

	# one can argue if this is required
	&pushf	();
	&cld	();

	&cmp	($s2,$speed_limit);
	&jb	(&label("slow_way"));
	&test	($s2,15);
	&jnz	(&label("slow_way"));
					if (!$x86only) {
	&bt	(&DWP(0,$s0),28);	# check for hyper-threading bit
	&jc	(&label("slow_way"));
					}
	# pre-allocate aligned stack frame...
	&lea	($acc,&DWP(-80-244,"esp"));
	&and	($acc,-64);

	# ... and make sure it doesn't alias with $tbl modulo 4096
	&mov	($s0,$tbl);
	&lea	($s1,&DWP(2048+256,$tbl));
	&mov	($s3,$acc);
	&and	($s0,0xfff);		# s = %ebp&0xfff
	&and	($s1,0xfff);		# e = (%ebp+2048+256)&0xfff
	&and	($s3,0xfff);		# p = %esp&0xfff

	&cmp	($s3,$s1);		# if (p>=e) %esp =- (p-e);
	&jb	(&label("tbl_break_out"));
	&sub	($s3,$s1);
	&sub	($acc,$s3);
	&jmp	(&label("tbl_ok"));
	&set_label("tbl_break_out",4);	# else %esp -= (p-s)&0xfff + framesz;
	&sub	($s3,$s0);
	&and	($s3,0xfff);
	&add	($s3,384);
	&sub	($acc,$s3);
	&set_label("tbl_ok",4);

	&lea	($s3,&wparam(0));	# obtain pointer to parameter block
	&exch	("esp",$acc);		# allocate stack frame
	&add	("esp",4);		# reserve for return address!
	&mov	($_tbl,$tbl);		# save %ebp
	&mov	($_esp,$acc);		# save %esp

	&mov	($s0,&DWP(0,$s3));	# load inp
	&mov	($s1,&DWP(4,$s3));	# load out
	#&mov	($s2,&DWP(8,$s3));	# load len
	&mov	($key,&DWP(12,$s3));	# load key
	&mov	($acc,&DWP(16,$s3));	# load ivp
	&mov	($s3,&DWP(20,$s3));	# load enc flag

	&mov	($_inp,$s0);		# save copy of inp
	&mov	($_out,$s1);		# save copy of out
	&mov	($_len,$s2);		# save copy of len
	&mov	($_key,$key);		# save copy of key
	&mov	($_ivp,$acc);		# save copy of ivp

	&mov	($mark,0);		# copy of aes_key->rounds = 0;
	# do we copy key schedule to stack?
	&mov	($s1 eq "ebx" ? $s1 : "",$key);
	&mov	($s2 eq "ecx" ? $s2 : "",244/4);
	&sub	($s1,$tbl);
	&mov	("esi",$key);
	&and	($s1,0xfff);
	&lea	("edi",$aes_key);
	&cmp	($s1,2048+256);
	&jb	(&label("do_copy"));
	&cmp	($s1,4096-244);
	&jb	(&label("skip_copy"));
	&set_label("do_copy",4);
		&mov	($_key,"edi");
		&data_word(0xA5F3F689);	# rep movsd
	&set_label("skip_copy");

	&mov	($key,16);
	&set_label("prefetch_tbl",4);
		&mov	($s0,&DWP(0,$tbl));
		&mov	($s1,&DWP(32,$tbl));
		&mov	($s2,&DWP(64,$tbl));
		&mov	($acc,&DWP(96,$tbl));
		&lea	($tbl,&DWP(128,$tbl));
		&sub	($key,1);
	&jnz	(&label("prefetch_tbl"));
	&sub	($tbl,2048);

	&mov	($acc,$_inp);
	&mov	($key,$_ivp);

	&cmp	($s3,0);
	&je	(&label("fast_decrypt"));

#----------------------------- ENCRYPT -----------------------------#
	&mov	($s0,&DWP(0,$key));		# load iv
	&mov	($s1,&DWP(4,$key));

	&set_label("fast_enc_loop",16);
		&mov	($s2,&DWP(8,$key));
		&mov	($s3,&DWP(12,$key));

		&xor	($s0,&DWP(0,$acc));	# xor input data
		&xor	($s1,&DWP(4,$acc));
		&xor	($s2,&DWP(8,$acc));
		&xor	($s3,&DWP(12,$acc));

		&mov	($key,$_key);		# load key
		&call	("_x86_AES_encrypt");

		&mov	($acc,$_inp);		# load inp
		&mov	($key,$_out);		# load out

		&mov	(&DWP(0,$key),$s0);	# save output data
		&mov	(&DWP(4,$key),$s1);
		&mov	(&DWP(8,$key),$s2);
		&mov	(&DWP(12,$key),$s3);

		&lea	($acc,&DWP(16,$acc));	# advance inp
		&mov	($s2,$_len);		# load len
		&mov	($_inp,$acc);		# save inp
		&lea	($s3,&DWP(16,$key));	# advance out
		&mov	($_out,$s3);		# save out
		&sub	($s2,16);		# decrease len
		&mov	($_len,$s2);		# save len
	&jnz	(&label("fast_enc_loop"));
	&mov	($acc,$_ivp);		# load ivp
	&mov	($s2,&DWP(8,$key));	# restore last 2 dwords
	&mov	($s3,&DWP(12,$key));
	&mov	(&DWP(0,$acc),$s0);	# save ivec
	&mov	(&DWP(4,$acc),$s1);
	&mov	(&DWP(8,$acc),$s2);
	&mov	(&DWP(12,$acc),$s3);

	&cmp	($mark,0);		# was the key schedule copied?
	&mov	("edi",$_key);
	&je	(&label("skip_ezero"));
	# zero copy of key schedule
	&mov	("ecx",240/4);
	&xor	("eax","eax");
	&align	(4);
	&data_word(0xABF3F689);	# rep stosd
	&set_label("skip_ezero")
	&mov	("esp",$_esp);
	&popf	();
    &set_label("drop_out");
	&function_end_A();
	&pushf	();			# kludge, never executed

#----------------------------- DECRYPT -----------------------------#
&set_label("fast_decrypt",16);

	&cmp	($acc,$_out);
	&je	(&label("fast_dec_in_place"));	# in-place processing...

	&mov	($_tmp,$key);

	&align	(4);
	&set_label("fast_dec_loop",16);
		&mov	($s0,&DWP(0,$acc));	# read input
		&mov	($s1,&DWP(4,$acc));
		&mov	($s2,&DWP(8,$acc));
		&mov	($s3,&DWP(12,$acc));

		&mov	($key,$_key);		# load key
		&call	("_x86_AES_decrypt");

		&mov	($key,$_tmp);		# load ivp
		&mov	($acc,$_len);		# load len
		&xor	($s0,&DWP(0,$key));	# xor iv
		&xor	($s1,&DWP(4,$key));
		&xor	($s2,&DWP(8,$key));
		&xor	($s3,&DWP(12,$key));

		&mov	($key,$_out);		# load out
		&mov	($acc,$_inp);		# load inp

		&mov	(&DWP(0,$key),$s0);	# write output
		&mov	(&DWP(4,$key),$s1);
		&mov	(&DWP(8,$key),$s2);
		&mov	(&DWP(12,$key),$s3);

		&mov	($s2,$_len);		# load len
		&mov	($_tmp,$acc);		# save ivp
		&lea	($acc,&DWP(16,$acc));	# advance inp
		&mov	($_inp,$acc);		# save inp
		&lea	($key,&DWP(16,$key));	# advance out
		&mov	($_out,$key);		# save out
		&sub	($s2,16);		# decrease len
		&mov	($_len,$s2);		# save len
	&jnz	(&label("fast_dec_loop"));
	&mov	($key,$_tmp);		# load temp ivp
	&mov	($acc,$_ivp);		# load user ivp
	&mov	($s0,&DWP(0,$key));	# load iv
	&mov	($s1,&DWP(4,$key));
	&mov	($s2,&DWP(8,$key));
	&mov	($s3,&DWP(12,$key));
	&mov	(&DWP(0,$acc),$s0);	# copy back to user
	&mov	(&DWP(4,$acc),$s1);
	&mov	(&DWP(8,$acc),$s2);
	&mov	(&DWP(12,$acc),$s3);
	&jmp	(&label("fast_dec_out"));

    &set_label("fast_dec_in_place",16);
	&set_label("fast_dec_in_place_loop");
		&mov	($s0,&DWP(0,$acc));	# read input
		&mov	($s1,&DWP(4,$acc));
		&mov	($s2,&DWP(8,$acc));
		&mov	($s3,&DWP(12,$acc));

		&lea	($key,$ivec);
		&mov	(&DWP(0,$key),$s0);	# copy to temp
		&mov	(&DWP(4,$key),$s1);
		&mov	(&DWP(8,$key),$s2);
		&mov	(&DWP(12,$key),$s3);

		&mov	($key,$_key);		# load key
		&call	("_x86_AES_decrypt");

		&mov	($key,$_ivp);		# load ivp
		&mov	($acc,$_out);		# load out
		&xor	($s0,&DWP(0,$key));	# xor iv
		&xor	($s1,&DWP(4,$key));
		&xor	($s2,&DWP(8,$key));
		&xor	($s3,&DWP(12,$key));

		&mov	(&DWP(0,$acc),$s0);	# write output
		&mov	(&DWP(4,$acc),$s1);
		&mov	(&DWP(8,$acc),$s2);
		&mov	(&DWP(12,$acc),$s3);

		&lea	($acc,&DWP(16,$acc));	# advance out
		&mov	($_out,$acc);		# save out

		&lea	($acc,$ivec);
		&mov	($s0,&DWP(0,$acc));	# read temp
		&mov	($s1,&DWP(4,$acc));
		&mov	($s2,&DWP(8,$acc));
		&mov	($s3,&DWP(12,$acc));

		&mov	(&DWP(0,$key),$s0);	# copy iv
		&mov	(&DWP(4,$key),$s1);
		&mov	(&DWP(8,$key),$s2);
		&mov	(&DWP(12,$key),$s3);

		&mov	($acc,$_inp);		# load inp
		&mov	($s2,$_len);		# load len
		&lea	($acc,&DWP(16,$acc));	# advance inp
		&mov	($_inp,$acc);		# save inp
		&sub	($s2,16);		# decrease len
		&mov	($_len,$s2);		# save len
	&jnz	(&label("fast_dec_in_place_loop"));

    &set_label("fast_dec_out",4);
	&cmp	($mark,0);		# was the key schedule copied?
	&mov	("edi",$_key);
	&je	(&label("skip_dzero"));
	# zero copy of key schedule
	&mov	("ecx",240/4);
	&xor	("eax","eax");
	&align	(4);
	&data_word(0xABF3F689);	# rep stosd
	&set_label("skip_dzero")
	&mov	("esp",$_esp);
	&popf	();
	&function_end_A();
	&pushf	();			# kludge, never executed

#--------------------------- SLOW ROUTINE ---------------------------#
&set_label("slow_way",16);

	&mov	($s0,&DWP(0,$s0)) if (!$x86only);# load OPENSSL_ia32cap
	&mov	($key,&wparam(3));	# load key

	# pre-allocate aligned stack frame...
	&lea	($acc,&DWP(-80,"esp"));
	&and	($acc,-64);

	# ... and make sure it doesn't alias with $key modulo 1024
	&lea	($s1,&DWP(-80-63,$key));
	&sub	($s1,$acc);
	&neg	($s1);
	&and	($s1,0x3C0);	# modulo 1024, but aligned to cache-line
	&sub	($acc,$s1);

	# pick S-box copy which can't overlap with stack frame or $key
	&lea	($s1,&DWP(768,$acc));
	&sub	($s1,$tbl);
	&and	($s1,0x300);
	&lea	($tbl,&DWP(2048+128,$tbl,$s1));

	&lea	($s3,&wparam(0));	# pointer to parameter block

	&exch	("esp",$acc);
	&add	("esp",4);		# reserve for return address!
	&mov	($_tbl,$tbl);		# save %ebp
	&mov	($_esp,$acc);		# save %esp
	&mov	($_tmp,$s0);		# save OPENSSL_ia32cap

	&mov	($s0,&DWP(0,$s3));	# load inp
	&mov	($s1,&DWP(4,$s3));	# load out
	#&mov	($s2,&DWP(8,$s3));	# load len
	#&mov	($key,&DWP(12,$s3));	# load key
	&mov	($acc,&DWP(16,$s3));	# load ivp
	&mov	($s3,&DWP(20,$s3));	# load enc flag

	&mov	($_inp,$s0);		# save copy of inp
	&mov	($_out,$s1);		# save copy of out
	&mov	($_len,$s2);		# save copy of len
	&mov	($_key,$key);		# save copy of key
	&mov	($_ivp,$acc);		# save copy of ivp

	&mov	($key,$acc);
	&mov	($acc,$s0);

	&cmp	($s3,0);
	&je	(&label("slow_decrypt"));

#--------------------------- SLOW ENCRYPT ---------------------------#
	&cmp	($s2,16);
	&mov	($s3,$s1);
	&jb	(&label("slow_enc_tail"));

					if (!$x86only) {
	&bt	($_tmp,25);		# check for SSE bit
	&jnc	(&label("slow_enc_x86"));

	&movq	("mm0",&QWP(0,$key));	# load iv
	&movq	("mm4",&QWP(8,$key));

	&set_label("slow_enc_loop_sse",16);
		&pxor	("mm0",&QWP(0,$acc));	# xor input data
		&pxor	("mm4",&QWP(8,$acc));

		&mov	($key,$_key);
		&call	("_sse_AES_encrypt_compact");

		&mov	($acc,$_inp);		# load inp
		&mov	($key,$_out);		# load out
		&mov	($s2,$_len);		# load len

		&movq	(&QWP(0,$key),"mm0");	# save output data
		&movq	(&QWP(8,$key),"mm4");

		&lea	($acc,&DWP(16,$acc));	# advance inp
		&mov	($_inp,$acc);		# save inp
		&lea	($s3,&DWP(16,$key));	# advance out
		&mov	($_out,$s3);		# save out
		&sub	($s2,16);		# decrease len
		&cmp	($s2,16);
		&mov	($_len,$s2);		# save len
	&jae	(&label("slow_enc_loop_sse"));
	&test	($s2,15);
	&jnz	(&label("slow_enc_tail"));
	&mov	($acc,$_ivp);		# load ivp
	&movq	(&QWP(0,$acc),"mm0");	# save ivec
	&movq	(&QWP(8,$acc),"mm4");
	&emms	();
	&mov	("esp",$_esp);
	&popf	();
	&function_end_A();
	&pushf	();			# kludge, never executed
					}
    &set_label("slow_enc_x86",16);
	&mov	($s0,&DWP(0,$key));	# load iv
	&mov	($s1,&DWP(4,$key));

	&set_label("slow_enc_loop_x86",4);
		&mov	($s2,&DWP(8,$key));
		&mov	($s3,&DWP(12,$key));

		&xor	($s0,&DWP(0,$acc));	# xor input data
		&xor	($s1,&DWP(4,$acc));
		&xor	($s2,&DWP(8,$acc));
		&xor	($s3,&DWP(12,$acc));

		&mov	($key,$_key);		# load key
		&call	("_x86_AES_encrypt_compact");

		&mov	($acc,$_inp);		# load inp
		&mov	($key,$_out);		# load out

		&mov	(&DWP(0,$key),$s0);	# save output data
		&mov	(&DWP(4,$key),$s1);
		&mov	(&DWP(8,$key),$s2);
		&mov	(&DWP(12,$key),$s3);

		&mov	($s2,$_len);		# load len
		&lea	($acc,&DWP(16,$acc));	# advance inp
		&mov	($_inp,$acc);		# save inp
		&lea	($s3,&DWP(16,$key));	# advance out
		&mov	($_out,$s3);		# save out
		&sub	($s2,16);		# decrease len
		&cmp	($s2,16);
		&mov	($_len,$s2);		# save len
	&jae	(&label("slow_enc_loop_x86"));
	&test	($s2,15);
	&jnz	(&label("slow_enc_tail"));
	&mov	($acc,$_ivp);		# load ivp
	&mov	($s2,&DWP(8,$key));	# restore last dwords
	&mov	($s3,&DWP(12,$key));
	&mov	(&DWP(0,$acc),$s0);	# save ivec
	&mov	(&DWP(4,$acc),$s1);
	&mov	(&DWP(8,$acc),$s2);
	&mov	(&DWP(12,$acc),$s3);

	&mov	("esp",$_esp);
	&popf	();
	&function_end_A();
	&pushf	();			# kludge, never executed

    &set_label("slow_enc_tail",16);
	&emms	()	if (!$x86only);
	&mov	($key eq "edi"? $key:"",$s3);	# load out to edi
	&mov	($s1,16);
	&sub	($s1,$s2);
	&cmp	($key,$acc eq "esi"? $acc:"");	# compare with inp
	&je	(&label("enc_in_place"));
	&align	(4);
	&data_word(0xA4F3F689);	# rep movsb	# copy input
	&jmp	(&label("enc_skip_in_place"));
    &set_label("enc_in_place");
	&lea	($key,&DWP(0,$key,$s2));
    &set_label("enc_skip_in_place");
	&mov	($s2,$s1);
	&xor	($s0,$s0);
	&align	(4);
	&data_word(0xAAF3F689);	# rep stosb	# zero tail

	&mov	($key,$_ivp);			# restore ivp
	&mov	($acc,$s3);			# output as input
	&mov	($s0,&DWP(0,$key));
	&mov	($s1,&DWP(4,$key));
	&mov	($_len,16);			# len=16
	&jmp	(&label("slow_enc_loop_x86"));	# one more spin...

#--------------------------- SLOW DECRYPT ---------------------------#
&set_label("slow_decrypt",16);
					if (!$x86only) {
	&bt	($_tmp,25);		# check for SSE bit
	&jnc	(&label("slow_dec_loop_x86"));

	&set_label("slow_dec_loop_sse",4);
		&movq	("mm0",&QWP(0,$acc));	# read input
		&movq	("mm4",&QWP(8,$acc));

		&mov	($key,$_key);
		&call	("_sse_AES_decrypt_compact");

		&mov	($acc,$_inp);		# load inp
		&lea	($s0,$ivec);
		&mov	($s1,$_out);		# load out
		&mov	($s2,$_len);		# load len
		&mov	($key,$_ivp);		# load ivp

		&movq	("mm1",&QWP(0,$acc));	# re-read input
		&movq	("mm5",&QWP(8,$acc));

		&pxor	("mm0",&QWP(0,$key));	# xor iv
		&pxor	("mm4",&QWP(8,$key));

		&movq	(&QWP(0,$key),"mm1");	# copy input to iv
		&movq	(&QWP(8,$key),"mm5");

		&sub	($s2,16);		# decrease len
		&jc	(&label("slow_dec_partial_sse"));

		&movq	(&QWP(0,$s1),"mm0");	# write output
		&movq	(&QWP(8,$s1),"mm4");

		&lea	($s1,&DWP(16,$s1));	# advance out
		&mov	($_out,$s1);		# save out
		&lea	($acc,&DWP(16,$acc));	# advance inp
		&mov	($_inp,$acc);		# save inp
		&mov	($_len,$s2);		# save len
	&jnz	(&label("slow_dec_loop_sse"));
	&emms	();
	&mov	("esp",$_esp);
	&popf	();
	&function_end_A();
	&pushf	();			# kludge, never executed

    &set_label("slow_dec_partial_sse",16);
	&movq	(&QWP(0,$s0),"mm0");	# save output to temp
	&movq	(&QWP(8,$s0),"mm4");
	&emms	();

	&add	($s2 eq "ecx" ? "ecx":"",16);
	&mov	("edi",$s1);		# out
	&mov	("esi",$s0);		# temp
	&align	(4);
	&data_word(0xA4F3F689);		# rep movsb # copy partial output

	&mov	("esp",$_esp);
	&popf	();
	&function_end_A();
	&pushf	();			# kludge, never executed
					}
	&set_label("slow_dec_loop_x86",16);
		&mov	($s0,&DWP(0,$acc));	# read input
		&mov	($s1,&DWP(4,$acc));
		&mov	($s2,&DWP(8,$acc));
		&mov	($s3,&DWP(12,$acc));

		&lea	($key,$ivec);
		&mov	(&DWP(0,$key),$s0);	# copy to temp
		&mov	(&DWP(4,$key),$s1);
		&mov	(&DWP(8,$key),$s2);
		&mov	(&DWP(12,$key),$s3);

		&mov	($key,$_key);		# load key
		&call	("_x86_AES_decrypt_compact");

		&mov	($key,$_ivp);		# load ivp
		&mov	($acc,$_len);		# load len
		&xor	($s0,&DWP(0,$key));	# xor iv
		&xor	($s1,&DWP(4,$key));
		&xor	($s2,&DWP(8,$key));
		&xor	($s3,&DWP(12,$key));

		&sub	($acc,16);
		&jc	(&label("slow_dec_partial_x86"));

		&mov	($_len,$acc);		# save len
		&mov	($acc,$_out);		# load out

		&mov	(&DWP(0,$acc),$s0);	# write output
		&mov	(&DWP(4,$acc),$s1);
		&mov	(&DWP(8,$acc),$s2);
		&mov	(&DWP(12,$acc),$s3);

		&lea	($acc,&DWP(16,$acc));	# advance out
		&mov	($_out,$acc);		# save out

		&lea	($acc,$ivec);
		&mov	($s0,&DWP(0,$acc));	# read temp
		&mov	($s1,&DWP(4,$acc));
		&mov	($s2,&DWP(8,$acc));
		&mov	($s3,&DWP(12,$acc));

		&mov	(&DWP(0,$key),$s0);	# copy it to iv
		&mov	(&DWP(4,$key),$s1);
		&mov	(&DWP(8,$key),$s2);
		&mov	(&DWP(12,$key),$s3);

		&mov	($acc,$_inp);		# load inp
		&lea	($acc,&DWP(16,$acc));	# advance inp
		&mov	($_inp,$acc);		# save inp
	&jnz	(&label("slow_dec_loop_x86"));
	&mov	("esp",$_esp);
	&popf	();
	&function_end_A();
	&pushf	();			# kludge, never executed

    &set_label("slow_dec_partial_x86",16);
	&lea	($acc,$ivec);
	&mov	(&DWP(0,$acc),$s0);	# save output to temp
	&mov	(&DWP(4,$acc),$s1);
	&mov	(&DWP(8,$acc),$s2);
	&mov	(&DWP(12,$acc),$s3);

	&mov	($acc,$_inp);
	&mov	($s0,&DWP(0,$acc));	# re-read input
	&mov	($s1,&DWP(4,$acc));
	&mov	($s2,&DWP(8,$acc));
	&mov	($s3,&DWP(12,$acc));

	&mov	(&DWP(0,$key),$s0);	# copy it to iv
	&mov	(&DWP(4,$key),$s1);
	&mov	(&DWP(8,$key),$s2);
	&mov	(&DWP(12,$key),$s3);

	&mov	("ecx",$_len);
	&mov	("edi",$_out);
	&lea	("esi",$ivec);
	&align	(4);
	&data_word(0xA4F3F689);		# rep movsb # copy partial output

	&mov	("esp",$_esp);
	&popf	();
&function_end("AES_cbc_encrypt");
}

#------------------------------------------------------------------#

sub enckey()
{
	&movz	("esi",&LB("edx"));		# rk[i]>>0
	&movz	("ebx",&BP(-128,$tbl,"esi",1));
	&movz	("esi",&HB("edx"));		# rk[i]>>8
	&shl	("ebx",24);
	&xor	("eax","ebx");

	&movz	("ebx",&BP(-128,$tbl,"esi",1));
	&shr	("edx",16);
	&movz	("esi",&LB("edx"));		# rk[i]>>16
	&xor	("eax","ebx");

	&movz	("ebx",&BP(-128,$tbl,"esi",1));
	&movz	("esi",&HB("edx"));		# rk[i]>>24
	&shl	("ebx",8);
	&xor	("eax","ebx");

	&movz	("ebx",&BP(-128,$tbl,"esi",1));
	&shl	("ebx",16);
	&xor	("eax","ebx");

	&xor	("eax",&DWP(1024-128,$tbl,"ecx",4));	# rcon
}

&function_begin("_x86_AES_set_encrypt_key");
	&mov	("esi",&wparam(1));		# user supplied key
	&mov	("edi",&wparam(3));		# private key schedule

	&test	("esi",-1);
	&jz	(&label("badpointer"));
	&test	("edi",-1);
	&jz	(&label("badpointer"));

	&call	(&label("pic_point"));
	&set_label("pic_point");
	&blindpop($tbl);
	&lea	($tbl,&DWP(&label("AES_Te")."-".&label("pic_point"),$tbl));
	&lea	($tbl,&DWP(2048+128,$tbl));

	# prefetch Te4
	&mov	("eax",&DWP(0-128,$tbl));
	&mov	("ebx",&DWP(32-128,$tbl));
	&mov	("ecx",&DWP(64-128,$tbl));
	&mov	("edx",&DWP(96-128,$tbl));
	&mov	("eax",&DWP(128-128,$tbl));
	&mov	("ebx",&DWP(160-128,$tbl));
	&mov	("ecx",&DWP(192-128,$tbl));
	&mov	("edx",&DWP(224-128,$tbl));

	&mov	("ecx",&wparam(2));		# number of bits in key
	&cmp	("ecx",128);
	&je	(&label("10rounds"));
	&cmp	("ecx",192);
	&je	(&label("12rounds"));
	&cmp	("ecx",256);
	&je	(&label("14rounds"));
	&mov	("eax",-2);			# invalid number of bits
	&jmp	(&label("exit"));

    &set_label("10rounds");
	&mov	("eax",&DWP(0,"esi"));		# copy first 4 dwords
	&mov	("ebx",&DWP(4,"esi"));
	&mov	("ecx",&DWP(8,"esi"));
	&mov	("edx",&DWP(12,"esi"));
	&mov	(&DWP(0,"edi"),"eax");
	&mov	(&DWP(4,"edi"),"ebx");
	&mov	(&DWP(8,"edi"),"ecx");
	&mov	(&DWP(12,"edi"),"edx");

	&xor	("ecx","ecx");
	&jmp	(&label("10shortcut"));

	&align	(4);
	&set_label("10loop");
		&mov	("eax",&DWP(0,"edi"));		# rk[0]
		&mov	("edx",&DWP(12,"edi"));		# rk[3]
	&set_label("10shortcut");
		&enckey	();

		&mov	(&DWP(16,"edi"),"eax");		# rk[4]
		&xor	("eax",&DWP(4,"edi"));
		&mov	(&DWP(20,"edi"),"eax");		# rk[5]
		&xor	("eax",&DWP(8,"edi"));
		&mov	(&DWP(24,"edi"),"eax");		# rk[6]
		&xor	("eax",&DWP(12,"edi"));
		&mov	(&DWP(28,"edi"),"eax");		# rk[7]
		&inc	("ecx");
		&add	("edi",16);
		&cmp	("ecx",10);
	&jl	(&label("10loop"));

	&mov	(&DWP(80,"edi"),10);		# setup number of rounds
	&xor	("eax","eax");
	&jmp	(&label("exit"));
		
    &set_label("12rounds");
	&mov	("eax",&DWP(0,"esi"));		# copy first 6 dwords
	&mov	("ebx",&DWP(4,"esi"));
	&mov	("ecx",&DWP(8,"esi"));
	&mov	("edx",&DWP(12,"esi"));
	&mov	(&DWP(0,"edi"),"eax");
	&mov	(&DWP(4,"edi"),"ebx");
	&mov	(&DWP(8,"edi"),"ecx");
	&mov	(&DWP(12,"edi"),"edx");
	&mov	("ecx",&DWP(16,"esi"));
	&mov	("edx",&DWP(20,"esi"));
	&mov	(&DWP(16,"edi"),"ecx");
	&mov	(&DWP(20,"edi"),"edx");

	&xor	("ecx","ecx");
	&jmp	(&label("12shortcut"));

	&align	(4);
	&set_label("12loop");
		&mov	("eax",&DWP(0,"edi"));		# rk[0]
		&mov	("edx",&DWP(20,"edi"));		# rk[5]
	&set_label("12shortcut");
		&enckey	();

		&mov	(&DWP(24,"edi"),"eax");		# rk[6]
		&xor	("eax",&DWP(4,"edi"));
		&mov	(&DWP(28,"edi"),"eax");		# rk[7]
		&xor	("eax",&DWP(8,"edi"));
		&mov	(&DWP(32,"edi"),"eax");		# rk[8]
		&xor	("eax",&DWP(12,"edi"));
		&mov	(&DWP(36,"edi"),"eax");		# rk[9]

		&cmp	("ecx",7);
		&je	(&label("12break"));
		&inc	("ecx");

		&xor	("eax",&DWP(16,"edi"));
		&mov	(&DWP(40,"edi"),"eax");		# rk[10]
		&xor	("eax",&DWP(20,"edi"));
		&mov	(&DWP(44,"edi"),"eax");		# rk[11]

		&add	("edi",24);
	&jmp	(&label("12loop"));

	&set_label("12break");
	&mov	(&DWP(72,"edi"),12);		# setup number of rounds
	&xor	("eax","eax");
	&jmp	(&label("exit"));

    &set_label("14rounds");
	&mov	("eax",&DWP(0,"esi"));		# copy first 8 dwords
	&mov	("ebx",&DWP(4,"esi"));
	&mov	("ecx",&DWP(8,"esi"));
	&mov	("edx",&DWP(12,"esi"));
	&mov	(&DWP(0,"edi"),"eax");
	&mov	(&DWP(4,"edi"),"ebx");
	&mov	(&DWP(8,"edi"),"ecx");
	&mov	(&DWP(12,"edi"),"edx");
	&mov	("eax",&DWP(16,"esi"));
	&mov	("ebx",&DWP(20,"esi"));
	&mov	("ecx",&DWP(24,"esi"));
	&mov	("edx",&DWP(28,"esi"));
	&mov	(&DWP(16,"edi"),"eax");
	&mov	(&DWP(20,"edi"),"ebx");
	&mov	(&DWP(24,"edi"),"ecx");
	&mov	(&DWP(28,"edi"),"edx");

	&xor	("ecx","ecx");
	&jmp	(&label("14shortcut"));

	&align	(4);
	&set_label("14loop");
		&mov	("edx",&DWP(28,"edi"));		# rk[7]
	&set_label("14shortcut");
		&mov	("eax",&DWP(0,"edi"));		# rk[0]

		&enckey	();

		&mov	(&DWP(32,"edi"),"eax");		# rk[8]
		&xor	("eax",&DWP(4,"edi"));
		&mov	(&DWP(36,"edi"),"eax");		# rk[9]
		&xor	("eax",&DWP(8,"edi"));
		&mov	(&DWP(40,"edi"),"eax");		# rk[10]
		&xor	("eax",&DWP(12,"edi"));
		&mov	(&DWP(44,"edi"),"eax");		# rk[11]

		&cmp	("ecx",6);
		&je	(&label("14break"));
		&inc	("ecx");

		&mov	("edx","eax");
		&mov	("eax",&DWP(16,"edi"));		# rk[4]
		&movz	("esi",&LB("edx"));		# rk[11]>>0
		&movz	("ebx",&BP(-128,$tbl,"esi",1));
		&movz	("esi",&HB("edx"));		# rk[11]>>8
		&xor	("eax","ebx");

		&movz	("ebx",&BP(-128,$tbl,"esi",1));
		&shr	("edx",16);
		&shl	("ebx",8);
		&movz	("esi",&LB("edx"));		# rk[11]>>16
		&xor	("eax","ebx");

		&movz	("ebx",&BP(-128,$tbl,"esi",1));
		&movz	("esi",&HB("edx"));		# rk[11]>>24
		&shl	("ebx",16);
		&xor	("eax","ebx");

		&movz	("ebx",&BP(-128,$tbl,"esi",1));
		&shl	("ebx",24);
		&xor	("eax","ebx");

		&mov	(&DWP(48,"edi"),"eax");		# rk[12]
		&xor	("eax",&DWP(20,"edi"));
		&mov	(&DWP(52,"edi"),"eax");		# rk[13]
		&xor	("eax",&DWP(24,"edi"));
		&mov	(&DWP(56,"edi"),"eax");		# rk[14]
		&xor	("eax",&DWP(28,"edi"));
		&mov	(&DWP(60,"edi"),"eax");		# rk[15]

		&add	("edi",32);
	&jmp	(&label("14loop"));

	&set_label("14break");
	&mov	(&DWP(48,"edi"),14);		# setup number of rounds
	&xor	("eax","eax");
	&jmp	(&label("exit"));

    &set_label("badpointer");
	&mov	("eax",-1);
    &set_label("exit");
&function_end("_x86_AES_set_encrypt_key");

# int private_AES_set_encrypt_key(const unsigned char *userKey, const int bits,
#                        AES_KEY *key)
&function_begin_B("private_AES_set_encrypt_key");
	&call	("_x86_AES_set_encrypt_key");
	&ret	();
&function_end_B("private_AES_set_encrypt_key");

sub deckey()
{ my ($i,$key,$tp1,$tp2,$tp4,$tp8) = @_;
  my $tmp = $tbl;

	&mov	($acc,$tp1);
	&and	($acc,0x80808080);
	&mov	($tmp,$acc);
	&shr	($tmp,7);
	&lea	($tp2,&DWP(0,$tp1,$tp1));
	&sub	($acc,$tmp);
	&and	($tp2,0xfefefefe);
	&and	($acc,0x1b1b1b1b);
	&xor	($acc,$tp2);
	&mov	($tp2,$acc);

	&and	($acc,0x80808080);
	&mov	($tmp,$acc);
	&shr	($tmp,7);
	&lea	($tp4,&DWP(0,$tp2,$tp2));
	&sub	($acc,$tmp);
	&and	($tp4,0xfefefefe);
	&and	($acc,0x1b1b1b1b);
	 &xor	($tp2,$tp1);	# tp2^tp1
	&xor	($acc,$tp4);
	&mov	($tp4,$acc);

	&and	($acc,0x80808080);
	&mov	($tmp,$acc);
	&shr	($tmp,7);
	&lea	($tp8,&DWP(0,$tp4,$tp4));
	 &xor	($tp4,$tp1);	# tp4^tp1
	&sub	($acc,$tmp);
	&and	($tp8,0xfefefefe);
	&and	($acc,0x1b1b1b1b);
	 &rotl	($tp1,8);	# = ROTATE(tp1,8)
	&xor	($tp8,$acc);

	&mov	($tmp,&DWP(4*($i+1),$key));	# modulo-scheduled load

	&xor	($tp1,$tp2);
	&xor	($tp2,$tp8);
	&xor	($tp1,$tp4);
	&rotl	($tp2,24);
	&xor	($tp4,$tp8);
	&xor	($tp1,$tp8);	# ^= tp8^(tp4^tp1)^(tp2^tp1)
	&rotl	($tp4,16);
	&xor	($tp1,$tp2);	# ^= ROTATE(tp8^tp2^tp1,24)
	&rotl	($tp8,8);
	&xor	($tp1,$tp4);	# ^= ROTATE(tp8^tp4^tp1,16)
	&mov	($tp2,$tmp);
	&xor	($tp1,$tp8);	# ^= ROTATE(tp8,8)

	&mov	(&DWP(4*$i,$key),$tp1);
}

# int private_AES_set_decrypt_key(const unsigned char *userKey, const int bits,
#                        AES_KEY *key)
&function_begin_B("private_AES_set_decrypt_key");
	&call	("_x86_AES_set_encrypt_key");
	&cmp	("eax",0);
	&je	(&label("proceed"));
	&ret	();

    &set_label("proceed");
	&push	("ebp");
	&push	("ebx");
	&push	("esi");
	&push	("edi");

	&mov	("esi",&wparam(2));
	&mov	("ecx",&DWP(240,"esi"));	# pull number of rounds
	&lea	("ecx",&DWP(0,"","ecx",4));
	&lea	("edi",&DWP(0,"esi","ecx",4));	# pointer to last chunk

	&set_label("invert",4);			# invert order of chunks
		&mov	("eax",&DWP(0,"esi"));
		&mov	("ebx",&DWP(4,"esi"));
		&mov	("ecx",&DWP(0,"edi"));
		&mov	("edx",&DWP(4,"edi"));
		&mov	(&DWP(0,"edi"),"eax");
		&mov	(&DWP(4,"edi"),"ebx");
		&mov	(&DWP(0,"esi"),"ecx");
		&mov	(&DWP(4,"esi"),"edx");
		&mov	("eax",&DWP(8,"esi"));
		&mov	("ebx",&DWP(12,"esi"));
		&mov	("ecx",&DWP(8,"edi"));
		&mov	("edx",&DWP(12,"edi"));
		&mov	(&DWP(8,"edi"),"eax");
		&mov	(&DWP(12,"edi"),"ebx");
		&mov	(&DWP(8,"esi"),"ecx");
		&mov	(&DWP(12,"esi"),"edx");
		&add	("esi",16);
		&sub	("edi",16);
		&cmp	("esi","edi");
	&jne	(&label("invert"));

	&mov	($key,&wparam(2));
	&mov	($acc,&DWP(240,$key));		# pull number of rounds
	&lea	($acc,&DWP(-2,$acc,$acc));
	&lea	($acc,&DWP(0,$key,$acc,8));
	&mov	(&wparam(2),$acc);

	&mov	($s0,&DWP(16,$key));		# modulo-scheduled load
	&set_label("permute",4);		# permute the key schedule
		&add	($key,16);
		&deckey	(0,$key,$s0,$s1,$s2,$s3);
		&deckey	(1,$key,$s1,$s2,$s3,$s0);
		&deckey	(2,$key,$s2,$s3,$s0,$s1);
		&deckey	(3,$key,$s3,$s0,$s1,$s2);
		&cmp	($key,&wparam(2));
	&jb	(&label("permute"));

	&xor	("eax","eax");			# return success
&function_end("private_AES_set_decrypt_key");
&asciz("AES for x86, CRYPTOGAMS by <appro\@openssl.org>");

&asm_finish();