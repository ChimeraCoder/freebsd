
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

/*
 * Copyright (c) 2009, 2010, Oracle and/or its affiliates. All rights reserved.
 */

/*
 * String helper functions
 */

#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include "libuutil.h"

/* Return true if strings are equal */
boolean_t
uu_streq(const char *a, const char *b)
{
	return (strcmp(a, b) == 0);
}

/* Return true if strings are equal, case-insensitively */
boolean_t
uu_strcaseeq(const char *a, const char *b)
{
	return (strcasecmp(a, b) == 0);
}

/* Return true if string a Begins With string b */
boolean_t
uu_strbw(const char *a, const char *b)
{
	return (strncmp(a, b, strlen(b)) == 0);
}