#! /bin/sh
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

#   EXPAT TEST SCRIPT FOR W3C XML TEST SUITE

# This script can be used to exercise Expat against the
# w3c.org xml test suite, available from
# http://www.w3.org/XML/Test/xmlts20020606.zip.

# To run this script, first set XMLWF below so that xmlwf can be
# found, then set the output directory with OUTPUT.

# The script lists all test cases where Expat shows a discrepancy
# from the expected result. Test cases where only the canonical
# output differs are prefixed with "Output differs:", and a diff file
# is generated in the appropriate subdirectory under $OUTPUT.

# If there are output files provided, the script will use
# output from xmlwf and compare the desired output against it.
# However, one has to take into account that the canonical output
# produced by xmlwf conforms to an older definition of canonical XML
# and does not generate notation declarations.

MYDIR="`dirname \"$0\"`"
cd "$MYDIR"
MYDIR="`pwd`"
XMLWF="`dirname \"$MYDIR\"`/xmlwf/xmlwf"
# XMLWF=/usr/local/bin/xmlwf
TS="$MYDIR/XML-Test-Suite"
# OUTPUT must terminate with the directory separator.
OUTPUT="$TS/out/"
# OUTPUT=/home/tmp/xml-testsuite-out/


# RunXmlwfNotWF file reldir
# reldir includes trailing slash
RunXmlwfNotWF() {
  file="$1"
  reldir="$2"
  $XMLWF -p "$file" > outfile || return $?
  read outdata < outfile
  if test "$outdata" = "" ; then
      echo "Expected not well-formed: $reldir$file"
      return 1
  else
      return 0
  fi 
}

# RunXmlwfWF file reldir
# reldir includes trailing slash
RunXmlwfWF() {
  file="$1"
  reldir="$2"
  $XMLWF -p -d "$OUTPUT$reldir" "$file" > outfile || return $?
  read outdata < outfile 
  if test "$outdata" = "" ; then 
      if [ -f "out/$file" ] ; then 
          diff -u "$OUTPUT$reldir$file" "out/$file" > outfile 
          if [ -s outfile ] ; then 
              cp outfile "$OUTPUT$reldir$file.diff"
              echo "Output differs: $reldir$file"
              return 1
          fi 
      fi 
      return 0
  else 
      echo "In $reldir: $outdata"
      return 1
  fi 
}

SUCCESS=0
ERROR=0

UpdateStatus() {
  if [ "$1" -eq 0 ] ; then
    SUCCESS=`expr $SUCCESS + 1`
  else
    ERROR=`expr $ERROR + 1`
  fi
}

##########################
# well-formed test cases #
##########################

cd "$TS/xmlconf"
for xmldir in ibm/valid/P* \
              ibm/invalid/P* \
              xmltest/valid/ext-sa \
              xmltest/valid/not-sa \
              xmltest/invalid \
              xmltest/invalid/not-sa \
              xmltest/valid/sa \
              sun/valid \
              sun/invalid ; do
  cd "$TS/xmlconf/$xmldir"
  mkdir -p "$OUTPUT$xmldir"
  for xmlfile in *.xml ; do
      RunXmlwfWF "$xmlfile" "$xmldir/"
      UpdateStatus $?
  done
  rm outfile
done

cd "$TS/xmlconf/oasis"
mkdir -p "$OUTPUT"oasis
for xmlfile in *pass*.xml ; do
    RunXmlwfWF "$xmlfile" "oasis/"
    UpdateStatus $?
done
rm outfile

##############################
# not well-formed test cases #
##############################

cd "$TS/xmlconf"
for xmldir in ibm/not-wf/P* \
              ibm/not-wf/p28a \
              ibm/not-wf/misc \
              xmltest/not-wf/ext-sa \
              xmltest/not-wf/not-sa \
              xmltest/not-wf/sa \
              sun/not-wf ; do
  cd "$TS/xmlconf/$xmldir"
  for xmlfile in *.xml ; do
      RunXmlwfNotWF "$xmlfile" "$xmldir/"
      UpdateStatus $?
  done
  rm outfile
done

cd "$TS/xmlconf/oasis"
for xmlfile in *fail*.xml ; do
    RunXmlwfNotWF "$xmlfile" "oasis/"
    UpdateStatus $?
done
rm outfile

echo "Passed: $SUCCESS"
echo "Failed: $ERROR"