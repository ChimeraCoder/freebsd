
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
#include <string.h>
#include <openssl/e_os2.h>

#ifdef OPENSSL_NO_ENGINE
int main(int argc, char *argv[])
{
    printf("No ENGINE support\n");
    return(0);
}
#else
#include <openssl/buffer.h>
#include <openssl/crypto.h>
#include <openssl/engine.h>
#include <openssl/err.h>

static void display_engine_list(void)
	{
	ENGINE *h;
	int loop;

	h = ENGINE_get_first();
	loop = 0;
	printf("listing available engine types\n");
	while(h)
		{
		printf("engine %i, id = \"%s\", name = \"%s\"\n",
			loop++, ENGINE_get_id(h), ENGINE_get_name(h));
		h = ENGINE_get_next(h);
		}
	printf("end of list\n");
	/* ENGINE_get_first() increases the struct_ref counter, so we 
           must call ENGINE_free() to decrease it again */
	ENGINE_free(h);
	}

int main(int argc, char *argv[])
	{
	ENGINE *block[512];
	char buf[256];
	const char *id, *name;
	ENGINE *ptr;
	int loop;
	int to_return = 1;
	ENGINE *new_h1 = NULL;
	ENGINE *new_h2 = NULL;
	ENGINE *new_h3 = NULL;
	ENGINE *new_h4 = NULL;

	/* enable memory leak checking unless explicitly disabled */
	if (!((getenv("OPENSSL_DEBUG_MEMORY") != NULL) && (0 == strcmp(getenv("OPENSSL_DEBUG_MEMORY"), "off"))))
		{
		CRYPTO_malloc_debug_init();
		CRYPTO_set_mem_debug_options(V_CRYPTO_MDEBUG_ALL);
		}
	else
		{
		/* OPENSSL_DEBUG_MEMORY=off */
		CRYPTO_set_mem_debug_functions(0, 0, 0, 0, 0);
		}
	CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
	ERR_load_crypto_strings();

	memset(block, 0, 512 * sizeof(ENGINE *));
	if(((new_h1 = ENGINE_new()) == NULL) ||
			!ENGINE_set_id(new_h1, "test_id0") ||
			!ENGINE_set_name(new_h1, "First test item") ||
			((new_h2 = ENGINE_new()) == NULL) ||
			!ENGINE_set_id(new_h2, "test_id1") ||
			!ENGINE_set_name(new_h2, "Second test item") ||
			((new_h3 = ENGINE_new()) == NULL) ||
			!ENGINE_set_id(new_h3, "test_id2") ||
			!ENGINE_set_name(new_h3, "Third test item") ||
			((new_h4 = ENGINE_new()) == NULL) ||
			!ENGINE_set_id(new_h4, "test_id3") ||
			!ENGINE_set_name(new_h4, "Fourth test item"))
		{
		printf("Couldn't set up test ENGINE structures\n");
		goto end;
		}
	printf("\nenginetest beginning\n\n");
	display_engine_list();
	if(!ENGINE_add(new_h1))
		{
		printf("Add failed!\n");
		goto end;
		}
	display_engine_list();
	ptr = ENGINE_get_first();
	if(!ENGINE_remove(ptr))
		{
		printf("Remove failed!\n");
		goto end;
		}
	if (ptr)
		ENGINE_free(ptr);
	display_engine_list();
	if(!ENGINE_add(new_h3) || !ENGINE_add(new_h2))
		{
		printf("Add failed!\n");
		goto end;
		}
	display_engine_list();
	if(!ENGINE_remove(new_h2))
		{
		printf("Remove failed!\n");
		goto end;
		}
	display_engine_list();
	if(!ENGINE_add(new_h4))
		{
		printf("Add failed!\n");
		goto end;
		}
	display_engine_list();
	if(ENGINE_add(new_h3))
		{
		printf("Add *should* have failed but didn't!\n");
		goto end;
		}
	else
		printf("Add that should fail did.\n");
	ERR_clear_error();
	if(ENGINE_remove(new_h2))
		{
		printf("Remove *should* have failed but didn't!\n");
		goto end;
		}
	else
		printf("Remove that should fail did.\n");
	ERR_clear_error();
	if(!ENGINE_remove(new_h3))
		{
		printf("Remove failed!\n");
		goto end;
		}
	display_engine_list();
	if(!ENGINE_remove(new_h4))
		{
		printf("Remove failed!\n");
		goto end;
		}
	display_engine_list();
	/* Depending on whether there's any hardware support compiled
	 * in, this remove may be destined to fail. */
	ptr = ENGINE_get_first();
	if(ptr)
		if(!ENGINE_remove(ptr))
			printf("Remove failed!i - probably no hardware "
				"support present.\n");
	if (ptr)
		ENGINE_free(ptr);
	display_engine_list();
	if(!ENGINE_add(new_h1) || !ENGINE_remove(new_h1))
		{
		printf("Couldn't add and remove to an empty list!\n");
		goto end;
		}
	else
		printf("Successfully added and removed to an empty list!\n");
	printf("About to beef up the engine-type list\n");
	for(loop = 0; loop < 512; loop++)
		{
		sprintf(buf, "id%i", loop);
		id = BUF_strdup(buf);
		sprintf(buf, "Fake engine type %i", loop);
		name = BUF_strdup(buf);
		if(((block[loop] = ENGINE_new()) == NULL) ||
				!ENGINE_set_id(block[loop], id) ||
				!ENGINE_set_name(block[loop], name))
			{
			printf("Couldn't create block of ENGINE structures.\n"
				"I'll probably also core-dump now, damn.\n");
			goto end;
			}
		}
	for(loop = 0; loop < 512; loop++)
		{
		if(!ENGINE_add(block[loop]))
			{
			printf("\nAdding stopped at %i, (%s,%s)\n",
				loop, ENGINE_get_id(block[loop]),
				ENGINE_get_name(block[loop]));
			goto cleanup_loop;
			}
		else
			printf("."); fflush(stdout);
		}
cleanup_loop:
	printf("\nAbout to empty the engine-type list\n");
	while((ptr = ENGINE_get_first()) != NULL)
		{
		if(!ENGINE_remove(ptr))
			{
			printf("\nRemove failed!\n");
			goto end;
			}
		ENGINE_free(ptr);
		printf("."); fflush(stdout);
		}
	for(loop = 0; loop < 512; loop++)
		{
		OPENSSL_free((void *)ENGINE_get_id(block[loop]));
		OPENSSL_free((void *)ENGINE_get_name(block[loop]));
		}
	printf("\nTests completed happily\n");
	to_return = 0;
end:
	if(to_return)
		ERR_print_errors_fp(stderr);
	if(new_h1) ENGINE_free(new_h1);
	if(new_h2) ENGINE_free(new_h2);
	if(new_h3) ENGINE_free(new_h3);
	if(new_h4) ENGINE_free(new_h4);
	for(loop = 0; loop < 512; loop++)
		if(block[loop])
			ENGINE_free(block[loop]);
	ENGINE_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
	ERR_remove_thread_state(NULL);
	CRYPTO_mem_leaks_fp(stderr);
	return to_return;
	}
#endif