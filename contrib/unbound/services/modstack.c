
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

/**
 * \file
 *
 * This file contains functions to help maintain a stack of modules.
 */
#include "config.h"
#include <ctype.h>
#include "services/modstack.h"
#include "util/module.h"
#include "util/fptr_wlist.h"
#include "iterator/iterator.h"
#include "validator/validator.h"

#ifdef WITH_PYTHONMODULE
#include "pythonmod/pythonmod.h"
#endif

/** count number of modules (words) in the string */
static int
count_modules(const char* s)
{
        int num = 0;
        if(!s)
                return 0;
        while(*s) {
                /* skip whitespace */
                while(*s && isspace((int)*s))
                        s++;
                if(*s && !isspace((int)*s)) {
                        /* skip identifier */
                        num++;
                        while(*s && !isspace((int)*s))
                                s++;
                }
        }
        return num;
}

void 
modstack_init(struct module_stack* stack)
{
	stack->num = 0;
	stack->mod = NULL;
}

int 
modstack_config(struct module_stack* stack, const char* module_conf)
{
        int i;
        verbose(VERB_QUERY, "module config: \"%s\"", module_conf);
        stack->num = count_modules(module_conf);
        if(stack->num == 0) {
                log_err("error: no modules specified");
                return 0;
        }
        if(stack->num > MAX_MODULE) {
                log_err("error: too many modules (%d max %d)",
                        stack->num, MAX_MODULE);
                return 0;
        }
        stack->mod = (struct module_func_block**)calloc((size_t)
                stack->num, sizeof(struct module_func_block*));
        if(!stack->mod) {
                log_err("out of memory");
                return 0;
        }
        for(i=0; i<stack->num; i++) {
                stack->mod[i] = module_factory(&module_conf);
                if(!stack->mod[i]) {
                        log_err("Unknown value for next module: '%s'",
                                module_conf);
                        return 0;
                }
        }
        return 1;
}

/** The list of module names */
const char**
module_list_avail(void)
{
        /* these are the modules available */
        static const char* names[] = {
#ifdef WITH_PYTHONMODULE
		"python", 
#endif
		"validator", 
		"iterator", 
		NULL};
	return names;
}

/** func block get function type */
typedef struct module_func_block* (*fbgetfunctype)(void);

/** The list of module func blocks */
static fbgetfunctype*
module_funcs_avail(void)
{
        static struct module_func_block* (*fb[])(void) = {
#ifdef WITH_PYTHONMODULE
		&pythonmod_get_funcblock, 
#endif
		&val_get_funcblock, 
		&iter_get_funcblock, 
		NULL};
	return fb;
}

struct 
module_func_block* module_factory(const char** str)
{
        int i = 0;
        const char* s = *str;
	const char** names = module_list_avail();
	fbgetfunctype* fb = module_funcs_avail();
        while(*s && isspace((int)*s))
                s++;
	while(names[i]) {
                if(strncmp(names[i], s, strlen(names[i])) == 0) {
                        s += strlen(names[i]);
                        *str = s;
                        return (*fb[i])();
                }
		i++;
        }
        return NULL;
}

int 
modstack_setup(struct module_stack* stack, const char* module_conf,
	struct module_env* env)
{
        int i;
        if(stack->num != 0)
                modstack_desetup(stack, env);
        /* fixed setup of the modules */
        if(!modstack_config(stack, module_conf)) {
		return 0;
        }
        env->need_to_validate = 0; /* set by module init below */
        for(i=0; i<stack->num; i++) {
                verbose(VERB_OPS, "init module %d: %s",
                        i, stack->mod[i]->name);
                fptr_ok(fptr_whitelist_mod_init(stack->mod[i]->init));
                if(!(*stack->mod[i]->init)(env, i)) {
                        log_err("module init for module %s failed",
                                stack->mod[i]->name);
			return 0;
                }
        }
	return 1;
}

void 
modstack_desetup(struct module_stack* stack, struct module_env* env)
{
        int i;
        for(i=0; i<stack->num; i++) {
                fptr_ok(fptr_whitelist_mod_deinit(stack->mod[i]->deinit));
                (*stack->mod[i]->deinit)(env, i);
        }
        stack->num = 0;
        free(stack->mod);
        stack->mod = NULL;
}

int 
modstack_find(struct module_stack* stack, const char* name)
{
	int i;
        for(i=0; i<stack->num; i++) {
		if(strcmp(stack->mod[i]->name, name) == 0)
			return i;
	}
	return -1;
}