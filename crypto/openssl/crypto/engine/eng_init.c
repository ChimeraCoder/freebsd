
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

#include "eng_int.h"

/* Initialise a engine type for use (or up its functional reference count
 * if it's already in use). This version is only used internally. */
int engine_unlocked_init(ENGINE *e)
	{
	int to_return = 1;

	if((e->funct_ref == 0) && e->init)
		/* This is the first functional reference and the engine
		 * requires initialisation so we do it now. */
		to_return = e->init(e);
	if(to_return)
		{
		/* OK, we return a functional reference which is also a
		 * structural reference. */
		e->struct_ref++;
		e->funct_ref++;
		engine_ref_debug(e, 0, 1)
		engine_ref_debug(e, 1, 1)
		}
	return to_return;
	}

/* Free a functional reference to a engine type. This version is only used
 * internally. */
int engine_unlocked_finish(ENGINE *e, int unlock_for_handlers)
	{
	int to_return = 1;

	/* Reduce the functional reference count here so if it's the terminating
	 * case, we can release the lock safely and call the finish() handler
	 * without risk of a race. We get a race if we leave the count until
	 * after and something else is calling "finish" at the same time -
	 * there's a chance that both threads will together take the count from
	 * 2 to 0 without either calling finish(). */
	e->funct_ref--;
	engine_ref_debug(e, 1, -1);
	if((e->funct_ref == 0) && e->finish)
		{
		if(unlock_for_handlers)
			CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
		to_return = e->finish(e);
		if(unlock_for_handlers)
			CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);
		if(!to_return)
			return 0;
		}
#ifdef REF_CHECK
	if(e->funct_ref < 0)
		{
		fprintf(stderr,"ENGINE_finish, bad functional reference count\n");
		abort();
		}
#endif
	/* Release the structural reference too */
	if(!engine_free_util(e, 0))
		{
		ENGINEerr(ENGINE_F_ENGINE_UNLOCKED_FINISH,ENGINE_R_FINISH_FAILED);
		return 0;
		}
	return to_return;
	}

/* The API (locked) version of "init" */
int ENGINE_init(ENGINE *e)
	{
	int ret;
	if(e == NULL)
		{
		ENGINEerr(ENGINE_F_ENGINE_INIT,ERR_R_PASSED_NULL_PARAMETER);
		return 0;
		}
	CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);
	ret = engine_unlocked_init(e);
	CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
	return ret;
	}

/* The API (locked) version of "finish" */
int ENGINE_finish(ENGINE *e)
	{
	int to_return = 1;

	if(e == NULL)
		{
		ENGINEerr(ENGINE_F_ENGINE_FINISH,ERR_R_PASSED_NULL_PARAMETER);
		return 0;
		}
	CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);
	to_return = engine_unlocked_finish(e, 1);
	CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
	if(!to_return)
		{
		ENGINEerr(ENGINE_F_ENGINE_FINISH,ENGINE_R_FINISH_FAILED);
		return 0;
		}
	return to_return;
	}