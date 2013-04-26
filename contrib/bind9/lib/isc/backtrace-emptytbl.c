
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

/* $Id: backtrace-emptytbl.c,v 1.3 2009/09/01 20:13:44 each Exp $ */

/*! \file */

/*
 * This file defines an empty (default) symbol table used in backtrace.c
 * If the application wants to have a complete symbol table, it should redefine
 * isc__backtrace_symtable with the complete table in some way, and link the
 * version of the library not including this definition
 * (e.g. libisc-nosymbol.a).
 */

#include <config.h>

#include <isc/backtrace.h>

const int isc__backtrace_nsymbols = 0;
const isc_backtrace_symmap_t isc__backtrace_symtable[] = { { NULL, "" } };