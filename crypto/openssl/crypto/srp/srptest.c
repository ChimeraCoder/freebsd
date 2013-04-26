
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
	/* use builtin 1024-bit params */	SRP_gN *GN = SRP_get_default_gN("1024");

	if(GN == NULL)
		{
		fprintf(stderr, "Failed to get SRP parameters\n");
		return -1;
		}
	/* Set up server's password entry */
	if(!SRP_create_verifier_BN(username, server_pass, &s, &v, GN->N, GN->g))
		{
		fprintf(stderr, "Failed to create SRP verifier\n");
		return -1;
		}

	showbn("N", GN->N);
	showbn("g", GN->g);
	showbn("Salt", s);
	showbn("Verifier", v);

	/* Server random */
	RAND_pseudo_bytes(rand_tmp, sizeof(rand_tmp));
	b = BN_bin2bn(rand_tmp, sizeof(rand_tmp), NULL);
	/* TODO - check b != 0 */
	showbn("b", b);

	/* Server's first message */
	Bpub = SRP_Calc_B(b, GN->N, GN->g, v);
	showbn("B", Bpub);

	if(!SRP_Verify_B_mod_N(Bpub, GN->N))
		{
		fprintf(stderr, "Invalid B\n");
		return -1;
		}

	/* Client random */
	RAND_pseudo_bytes(rand_tmp, sizeof(rand_tmp));
	a = BN_bin2bn(rand_tmp, sizeof(rand_tmp), NULL);
	/* TODO - check a != 0 */
	showbn("a", a);

	/* Client's response */
	Apub = SRP_Calc_A(a, GN->N, GN->g);
	showbn("A", Apub);

	if(!SRP_Verify_A_mod_N(Apub, GN->N))
		{
		fprintf(stderr, "Invalid A\n");
		return -1;
		}

	/* Both sides calculate u */
	u = SRP_Calc_u(Apub, Bpub, GN->N);

	/* Client's key */
	x = SRP_Calc_x(s, username, client_pass);
	Kclient = SRP_Calc_client_key(GN->N, Bpub, GN->g, x, a, u);
	showbn("Client's key", Kclient);

	/* Server's key */
	Kserver = SRP_Calc_server_key(Apub, v, u, b, GN->N);
	showbn("Server's key", Kserver);

	if(BN_cmp(Kclient, Kserver) == 0)
		{
		ret = 0;
		}
	else
		{
		fprintf(stderr, "Keys mismatch\n");
		ret = 1;
		}

	BN_clear_free(Kclient);
	BN_clear_free(Kserver);
	BN_clear_free(x);
	BN_free(u);
	BN_free(Apub);
	BN_clear_free(a);
	BN_free(Bpub);
	BN_clear_free(b);
	BN_free(s);
	BN_clear_free(v);

	return ret;
	}

int main(int argc, char **argv)
	{
	BIO *bio_err;
	bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

	CRYPTO_malloc_debug_init();
	CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
	CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);

	ERR_load_crypto_strings();

	/* "Negative" test, expect a mismatch */
	if(run_srp("alice", "password1", "password2") == 0)
		{
		fprintf(stderr, "Mismatched SRP run failed\n");
		return 1;
		}

	/* "Positive" test, should pass */
	if(run_srp("alice", "password", "password") != 0)
		{
		fprintf(stderr, "Plain SRP run failed\n");
		return 1;
		}

	CRYPTO_cleanup_all_ex_data();
	ERR_remove_thread_state(NULL);
	ERR_free_strings();
	CRYPTO_mem_leaks(bio_err);

	return 0;
	}
#endif