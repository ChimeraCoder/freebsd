
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
#include "cryptlib.h"
#include <openssl/asn1.h>
#include <openssl/conf.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

/* Test application to add extensions from a config file */

int main(int argc, char **argv)
{
	LHASH *conf;
	X509 *cert;
	FILE *inf;
	char *conf_file;
	int i;
	int count;
	X509_EXTENSION *ext;
	X509V3_add_standard_extensions();
	ERR_load_crypto_strings();
	if(!argv[1]) {
		fprintf(stderr, "Usage: v3conf cert.pem [file.cnf]\n");
		exit(1);
	}
	conf_file = argv[2];
	if(!conf_file) conf_file = "test.cnf";
	conf = CONF_load(NULL, "test.cnf", NULL);
	if(!conf) {
		fprintf(stderr, "Error opening Config file %s\n", conf_file);
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	inf = fopen(argv[1], "r");
	if(!inf) {
		fprintf(stderr, "Can't open certificate file %s\n", argv[1]);
		exit(1);
	}
	cert = PEM_read_X509(inf, NULL, NULL);
	if(!cert) {
		fprintf(stderr, "Error reading certificate file %s\n", argv[1]);
		exit(1);
	}
	fclose(inf);

	sk_pop_free(cert->cert_info->extensions, X509_EXTENSION_free);
	cert->cert_info->extensions = NULL;

	if(!X509V3_EXT_add_conf(conf, NULL, "test_section", cert)) {
		fprintf(stderr, "Error adding extensions\n");
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	count = X509_get_ext_count(cert);
	printf("%d extensions\n", count);
	for(i = 0; i < count; i++) {
		ext = X509_get_ext(cert, i);
		printf("%s", OBJ_nid2ln(OBJ_obj2nid(ext->object)));
		if(ext->critical) printf(",critical:\n");
		else printf(":\n");
		X509V3_EXT_print_fp(stdout, ext, 0, 0);
		printf("\n");
		
	}
	return 0;
}