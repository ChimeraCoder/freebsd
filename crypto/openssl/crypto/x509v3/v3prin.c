
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



#include <stdio.h>
#include <openssl/asn1.h>
#include <openssl/conf.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

int main(int argc, char **argv)
{
	X509 *cert;
	FILE *inf;
	int i, count;
	X509_EXTENSION *ext;
	X509V3_add_standard_extensions();
	ERR_load_crypto_strings();
	if(!argv[1]) {
		fprintf(stderr, "Usage v3prin cert.pem\n");
		exit(1);
	}
	if(!(inf = fopen(argv[1], "r"))) {
		fprintf(stderr, "Can't open %s\n", argv[1]);
		exit(1);
	}
	if(!(cert = PEM_read_X509(inf, NULL, NULL))) {
		fprintf(stderr, "Can't read certificate %s\n", argv[1]);
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	fclose(inf);
	count = X509_get_ext_count(cert);
	printf("%d extensions\n", count);
	for(i = 0; i < count; i++) {
		ext = X509_get_ext(cert, i);
		printf("%s\n", OBJ_nid2ln(OBJ_obj2nid(ext->object)));
		if(!X509V3_EXT_print_fp(stdout, ext, 0, 0)) ERR_print_errors_fp(stderr);
		printf("\n");
		
	}
	return 0;
}