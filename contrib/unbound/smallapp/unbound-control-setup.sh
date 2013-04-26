#!/bin/sh
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

# settings:

# directory for files
DESTDIR=/usr/local/etc/unbound

# issuer and subject name for certificates
SERVERNAME=unbound
CLIENTNAME=unbound-control

# validity period for certificates
DAYS=7200

# size of keys in bits
BITS=1536

# hash algorithm
HASH=sha256

# base name for unbound server keys
SVR_BASE=unbound_server

# base name for unbound-control keys
CTL_BASE=unbound_control

# we want -rw-r--- access (say you run this as root: grp=yes (server), all=no).
umask 0026

# end of options

# functions:
error ( ) {
	echo "$0 fatal error: $1"
	exit 1
}

# check arguments:
while test $# -ne 0; do
	case $1 in
	-d)
	if test $# -eq 1; then error "need argument for -d"; fi
	DESTDIR="$2"
	shift
	;;
	*)
	echo "unbound-control-setup.sh - setup SSL keys for unbound-control"
	echo "	-d dir	use directory to store keys and certificates."
	echo "		default: $DESTDIR"
	echo "please run this command using the same user id that the "
	echo "unbound daemon uses, it needs read privileges."
	exit 1
	;;
	esac
	shift
done

# go!:
echo "setup in directory $DESTDIR"
cd "$DESTDIR" || error "could not cd to $DESTDIR"

# create certificate keys; do not recreate if they already exist.
if test -f $SVR_BASE.key; then
	echo "$SVR_BASE.key exists"
else
	echo "generating $SVR_BASE.key"
	openssl genrsa -out $SVR_BASE.key $BITS || error "could not genrsa"
fi
if test -f $CTL_BASE.key; then
	echo "$CTL_BASE.key exists"
else
	echo "generating $CTL_BASE.key"
	openssl genrsa -out $CTL_BASE.key $BITS || error "could not genrsa"
fi

# create self-signed cert for server
cat >request.cfg <<EOF
[req]
default_bits=$BITS
default_md=$HASH
prompt=no
distinguished_name=req_distinguished_name

[req_distinguished_name]
commonName=$SERVERNAME
EOF
test -f request.cfg || error "could not create request.cfg"

echo "create $SVR_BASE.pem (self signed certificate)"
openssl req -key $SVR_BASE.key -config request.cfg  -new -x509 -days $DAYS -out $SVR_BASE.pem || error "could not create $SVR_BASE.pem"
# create trusted usage pem
openssl x509 -in $SVR_BASE.pem -addtrust serverAuth -out $SVR_BASE"_trust.pem"

# create client request and sign it, piped
cat >request.cfg <<EOF
[req]
default_bits=$BITS
default_md=$HASH
prompt=no
distinguished_name=req_distinguished_name

[req_distinguished_name]
commonName=$CLIENTNAME
EOF
test -f request.cfg || error "could not create request.cfg"

echo "create $CTL_BASE.pem (signed client certificate)"
openssl req -key $CTL_BASE.key -config request.cfg -new | openssl x509 -req -days $DAYS -CA $SVR_BASE"_trust.pem" -CAkey $SVR_BASE.key -CAcreateserial -$HASH -out $CTL_BASE.pem
test -f $CTL_BASE.pem || error "could not create $CTL_BASE.pem"
# create trusted usage pem
# openssl x509 -in $CTL_BASE.pem -addtrust clientAuth -out $CTL_BASE"_trust.pem"

# see details with openssl x509 -noout -text < $SVR_BASE.pem
# echo "create $CTL_BASE""_browser.pfx (web client certificate)"
# echo "create webbrowser PKCS#12 .PFX certificate file. In Firefox import in:"
# echo "preferences - advanced - encryption - view certificates - your certs"
# echo "empty password is used, simply click OK on the password dialog box."
# openssl pkcs12 -export -in $CTL_BASE"_trust.pem" -inkey $CTL_BASE.key -name "unbound remote control client cert" -out $CTL_BASE"_browser.pfx" -password "pass:" || error "could not create browser certificate"

# remove unused permissions
chmod o-rw $SVR_BASE.pem $SVR_BASE.key $CTL_BASE.pem $CTL_BASE.key

# remove crap
rm -f request.cfg
rm -f $CTL_BASE"_trust.pem" $SVR_BASE"_trust.pem" $SVR_BASE"_trust.srl"

echo "Setup success. Certificates created. Enable in unbound.conf file to use"

exit 0