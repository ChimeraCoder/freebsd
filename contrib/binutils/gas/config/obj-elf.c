
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
/* ELF object file format   Copyright 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
   2001, 2002, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2,
   or (at your option) any later version.

   GAS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define OBJ_HEADER "obj-elf.h"
#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "obstack.h"
#include "struc-symbol.h"
#include "dwarf2dbg.h"

#ifndef ECOFF_DEBUGGING
#define ECOFF_DEBUGGING 0
#else
#define NEED_ECOFF_DEBUG
#endif

#ifdef NEED_ECOFF_DEBUG
#include "ecoff.h"
#endif

#ifdef TC_ALPHA
#include "elf/alpha.h"
#endif

#ifdef TC_MIPS
#include "elf/mips.h"
#endif

#ifdef TC_PPC
#include "elf/ppc.h"
#endif

#ifdef TC_I370
#include "elf/i370.h"
#endif

#ifdef TC_I386
#include "elf/x86-64.h"
#endif

#ifdef TC_MEP
#include "elf/mep.h"
#endif

static void obj_elf_line (int);
static void obj_elf_size (int);
static void obj_elf_type (int);
static void obj_elf_ident (int);
static void obj_elf_weak (int);
static void obj_elf_local (int);
static void obj_elf_visibility (int);
static void obj_elf_symver (int);
static void obj_elf_subsection (int);
static void obj_elf_popsection (int);
static void obj_elf_tls_common (int);
static void obj_elf_lcomm (int);
static void obj_elf_struct (int);

static const pseudo_typeS elf_pseudo_table[] =
{
  {"comm", obj_elf_common, 0},
  {"common", obj_elf_common, 1},
  {"ident", obj_elf_ident, 0},
  {"lcomm", obj_elf_lcomm, 0},
  {"local", obj_elf_local, 0},
  {"previous", obj_elf_previous, 0},
  {"section", obj_elf_section, 0},
  {"section.s", obj_elf_section, 0},
  {"sect", obj_elf_section, 0},
  {"sect.s", obj_elf_section, 0},
  {"pushsection", obj_elf_section, 1},
  {"popsection", obj_elf_popsection, 0},
  {"size", obj_elf_size, 0},
  {"type", obj_elf_type, 0},
  {"version", obj_elf_version, 0},
  {"weak", obj_elf_weak, 0},

  /* These define symbol visibility.  */
  {"internal", obj_elf_visibility, STV_INTERNAL},
  {"hidden", obj_elf_visibility, STV_HIDDEN},
  {"protected", obj_elf_visibility, STV_PROTECTED},

  /* These are used for stabs-in-elf configurations.  */
  {"line", obj_elf_line, 0},

  /* This is a GNU extension to handle symbol versions.  */
  {"symver", obj_elf_symver, 0},

  /* A GNU extension to change subsection only.  */
  {"subsection", obj_elf_subsection, 0},

  /* These are GNU extensions to aid in garbage collecting C++ vtables.  */
  {"vtable_inherit", (void (*) (int)) &obj_elf_vtable_inherit, 0},
  {"vtable_entry", (void (*) (int)) &obj_elf_vtable_entry, 0},

  /* These are used for dwarf.  */
  {"2byte", cons, 2},
  {"4byte", cons, 4},
  {"8byte", cons, 8},
  /* These are used for dwarf2.  */
  { "file", (void (*) (int)) dwarf2_directive_file, 0 },
  { "loc",  dwarf2_directive_loc,  0 },
  { "loc_mark_labels", dwarf2_directive_loc_mark_labels, 0 },

  /* We need to trap the section changing calls to handle .previous.  */
  {"data", obj_elf_data, 0},
  {"offset", obj_elf_struct, 0},
  {"struct", obj_elf_struct, 0},
  {"text", obj_elf_text, 0},

  {"tls_common", obj_elf_tls_common, 0},

  /* End sentinel.  */
  {NULL, NULL, 0},
};

static const pseudo_typeS ecoff_debug_pseudo_table[] =
{
#ifdef NEED_ECOFF_DEBUG
  /* COFF style debugging information for ECOFF. .ln is not used; .loc
     is used instead.  */
  { "def",	ecoff_directive_def,	0 },
  { "dim",	ecoff_directive_dim,	0 },
  { "endef",	ecoff_directive_endef,	0 },
  { "file",	ecoff_directive_file,	0 },
  { "scl",	ecoff_directive_scl,	0 },
  { "tag",	ecoff_directive_tag,	0 },
  { "val",	ecoff_directive_val,	0 },

  /* COFF debugging requires pseudo-ops .size and .type, but ELF
     already has meanings for those.  We use .esize and .etype
     instead.  These are only generated by gcc anyhow.  */
  { "esize",	ecoff_directive_size,	0 },
  { "etype",	ecoff_directive_type,	0 },

  /* ECOFF specific debugging information.  */
  { "begin",	ecoff_directive_begin,	0 },
  { "bend",	ecoff_directive_bend,	0 },
  { "end",	ecoff_directive_end,	0 },
  { "ent",	ecoff_directive_ent,	0 },
  { "fmask",	ecoff_directive_fmask,	0 },
  { "frame",	ecoff_directive_frame,	0 },
  { "loc",	ecoff_directive_loc,	0 },
  { "mask",	ecoff_directive_mask,	0 },

  /* Other ECOFF directives.  */
  { "extern",	ecoff_directive_extern,	0 },

  /* These are used on Irix.  I don't know how to implement them.  */
  { "alias",	s_ignore,		0 },
  { "bgnb",	s_ignore,		0 },
  { "endb",	s_ignore,		0 },
  { "lab",	s_ignore,		0 },
  { "noalias",	s_ignore,		0 },
  { "verstamp",	s_ignore,		0 },
  { "vreg",	s_ignore,		0 },
#endif

  {NULL, NULL, 0}			/* end sentinel */
};

#undef NO_RELOC
#include "aout/aout64.h"

/* This is called when the assembler starts.  */

asection *elf_com_section_ptr;

void
elf_begin (void)
{
  asection *s;

  /* Add symbols for the known sections to the symbol table.  */
  s = bfd_get_section_by_name (stdoutput, TEXT_SECTION_NAME);
  symbol_table_insert (section_symbol (s));
  s = bfd_get_section_by_name (stdoutput, DATA_SECTION_NAME);
  symbol_table_insert (section_symbol (s));
  s = bfd_get_section_by_name (stdoutput, BSS_SECTION_NAME);
  symbol_table_insert (section_symbol (s));
  elf_com_section_ptr = bfd_com_section_ptr;
}

void
elf_pop_insert (void)
{
  pop_insert (elf_pseudo_table);
  if (ECOFF_DEBUGGING)
    pop_insert (ecoff_debug_pseudo_table);
}

static bfd_vma
elf_s_get_size (symbolS *sym)
{
  return S_GET_SIZE (sym);
}

static void
elf_s_set_size (symbolS *sym, bfd_vma sz)
{
  S_SET_SIZE (sym, sz);
}

static bfd_vma
elf_s_get_align (symbolS *sym)
{
  return S_GET_ALIGN (sym);
}

static void
elf_s_set_align (symbolS *sym, bfd_vma align)
{
  S_SET_ALIGN (sym, align);
}

int
elf_s_get_other (symbolS *sym)
{
  return elf_symbol (symbol_get_bfdsym (sym))->internal_elf_sym.st_other;
}

static void
elf_s_set_other (symbolS *sym, int other)
{
  S_SET_OTHER (sym, other);
}

static int
elf_sec_sym_ok_for_reloc (asection *sec)
{
  return obj_sec_sym_ok_for_reloc (sec);
}

void
elf_file_symbol (const char *s, int appfile)
{
  if (!appfile
      || symbol_rootP == NULL
      || symbol_rootP->bsym == NULL
      || (symbol_rootP->bsym->flags & BSF_FILE) == 0)
    {
      symbolS *sym;

      sym = symbol_new (s, absolute_section, 0, NULL);
      symbol_set_frag (sym, &zero_address_frag);
      symbol_get_bfdsym (sym)->flags |= BSF_FILE;

      if (symbol_rootP != sym)
	{
	  symbol_remove (sym, &symbol_rootP, &symbol_lastP);
	  symbol_insert (sym, symbol_rootP, &symbol_rootP, &symbol_lastP);
#ifdef DEBUG
	  verify_symbol_chain (symbol_rootP, symbol_lastP);
#endif
	}
    }

#ifdef NEED_ECOFF_DEBUG
  ecoff_new_file (s, appfile);
#endif
}

/* Called from read.c:s_comm after we've parsed .comm symbol, size.
   Parse a possible alignment value.  */

symbolS *
elf_common_parse (int ignore ATTRIBUTE_UNUSED, symbolS *symbolP, addressT size)
{
  addressT align = 0;
  int is_local = symbol_get_obj (symbolP)->local;

  if (*input_line_pointer == ',')
    {
      char *save = input_line_pointer;

      input_line_pointer++;
      SKIP_WHITESPACE ();

      if (*input_line_pointer == '"')
	{
	  /* For sparc.  Accept .common symbol, length, "bss"  */
	  input_line_pointer++;
	  /* Some use the dot, some don't.  */
	  if (*input_line_pointer == '.')
	    input_line_pointer++;
	  /* Some say data, some say bss.  */
	  if (strncmp (input_line_pointer, "bss\"", 4) == 0)
	    input_line_pointer += 4;
	  else if (strncmp (input_line_pointer, "data\"", 5) == 0)
	    input_line_pointer += 5;
	  else
	    {
	      char *p = input_line_pointer;
	      char c;

	      while (*--p != '"')
		;
	      while (!is_end_of_line[(unsigned char) *input_line_pointer])
		if (*input_line_pointer++ == '"')
		  break;
	      c = *input_line_pointer;
	      *input_line_pointer = '\0';
	      as_bad (_("bad .common segment %s"), p);
	      *input_line_pointer = c;
	      ignore_rest_of_line ();
	      return NULL;
	    }
	  /* ??? Don't ask me why these are always global.  */
	  is_local = 0;
	}
      else
	{
	  input_line_pointer = save;
	  align = parse_align (is_local);
	  if (align == (addressT) -1)
	    return NULL;
	}
    }

  if (is_local)
    {
      bss_alloc (symbolP, size, align);
      S_CLEAR_EXTERNAL (symbolP);
    }
  else
    {
      S_SET_VALUE (symbolP, size);
      S_SET_ALIGN (symbolP, align);
      S_SET_EXTERNAL (symbolP);
      S_SET_SEGMENT (symbolP, elf_com_section_ptr);
    }

  symbol_get_bfdsym (symbolP)->flags |= BSF_OBJECT;

  return symbolP;
}

void
obj_elf_common (int is_common)
{
  if (flag_mri && is_common)
    s_mri_common (0);
  else
    s_comm_internal (0, elf_common_parse);
}

static void
obj_elf_tls_common (int ignore ATTRIBUTE_UNUSED)
{
  symbolS *symbolP = s_comm_internal (0, elf_common_parse);

  if (symbolP)
    symbol_get_bfdsym (symbolP)->flags |= BSF_THREAD_LOCAL;
}

static void
obj_elf_lcomm (int ignore ATTRIBUTE_UNUSED)
{
  symbolS *symbolP = s_comm_internal (0, s_lcomm_internal);

  if (symbolP)
    symbol_get_bfdsym (symbolP)->flags |= BSF_OBJECT;
}

static void
obj_elf_local (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  int c;
  symbolS *symbolP;

  do
    {
      name = input_line_pointer;
      c = get_symbol_end ();
      symbolP = symbol_find_or_make (name);
      *input_line_pointer = c;
      SKIP_WHITESPACE ();
      S_CLEAR_EXTERNAL (symbolP);
      symbol_get_obj (symbolP)->local = 1;
      if (c == ',')
	{
	  input_line_pointer++;
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer == '\n')
	    c = '\n';
	}
    }
  while (c == ',');
  demand_empty_rest_of_line ();
}

static void
obj_elf_weak (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  int c;
  symbolS *symbolP;

  do
    {
      name = input_line_pointer;
      c = get_symbol_end ();
      symbolP = symbol_find_or_make (name);
      *input_line_pointer = c;
      SKIP_WHITESPACE ();
      S_SET_WEAK (symbolP);
      symbol_get_obj (symbolP)->local = 1;
      if (c == ',')
	{
	  input_line_pointer++;
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer == '\n')
	    c = '\n';
	}
    }
  while (c == ',');
  demand_empty_rest_of_line ();
}

static void
obj_elf_visibility (int visibility)
{
  char *name;
  int c;
  symbolS *symbolP;
  asymbol *bfdsym;
  elf_symbol_type *elfsym;

  do
    {
      name = input_line_pointer;
      c = get_symbol_end ();
      symbolP = symbol_find_or_make (name);
      *input_line_pointer = c;

      SKIP_WHITESPACE ();

      bfdsym = symbol_get_bfdsym (symbolP);
      elfsym = elf_symbol_from (bfd_asymbol_bfd (bfdsym), bfdsym);

      assert (elfsym);

      elfsym->internal_elf_sym.st_other &= ~3;
      elfsym->internal_elf_sym.st_other |= visibility;

      if (c == ',')
	{
	  input_line_pointer ++;

	  SKIP_WHITESPACE ();

	  if (*input_line_pointer == '\n')
	    c = '\n';
	}
    }
  while (c == ',');

  demand_empty_rest_of_line ();
}

static segT previous_section;
static int previous_subsection;

struct section_stack
{
  struct section_stack *next;
  segT seg, prev_seg;
  int subseg, prev_subseg;
};

static struct section_stack *section_stack;

static bfd_boolean
get_section (bfd *abfd ATTRIBUTE_UNUSED, asection *sec, void *inf)
{
  const char *gname = inf;
  const char *group_name = elf_group_name (sec);
  
  return (group_name == gname
	  || (group_name != NULL
	      && gname != NULL
	      && strcmp (group_name, gname) == 0));
}

/* Handle the .section pseudo-op.  This code supports two different
   syntaxes.

   The first is found on Solaris, and looks like
       .section ".sec1",#alloc,#execinstr,#write
   Here the names after '#' are the SHF_* flags to turn on for the
   section.  I'm not sure how it determines the SHT_* type (BFD
   doesn't really give us control over the type, anyhow).

   The second format is found on UnixWare, and probably most SVR4
   machines, and looks like
       .section .sec1,"a",@progbits
   The quoted string may contain any combination of a, w, x, and
   represents the SHF_* flags to turn on for the section.  The string
   beginning with '@' can be progbits or nobits.  There should be
   other possibilities, but I don't know what they are.  In any case,
   BFD doesn't really let us set the section type.  */

void
obj_elf_change_section (const char *name,
			int type,
			int attr,
			int entsize,
			const char *group_name,
			int linkonce,
			int push)
{
  asection *old_sec;
  segT sec;
  flagword flags;
  const struct elf_backend_data *bed;
  const struct bfd_elf_special_section *ssect;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  /* Switch to the section, creating it if necessary.  */
  if (push)
    {
      struct section_stack *elt;
      elt = xmalloc (sizeof (struct section_stack));
      elt->next = section_stack;
      elt->seg = now_seg;
      elt->prev_seg = previous_section;
      elt->subseg = now_subseg;
      elt->prev_subseg = previous_subsection;
      section_stack = elt;
    }
  previous_section = now_seg;
  previous_subsection = now_subseg;

  old_sec = bfd_get_section_by_name_if (stdoutput, name, get_section,
					(void *) group_name);
  if (old_sec)
    {
      sec = old_sec;
      subseg_set (sec, 0);
    }
  else
    sec = subseg_force_new (name, 0);

  bed = get_elf_backend_data (stdoutput);
  ssect = (*bed->get_sec_type_attr) (stdoutput, sec);

  if (ssect != NULL)
    {
      bfd_boolean override = FALSE;

      if (type == SHT_NULL)
	type = ssect->type;
      else if (type != ssect->type)
	{
	  if (old_sec == NULL
	      /* FIXME: gcc, as of 2002-10-22, will emit

		 .section .init_array,"aw",@progbits

		 for __attribute__ ((section (".init_array"))).
		 "@progbits" is incorrect.  Also for x86-64 large bss
		 sections, gcc, as of 2005-07-06, will emit

		 .section .lbss,"aw",@progbits

		 "@progbits" is incorrect.  */
#ifdef TC_I386
	      && (bed->s->arch_size != 64
		  || !(ssect->attr & SHF_X86_64_LARGE))
#endif
	      && ssect->type != SHT_INIT_ARRAY
	      && ssect->type != SHT_FINI_ARRAY
	      && ssect->type != SHT_PREINIT_ARRAY)
	    {
	      /* We allow to specify any type for a .note section.  */
	      if (ssect->type != SHT_NOTE)
		as_warn (_("setting incorrect section type for %s"),
			 name);
	    }
	  else
	    {
	      as_warn (_("ignoring incorrect section type for %s"),
		       name);
	      type = ssect->type;
	    }
	}

      if (old_sec == NULL && (attr & ~ssect->attr) != 0)
	{
	  /* As a GNU extension, we permit a .note section to be
	     allocatable.  If the linker sees an allocatable .note
	     section, it will create a PT_NOTE segment in the output
	     file.  We also allow "x" for .note.GNU-stack.  */
	  if (ssect->type == SHT_NOTE
	      && (attr == SHF_ALLOC || attr == SHF_EXECINSTR))
	    ;
	  /* Allow different SHF_MERGE and SHF_STRINGS if we have
	     something like .rodata.str.  */
	  else if (ssect->suffix_length == -2
		   && name[ssect->prefix_length] == '.'
		   && (attr
		       & ~ssect->attr
		       & ~SHF_MERGE
		       & ~SHF_STRINGS) == 0)
	    ;
	  /* .interp, .strtab and .symtab can have SHF_ALLOC.  */
	  else if (attr == SHF_ALLOC
		   && (strcmp (name, ".interp") == 0
		       || strcmp (name, ".strtab") == 0
		       || strcmp (name, ".symtab") == 0))
	    override = TRUE;
	  /* .note.GNU-stack can have SHF_EXECINSTR.  */
	  else if (attr == SHF_EXECINSTR
		   && strcmp (name, ".note.GNU-stack") == 0)
	    override = TRUE;
#ifdef TC_ALPHA
	  /* A section on Alpha may have SHF_ALPHA_GPREL.  */
	  else if ((attr & ~ssect->attr) == SHF_ALPHA_GPREL)
	    override = TRUE;
#endif
	  else
	    {
	      if (group_name == NULL)
		as_warn (_("setting incorrect section attributes for %s"),
			 name);
	      override = TRUE;
	    }
	}
      if (!override && old_sec == NULL)
	attr |= ssect->attr;
    }

  /* Convert ELF type and flags to BFD flags.  */
  flags = (SEC_RELOC
	   | ((attr & SHF_WRITE) ? 0 : SEC_READONLY)
	   | ((attr & SHF_ALLOC) ? SEC_ALLOC : 0)
	   | (((attr & SHF_ALLOC) && type != SHT_NOBITS) ? SEC_LOAD : 0)
	   | ((attr & SHF_EXECINSTR) ? SEC_CODE : 0)
	   | ((attr & SHF_MERGE) ? SEC_MERGE : 0)
	   | ((attr & SHF_STRINGS) ? SEC_STRINGS : 0)
	   | ((attr & SHF_TLS) ? SEC_THREAD_LOCAL : 0));
#ifdef md_elf_section_flags
  flags = md_elf_section_flags (flags, attr, type);
#endif

  if (linkonce)
    flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;

  if (old_sec == NULL)
    {
      symbolS *secsym;

      elf_section_type (sec) = type;
      elf_section_flags (sec) = attr;

      /* Prevent SEC_HAS_CONTENTS from being inadvertently set.  */
      if (type == SHT_NOBITS)
	seg_info (sec)->bss = 1;

      bfd_set_section_flags (stdoutput, sec, flags);
      if (flags & SEC_MERGE)
	sec->entsize = entsize;
      elf_group_name (sec) = group_name;

      /* Add a symbol for this section to the symbol table.  */
      secsym = symbol_find (name);
      if (secsym != NULL)
	symbol_set_bfdsym (secsym, sec->symbol);
      else
	symbol_table_insert (section_symbol (sec));
    }
  else
    {
      if (type != SHT_NULL
	  && (unsigned) type != elf_section_type (old_sec))
	as_warn (_("ignoring changed section type for %s"), name);

      if (attr != 0)
	{
	  /* If section attributes are specified the second time we see a
	     particular section, then check that they are the same as we
	     saw the first time.  */
	  if (((old_sec->flags ^ flags)
	       & (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE
		  | SEC_EXCLUDE | SEC_SORT_ENTRIES | SEC_MERGE | SEC_STRINGS
		  | SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD
		  | SEC_THREAD_LOCAL)))
	    as_warn (_("ignoring changed section attributes for %s"), name);
	  if ((flags & SEC_MERGE) && old_sec->entsize != (unsigned) entsize)
	    as_warn (_("ignoring changed section entity size for %s"), name);
	}
    }

#ifdef md_elf_section_change_hook
  md_elf_section_change_hook ();
#endif
}

static int
obj_elf_parse_section_letters (char *str, size_t len)
{
  int attr = 0;

  while (len > 0)
    {
      switch (*str)
	{
	case 'a':
	  attr |= SHF_ALLOC;
	  break;
	case 'w':
	  attr |= SHF_WRITE;
	  break;
	case 'x':
	  attr |= SHF_EXECINSTR;
	  break;
	case 'M':
	  attr |= SHF_MERGE;
	  break;
	case 'S':
	  attr |= SHF_STRINGS;
	  break;
	case 'G':
	  attr |= SHF_GROUP;
	  break;
	case 'T':
	  attr |= SHF_TLS;
	  break;
	/* Compatibility.  */
	case 'm':
	  if (*(str - 1) == 'a')
	    {
	      attr |= SHF_MERGE;
	      if (len > 1 && str[1] == 's')
		{
		  attr |= SHF_STRINGS;
		  str++, len--;
		}
	      break;
	    }
	default:
	  {
	    char *bad_msg = _("unrecognized .section attribute: want a,w,x,M,S,G,T");
#ifdef md_elf_section_letter
	    int md_attr = md_elf_section_letter (*str, &bad_msg);
	    if (md_attr >= 0)
	      attr |= md_attr;
	    else
#endif
	      as_fatal ("%s", bad_msg);
	  }
	  break;
	}
      str++, len--;
    }

  return attr;
}

static int
obj_elf_section_word (char *str, size_t len)
{
  if (len == 5 && strncmp (str, "write", 5) == 0)
    return SHF_WRITE;
  if (len == 5 && strncmp (str, "alloc", 5) == 0)
    return SHF_ALLOC;
  if (len == 9 && strncmp (str, "execinstr", 9) == 0)
    return SHF_EXECINSTR;
  if (len == 3 && strncmp (str, "tls", 3) == 0)
    return SHF_TLS;

#ifdef md_elf_section_word
  {
    int md_attr = md_elf_section_word (str, len);
    if (md_attr >= 0)
      return md_attr;
  }
#endif

  as_warn (_("unrecognized section attribute"));
  return 0;
}

static int
obj_elf_section_type (char *str, size_t len)
{
  if (len == 8 && strncmp (str, "progbits", 8) == 0)
    return SHT_PROGBITS;
  if (len == 6 && strncmp (str, "nobits", 6) == 0)
    return SHT_NOBITS;
  if (len == 4 && strncmp (str, "note", 4) == 0)
    return SHT_NOTE;
  if (len == 10 && strncmp (str, "init_array", 10) == 0)
    return SHT_INIT_ARRAY;
  if (len == 10 && strncmp (str, "fini_array", 10) == 0)
    return SHT_FINI_ARRAY;
  if (len == 13 && strncmp (str, "preinit_array", 13) == 0)
    return SHT_PREINIT_ARRAY;

#ifdef md_elf_section_type
  {
    int md_type = md_elf_section_type (str, len);
    if (md_type >= 0)
      return md_type;
  }
#endif

  as_warn (_("unrecognized section type"));
  return 0;
}

/* Get name of section.  */
static char *
obj_elf_section_name (void)
{
  char *name;

  SKIP_WHITESPACE ();
  if (*input_line_pointer == '"')
    {
      int dummy;

      name = demand_copy_C_string (&dummy);
      if (name == NULL)
	{
	  ignore_rest_of_line ();
	  return NULL;
	}
    }
  else
    {
      char *end = input_line_pointer;

      while (0 == strchr ("\n\t,; ", *end))
	end++;
      if (end == input_line_pointer)
	{
	  as_bad (_("missing name"));
	  ignore_rest_of_line ();
	  return NULL;
	}

      name = xmalloc (end - input_line_pointer + 1);
      memcpy (name, input_line_pointer, end - input_line_pointer);
      name[end - input_line_pointer] = '\0';
#ifdef tc_canonicalize_section_name
      name = tc_canonicalize_section_name (name);
#endif
      input_line_pointer = end;
    }
  SKIP_WHITESPACE ();
  return name;
}

void
obj_elf_section (int push)
{
  char *name, *group_name, *beg;
  int type, attr, dummy;
  int entsize;
  int linkonce;

#ifndef TC_I370
  if (flag_mri)
    {
      char mri_type;

#ifdef md_flush_pending_output
      md_flush_pending_output ();
#endif

      previous_section = now_seg;
      previous_subsection = now_subseg;

      s_mri_sect (&mri_type);

#ifdef md_elf_section_change_hook
      md_elf_section_change_hook ();
#endif

      return;
    }
#endif /* ! defined (TC_I370) */

  name = obj_elf_section_name ();
  if (name == NULL)
    return;
  type = SHT_NULL;
  attr = 0;
  group_name = NULL;
  entsize = 0;
  linkonce = 0;

  if (*input_line_pointer == ',')
    {
      /* Skip the comma.  */
      ++input_line_pointer;
      SKIP_WHITESPACE ();

      if (*input_line_pointer == '"')
	{
	  beg = demand_copy_C_string (&dummy);
	  if (beg == NULL)
	    {
	      ignore_rest_of_line ();
	      return;
	    }
	  attr |= obj_elf_parse_section_letters (beg, strlen (beg));

	  SKIP_WHITESPACE ();
	  if (*input_line_pointer == ',')
	    {
	      char c;
	      char *save = input_line_pointer;

	      ++input_line_pointer;
	      SKIP_WHITESPACE ();
	      c = *input_line_pointer;
	      if (c == '"')
		{
		  beg = demand_copy_C_string (&dummy);
		  if (beg == NULL)
		    {
		      ignore_rest_of_line ();
		      return;
		    }
		  type = obj_elf_section_type (beg, strlen (beg));
		}
	      else if (c == '@' || c == '%')
		{
		  beg = ++input_line_pointer;
		  c = get_symbol_end ();
		  *input_line_pointer = c;
		  type = obj_elf_section_type (beg, input_line_pointer - beg);
		}
	      else
		input_line_pointer = save;
	    }

	  SKIP_WHITESPACE ();
	  if ((attr & SHF_MERGE) != 0 && *input_line_pointer == ',')
	    {
	      ++input_line_pointer;
	      SKIP_WHITESPACE ();
	      entsize = get_absolute_expression ();
	      SKIP_WHITESPACE ();
	      if (entsize < 0)
		{
		  as_warn (_("invalid merge entity size"));
		  attr &= ~SHF_MERGE;
		  entsize = 0;
		}
	    }
	  else if ((attr & SHF_MERGE) != 0)
	    {
	      as_warn (_("entity size for SHF_MERGE not specified"));
	      attr &= ~SHF_MERGE;
	    }

	  if ((attr & SHF_GROUP) != 0 && *input_line_pointer == ',')
	    {
	      ++input_line_pointer;
	      group_name = obj_elf_section_name ();
	      if (group_name == NULL)
		attr &= ~SHF_GROUP;
	      else if (strncmp (input_line_pointer, ",comdat", 7) == 0)
		{
		  input_line_pointer += 7;
		  linkonce = 1;
		}
	      else if (strncmp (name, ".gnu.linkonce", 13) == 0)
		linkonce = 1;
	    }
	  else if ((attr & SHF_GROUP) != 0)
	    {
	      as_warn (_("group name for SHF_GROUP not specified"));
	      attr &= ~SHF_GROUP;
	    }
	}
      else
	{
	  do
	    {
	      char c;

	      SKIP_WHITESPACE ();
	      if (*input_line_pointer != '#')
		{
		  as_bad (_("character following name is not '#'"));
		  ignore_rest_of_line ();
		  return;
		}
	      beg = ++input_line_pointer;
	      c = get_symbol_end ();
	      *input_line_pointer = c;

	      attr |= obj_elf_section_word (beg, input_line_pointer - beg);

	      SKIP_WHITESPACE ();
	    }
	  while (*input_line_pointer++ == ',');
	  --input_line_pointer;
	}
    }

  demand_empty_rest_of_line ();

  obj_elf_change_section (name, type, attr, entsize, group_name, linkonce, push);
}

/* Change to the .data section.  */

void
obj_elf_data (int i)
{
#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  previous_section = now_seg;
  previous_subsection = now_subseg;
  s_data (i);

#ifdef md_elf_section_change_hook
  md_elf_section_change_hook ();
#endif
}

/* Change to the .text section.  */

void
obj_elf_text (int i)
{
#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  previous_section = now_seg;
  previous_subsection = now_subseg;
  s_text (i);

#ifdef md_elf_section_change_hook
  md_elf_section_change_hook ();
#endif
}

/* Change to the *ABS* section.  */

void
obj_elf_struct (int i)
{
#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  previous_section = now_seg;
  previous_subsection = now_subseg;
  s_struct (i);

#ifdef md_elf_section_change_hook
  md_elf_section_change_hook ();
#endif
}

static void
obj_elf_subsection (int ignore ATTRIBUTE_UNUSED)
{
  register int temp;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  previous_section = now_seg;
  previous_subsection = now_subseg;

  temp = get_absolute_expression ();
  subseg_set (now_seg, (subsegT) temp);
  demand_empty_rest_of_line ();

#ifdef md_elf_section_change_hook
  md_elf_section_change_hook ();
#endif
}

/* This can be called from the processor backends if they change
   sections.  */

void
obj_elf_section_change_hook (void)
{
  previous_section = now_seg;
  previous_subsection = now_subseg;
}

void
obj_elf_previous (int ignore ATTRIBUTE_UNUSED)
{
  segT new_section;
  int new_subsection;

  if (previous_section == 0)
    {
      as_warn (_(".previous without corresponding .section; ignored"));
      return;
    }

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  new_section = previous_section;
  new_subsection = previous_subsection;
  previous_section = now_seg;
  previous_subsection = now_subseg;
  subseg_set (new_section, new_subsection);

#ifdef md_elf_section_change_hook
  md_elf_section_change_hook ();
#endif
}

static void
obj_elf_popsection (int xxx ATTRIBUTE_UNUSED)
{
  struct section_stack *top = section_stack;

  if (top == NULL)
    {
      as_warn (_(".popsection without corresponding .pushsection; ignored"));
      return;
    }

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  section_stack = top->next;
  previous_section = top->prev_seg;
  previous_subsection = top->prev_subseg;
  subseg_set (top->seg, top->subseg);
  free (top);

#ifdef md_elf_section_change_hook
  md_elf_section_change_hook ();
#endif
}

static void
obj_elf_line (int ignore ATTRIBUTE_UNUSED)
{
  /* Assume delimiter is part of expression.  BSD4.2 as fails with
     delightful bug, so we are not being incompatible here.  */
  new_logical_line (NULL, get_absolute_expression ());
  demand_empty_rest_of_line ();
}

/* This handles the .symver pseudo-op, which is used to specify a
   symbol version.  The syntax is ``.symver NAME,SYMVERNAME''.
   SYMVERNAME may contain ELF_VER_CHR ('@') characters.  This
   pseudo-op causes the assembler to emit a symbol named SYMVERNAME
   with the same value as the symbol NAME.  */

static void
obj_elf_symver (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  char c;
  char old_lexat;
  symbolS *sym;

  name = input_line_pointer;
  c = get_symbol_end ();

  sym = symbol_find_or_make (name);

  *input_line_pointer = c;

  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad (_("expected comma after name in .symver"));
      ignore_rest_of_line ();
      return;
    }

  ++input_line_pointer;
  SKIP_WHITESPACE ();
  name = input_line_pointer;

  /* Temporarily include '@' in symbol names.  */
  old_lexat = lex_type[(unsigned char) '@'];
  lex_type[(unsigned char) '@'] |= LEX_NAME;
  c = get_symbol_end ();
  lex_type[(unsigned char) '@'] = old_lexat;

  if (symbol_get_obj (sym)->versioned_name == NULL)
    {
      symbol_get_obj (sym)->versioned_name = xstrdup (name);

      *input_line_pointer = c;

      if (strchr (symbol_get_obj (sym)->versioned_name,
		  ELF_VER_CHR) == NULL)
	{
	  as_bad (_("missing version name in `%s' for symbol `%s'"),
		  symbol_get_obj (sym)->versioned_name,
		  S_GET_NAME (sym));
	  ignore_rest_of_line ();
	  return;
	}
    }
  else
    {
      if (strcmp (symbol_get_obj (sym)->versioned_name, name))
	{
	  as_bad (_("multiple versions [`%s'|`%s'] for symbol `%s'"),
		  name, symbol_get_obj (sym)->versioned_name,
		  S_GET_NAME (sym));
	  ignore_rest_of_line ();
	  return;
	}

      *input_line_pointer = c;
    }

  demand_empty_rest_of_line ();
}

/* This handles the .vtable_inherit pseudo-op, which is used to indicate
   to the linker the hierarchy in which a particular table resides.  The
   syntax is ".vtable_inherit CHILDNAME, PARENTNAME".  */

struct fix *
obj_elf_vtable_inherit (int ignore ATTRIBUTE_UNUSED)
{
  char *cname, *pname;
  symbolS *csym, *psym;
  char c, bad = 0;

  if (*input_line_pointer == '#')
    ++input_line_pointer;

  cname = input_line_pointer;
  c = get_symbol_end ();
  csym = symbol_find (cname);

  /* GCFIXME: should check that we don't have two .vtable_inherits for
     the same child symbol.  Also, we can currently only do this if the
     child symbol is already exists and is placed in a fragment.  */

  if (csym == NULL || symbol_get_frag (csym) == NULL)
    {
      as_bad ("expected `%s' to have already been set for .vtable_inherit",
	      cname);
      bad = 1;
    }

  *input_line_pointer = c;

  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad ("expected comma after name in .vtable_inherit");
      ignore_rest_of_line ();
      return NULL;
    }

  ++input_line_pointer;
  SKIP_WHITESPACE ();

  if (*input_line_pointer == '#')
    ++input_line_pointer;

  if (input_line_pointer[0] == '0'
      && (input_line_pointer[1] == '\0'
	  || ISSPACE (input_line_pointer[1])))
    {
      psym = section_symbol (absolute_section);
      ++input_line_pointer;
    }
  else
    {
      pname = input_line_pointer;
      c = get_symbol_end ();
      psym = symbol_find_or_make (pname);
      *input_line_pointer = c;
    }

  demand_empty_rest_of_line ();

  if (bad)
    return NULL;

  assert (symbol_get_value_expression (csym)->X_op == O_constant);
  return fix_new (symbol_get_frag (csym),
		  symbol_get_value_expression (csym)->X_add_number,
		  0, psym, 0, 0, BFD_RELOC_VTABLE_INHERIT);
}

/* This handles the .vtable_entry pseudo-op, which is used to indicate
   to the linker that a vtable slot was used.  The syntax is
   ".vtable_entry tablename, offset".  */

struct fix *
obj_elf_vtable_entry (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  symbolS *sym;
  offsetT offset;
  char c;

  if (*input_line_pointer == '#')
    ++input_line_pointer;

  name = input_line_pointer;
  c = get_symbol_end ();
  sym = symbol_find_or_make (name);
  *input_line_pointer = c;

  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad ("expected comma after name in .vtable_entry");
      ignore_rest_of_line ();
      return NULL;
    }

  ++input_line_pointer;
  if (*input_line_pointer == '#')
    ++input_line_pointer;

  offset = get_absolute_expression ();

  demand_empty_rest_of_line ();

  return fix_new (frag_now, frag_now_fix (), 0, sym, offset, 0,
		  BFD_RELOC_VTABLE_ENTRY);
}

void
elf_obj_read_begin_hook (void)
{
#ifdef NEED_ECOFF_DEBUG
  if (ECOFF_DEBUGGING)
    ecoff_read_begin_hook ();
#endif
}

void
elf_obj_symbol_new_hook (symbolS *symbolP)
{
  struct elf_obj_sy *sy_obj;

  sy_obj = symbol_get_obj (symbolP);
  sy_obj->size = NULL;
  sy_obj->versioned_name = NULL;

#ifdef NEED_ECOFF_DEBUG
  if (ECOFF_DEBUGGING)
    ecoff_symbol_new_hook (symbolP);
#endif
}

/* When setting one symbol equal to another, by default we probably
   want them to have the same "size", whatever it means in the current
   context.  */

void
elf_copy_symbol_attributes (symbolS *dest, symbolS *src)
{
  struct elf_obj_sy *srcelf = symbol_get_obj (src);
  struct elf_obj_sy *destelf = symbol_get_obj (dest);
  if (srcelf->size)
    {
      if (destelf->size == NULL)
	destelf->size = xmalloc (sizeof (expressionS));
      *destelf->size = *srcelf->size;
    }
  else
    {
      if (destelf->size != NULL)
	free (destelf->size);
      destelf->size = NULL;
    }
  S_SET_SIZE (dest, S_GET_SIZE (src));
  /* Don't copy visibility.  */
  S_SET_OTHER (dest, (ELF_ST_VISIBILITY (S_GET_OTHER (dest))
		      | (S_GET_OTHER (src) & ~ELF_ST_VISIBILITY (-1))));
}

void
obj_elf_version (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  unsigned int c;
  char *p;
  asection *seg = now_seg;
  subsegT subseg = now_subseg;
  Elf_Internal_Note i_note;
  Elf_External_Note e_note;
  asection *note_secp = NULL;

  SKIP_WHITESPACE ();
  if (*input_line_pointer == '\"')
    {
      unsigned int len;

      ++input_line_pointer;	/* -> 1st char of string.  */
      name = input_line_pointer;

      while (is_a_char (c = next_char_of_string ()))
	;
      c = *input_line_pointer;
      *input_line_pointer = '\0';
      *(input_line_pointer - 1) = '\0';
      *input_line_pointer = c;

      /* Create the .note section.  */
      note_secp = subseg_new (".note", 0);
      bfd_set_section_flags (stdoutput,
			     note_secp,
			     SEC_HAS_CONTENTS | SEC_READONLY);

      /* Process the version string.  */
      len = strlen (name) + 1;

      /* PR 3456: Although the name field is padded out to an 4-byte
	 boundary, the namesz field should not be adjusted.  */
      i_note.namesz = len;
      i_note.descsz = 0;	/* No description.  */
      i_note.type = NT_VERSION;
      p = frag_more (sizeof (e_note.namesz));
      md_number_to_chars (p, i_note.namesz, sizeof (e_note.namesz));
      p = frag_more (sizeof (e_note.descsz));
      md_number_to_chars (p, i_note.descsz, sizeof (e_note.descsz));
      p = frag_more (sizeof (e_note.type));
      md_number_to_chars (p, i_note.type, sizeof (e_note.type));
      p = frag_more (len);
      memcpy (p, name, len);

      frag_align (2, 0, 0);

      subseg_set (seg, subseg);
    }
  else
    as_bad (_("expected quoted string"));

  demand_empty_rest_of_line ();
}

static void
obj_elf_size (int ignore ATTRIBUTE_UNUSED)
{
  char *name = input_line_pointer;
  char c = get_symbol_end ();
  char *p;
  expressionS exp;
  symbolS *sym;

  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      *p = 0;
      as_bad (_("expected comma after name `%s' in .size directive"), name);
      *p = c;
      ignore_rest_of_line ();
      return;
    }
  input_line_pointer++;
  expression (&exp);
  if (exp.X_op == O_absent)
    {
      as_bad (_("missing expression in .size directive"));
      exp.X_op = O_constant;
      exp.X_add_number = 0;
    }
  *p = 0;
  sym = symbol_find_or_make (name);
  *p = c;
  if (exp.X_op == O_constant)
    {
      S_SET_SIZE (sym, exp.X_add_number);
      if (symbol_get_obj (sym)->size)
	{
	  xfree (symbol_get_obj (sym)->size);
	  symbol_get_obj (sym)->size = NULL;
	}
    }
  else
    {
      symbol_get_obj (sym)->size = xmalloc (sizeof (expressionS));
      *symbol_get_obj (sym)->size = exp;
    }
  demand_empty_rest_of_line ();
}

/* Handle the ELF .type pseudo-op.  This sets the type of a symbol.
   There are five syntaxes:

   The first (used on Solaris) is
       .type SYM,#function
   The second (used on UnixWare) is
       .type SYM,@function
   The third (reportedly to be used on Irix 6.0) is
       .type SYM STT_FUNC
   The fourth (used on NetBSD/Arm and Linux/ARM) is
       .type SYM,%function
   The fifth (used on SVR4/860) is
       .type SYM,"function"
   */

static void
obj_elf_type (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  char c;
  int type;
  const char *typename;
  symbolS *sym;
  elf_symbol_type *elfsym;

  name = input_line_pointer;
  c = get_symbol_end ();
  sym = symbol_find_or_make (name);
  elfsym = (elf_symbol_type *) symbol_get_bfdsym (sym);
  *input_line_pointer = c;

  SKIP_WHITESPACE ();
  if (*input_line_pointer == ',')
    ++input_line_pointer;

  SKIP_WHITESPACE ();
  if (   *input_line_pointer == '#'
      || *input_line_pointer == '@'
      || *input_line_pointer == '"'
      || *input_line_pointer == '%')
    ++input_line_pointer;

  typename = input_line_pointer;
  c = get_symbol_end ();

  type = 0;
  if (strcmp (typename, "function") == 0
      || strcmp (typename, "STT_FUNC") == 0)
    type = BSF_FUNCTION;
  else if (strcmp (typename, "object") == 0
	   || strcmp (typename, "STT_OBJECT") == 0)
    type = BSF_OBJECT;
  else if (strcmp (typename, "tls_object") == 0
	   || strcmp (typename, "STT_TLS") == 0)
    type = BSF_OBJECT | BSF_THREAD_LOCAL;
  else if (strcmp (typename, "notype") == 0
	   || strcmp (typename, "STT_NOTYPE") == 0)
    ;
#ifdef md_elf_symbol_type
  else if ((type = md_elf_symbol_type (typename, sym, elfsym)) != -1)
    ;
#endif
  else
    as_bad (_("unrecognized symbol type \"%s\""), typename);

  *input_line_pointer = c;

  if (*input_line_pointer == '"')
    ++input_line_pointer;

  elfsym->symbol.flags |= type;

  demand_empty_rest_of_line ();
}

static void
obj_elf_ident (int ignore ATTRIBUTE_UNUSED)
{
  static segT comment_section;
  segT old_section = now_seg;
  int old_subsection = now_subseg;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (!comment_section)
    {
      char *p;
      comment_section = subseg_new (".comment", 0);
      bfd_set_section_flags (stdoutput, comment_section,
			     SEC_READONLY | SEC_HAS_CONTENTS);
      p = frag_more (1);
      *p = 0;
    }
  else
    subseg_set (comment_section, 0);
  stringer (1);
  subseg_set (old_section, old_subsection);
}

#ifdef INIT_STAB_SECTION

/* The first entry in a .stabs section is special.  */

void
obj_elf_init_stab_section (segT seg)
{
  char *file;
  char *p;
  char *stabstr_name;
  unsigned int stroff;

  /* Force the section to align to a longword boundary.  Without this,
     UnixWare ar crashes.  */
  (void) bfd_set_section_alignment (stdoutput, seg, 2);

  /* Make space for this first symbol.  */
  p = frag_more (12);
  /* Zero it out.  */
  memset (p, 0, 12);
  as_where (&file, NULL);
  stabstr_name = xmalloc (strlen (segment_name (seg)) + 4);
  strcpy (stabstr_name, segment_name (seg));
  strcat (stabstr_name, "str");
  stroff = get_stab_string_offset (file, stabstr_name);
  know (stroff == 1);
  md_number_to_chars (p, stroff, 4);
  seg_info (seg)->stabu.p = p;
}

#endif

/* Fill in the counts in the first entry in a .stabs section.  */

static void
adjust_stab_sections (bfd *abfd, asection *sec, void *xxx ATTRIBUTE_UNUSED)
{
  char *name;
  asection *strsec;
  char *p;
  int strsz, nsyms;

  if (strncmp (".stab", sec->name, 5))
    return;
  if (!strcmp ("str", sec->name + strlen (sec->name) - 3))
    return;

  name = alloca (strlen (sec->name) + 4);
  strcpy (name, sec->name);
  strcat (name, "str");
  strsec = bfd_get_section_by_name (abfd, name);
  if (strsec)
    strsz = bfd_section_size (abfd, strsec);
  else
    strsz = 0;
  nsyms = bfd_section_size (abfd, sec) / 12 - 1;

  p = seg_info (sec)->stabu.p;
  assert (p != 0);

  bfd_h_put_16 (abfd, nsyms, p + 6);
  bfd_h_put_32 (abfd, strsz, p + 8);
}

#ifdef NEED_ECOFF_DEBUG

/* This function is called by the ECOFF code.  It is supposed to
   record the external symbol information so that the backend can
   write it out correctly.  The ELF backend doesn't actually handle
   this at the moment, so we do it ourselves.  We save the information
   in the symbol.  */

#ifdef OBJ_MAYBE_ELF
static
#endif
void
elf_ecoff_set_ext (symbolS *sym, struct ecoff_extr *ext)
{
  symbol_get_bfdsym (sym)->udata.p = ext;
}

/* This function is called by bfd_ecoff_debug_externals.  It is
   supposed to *EXT to the external symbol information, and return
   whether the symbol should be used at all.  */

static bfd_boolean
elf_get_extr (asymbol *sym, EXTR *ext)
{
  if (sym->udata.p == NULL)
    return FALSE;
  *ext = *(EXTR *) sym->udata.p;
  return TRUE;
}

/* This function is called by bfd_ecoff_debug_externals.  It has
   nothing to do for ELF.  */

static void
elf_set_index (asymbol *sym ATTRIBUTE_UNUSED,
	       bfd_size_type indx ATTRIBUTE_UNUSED)
{
}

#endif /* NEED_ECOFF_DEBUG */

void
elf_frob_symbol (symbolS *symp, int *puntp)
{
  struct elf_obj_sy *sy_obj;

#ifdef NEED_ECOFF_DEBUG
  if (ECOFF_DEBUGGING)
    ecoff_frob_symbol (symp);
#endif

  sy_obj = symbol_get_obj (symp);

  if (sy_obj->size != NULL)
    {
      switch (sy_obj->size->X_op)
	{
	case O_subtract:
	  S_SET_SIZE (symp,
		      (S_GET_VALUE (sy_obj->size->X_add_symbol)
		       + sy_obj->size->X_add_number
		       - S_GET_VALUE (sy_obj->size->X_op_symbol)));
	  break;
	case O_constant:
	  S_SET_SIZE (symp,
		      (S_GET_VALUE (sy_obj->size->X_add_symbol)
		       + sy_obj->size->X_add_number));
	  break;
	default:
	  as_bad (_(".size expression too complicated to fix up"));
	  break;
	}
      free (sy_obj->size);
      sy_obj->size = NULL;
    }

  if (sy_obj->versioned_name != NULL)
    {
      char *p;

      p = strchr (sy_obj->versioned_name, ELF_VER_CHR);
      know (p != NULL);

      /* This symbol was given a new name with the .symver directive.

	 If this is an external reference, just rename the symbol to
	 include the version string.  This will make the relocs be
	 against the correct versioned symbol.

	 If this is a definition, add an alias.  FIXME: Using an alias
	 will permit the debugging information to refer to the right
	 symbol.  However, it's not clear whether it is the best
	 approach.  */

      if (! S_IS_DEFINED (symp))
	{
	  /* Verify that the name isn't using the @@ syntax--this is
	     reserved for definitions of the default version to link
	     against.  */
	  if (p[1] == ELF_VER_CHR)
	    {
	      as_bad (_("invalid attempt to declare external version name as default in symbol `%s'"),
		      sy_obj->versioned_name);
	      *puntp = TRUE;
	    }
	  S_SET_NAME (symp, sy_obj->versioned_name);
	}
      else
	{
	  if (p[1] == ELF_VER_CHR && p[2] == ELF_VER_CHR)
	    {
	      size_t l;

	      /* The @@@ syntax is a special case. It renames the
		 symbol name to versioned_name with one `@' removed.  */
	      l = strlen (&p[3]) + 1;
	      memmove (&p[2], &p[3], l);
	      S_SET_NAME (symp, sy_obj->versioned_name);
	    }
	  else
	    {
	      symbolS *symp2;

	      /* FIXME: Creating a new symbol here is risky.  We're
		 in the final loop over the symbol table.  We can
		 get away with it only because the symbol goes to
		 the end of the list, where the loop will still see
		 it.  It would probably be better to do this in
		 obj_frob_file_before_adjust.  */

	      symp2 = symbol_find_or_make (sy_obj->versioned_name);

	      /* Now we act as though we saw symp2 = sym.  */

	      S_SET_SEGMENT (symp2, S_GET_SEGMENT (symp));

	      /* Subtracting out the frag address here is a hack
		 because we are in the middle of the final loop.  */
	      S_SET_VALUE (symp2,
			   (S_GET_VALUE (symp)
			    - symbol_get_frag (symp)->fr_address));

	      symbol_set_frag (symp2, symbol_get_frag (symp));

	      /* This will copy over the size information.  */
	      copy_symbol_attributes (symp2, symp);

	      S_SET_OTHER (symp2, S_GET_OTHER (symp));

	      if (S_IS_WEAK (symp))
		S_SET_WEAK (symp2);

	      if (S_IS_EXTERNAL (symp))
		S_SET_EXTERNAL (symp2);
	    }
	}
    }

  /* Double check weak symbols.  */
  if (S_IS_WEAK (symp))
    {
      if (S_IS_COMMON (symp))
	as_bad (_("symbol `%s' can not be both weak and common"),
		S_GET_NAME (symp));
    }

#ifdef TC_MIPS
  /* The Irix 5 and 6 assemblers set the type of any common symbol and
     any undefined non-function symbol to STT_OBJECT.  We try to be
     compatible, since newer Irix 5 and 6 linkers care.  However, we
     only set undefined symbols to be STT_OBJECT if we are on Irix,
     because that is the only time gcc will generate the necessary
     .global directives to mark functions.  */

  if (S_IS_COMMON (symp))
    symbol_get_bfdsym (symp)->flags |= BSF_OBJECT;

  if (strstr (TARGET_OS, "irix") != NULL
      && ! S_IS_DEFINED (symp)
      && (symbol_get_bfdsym (symp)->flags & BSF_FUNCTION) == 0)
    symbol_get_bfdsym (symp)->flags |= BSF_OBJECT;
#endif
}

struct group_list
{
  asection **head;		/* Section lists.  */
  unsigned int *elt_count;	/* Number of sections in each list.  */
  unsigned int num_group;	/* Number of lists.  */
};

/* Called via bfd_map_over_sections.  If SEC is a member of a group,
   add it to a list of sections belonging to the group.  INF is a
   pointer to a struct group_list, which is where we store the head of
   each list.  */

static void
build_group_lists (bfd *abfd ATTRIBUTE_UNUSED, asection *sec, void *inf)
{
  struct group_list *list = inf;
  const char *group_name = elf_group_name (sec);
  unsigned int i;

  if (group_name == NULL)
    return;

  /* If this group already has a list, add the section to the head of
     the list.  */
  for (i = 0; i < list->num_group; i++)
    {
      if (strcmp (group_name, elf_group_name (list->head[i])) == 0)
	{
	  elf_next_in_group (sec) = list->head[i];
	  list->head[i] = sec;
	  list->elt_count[i] += 1;
	  return;
	}
    }

  /* New group.  Make the arrays bigger in chunks to minimize calls to
     realloc.  */
  i = list->num_group;
  if ((i & 127) == 0)
    {
      unsigned int newsize = i + 128;
      list->head = xrealloc (list->head, newsize * sizeof (*list->head));
      list->elt_count = xrealloc (list->elt_count,
				  newsize * sizeof (*list->elt_count));
    }
  list->head[i] = sec;
  list->elt_count[i] = 1;
  list->num_group += 1;
}

void
elf_frob_file (void)
{
  struct group_list list;
  unsigned int i;

  bfd_map_over_sections (stdoutput, adjust_stab_sections, NULL);

  /* Go find section groups.  */
  list.num_group = 0;
  list.head = NULL;
  list.elt_count = NULL;
  bfd_map_over_sections (stdoutput, build_group_lists, &list);

  /* Make the SHT_GROUP sections that describe each section group.  We
     can't set up the section contents here yet, because elf section
     indices have yet to be calculated.  elf.c:set_group_contents does
     the rest of the work.  */
  for (i = 0; i < list.num_group; i++)
    {
      const char *group_name = elf_group_name (list.head[i]);
      const char *sec_name;
      asection *s;
      flagword flags;
      struct symbol *sy;
      int has_sym;
      bfd_size_type size;

      flags = SEC_READONLY | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_GROUP;
      for (s = list.head[i]; s != NULL; s = elf_next_in_group (s))
	if ((s->flags ^ flags) & SEC_LINK_ONCE)
	  {
	    flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
	    if (s != list.head[i])
	      {
		as_warn (_("assuming all members of group `%s' are COMDAT"),
			 group_name);
		break;
	      }
	  }

      sec_name = group_name;
      sy = symbol_find_exact (group_name);
      has_sym = 0;
      if (sy != NULL
	  && (sy == symbol_lastP
	      || (sy->sy_next != NULL
		  && sy->sy_next->sy_previous == sy)))
	{
	  has_sym = 1;
	  sec_name = ".group";
	}
      s = subseg_force_new (sec_name, 0);
      if (s == NULL
	  || !bfd_set_section_flags (stdoutput, s, flags)
	  || !bfd_set_section_alignment (stdoutput, s, 2))
	{
	  as_fatal (_("can't create group: %s"),
		    bfd_errmsg (bfd_get_error ()));
	}
      elf_section_type (s) = SHT_GROUP;

      /* Pass a pointer to the first section in this group.  */
      elf_next_in_group (s) = list.head[i];
      if (has_sym)
	elf_group_id (s) = sy->bsym;

      size = 4 * (list.elt_count[i] + 1);
      bfd_set_section_size (stdoutput, s, size);
      s->contents = (unsigned char *) frag_more (size);
      frag_now->fr_fix = frag_now_fix_octets ();
      frag_wane (frag_now);
    }

#ifdef elf_tc_final_processing
  elf_tc_final_processing ();
#endif
}

/* It removes any unneeded versioned symbols from the symbol table.  */

void
elf_frob_file_before_adjust (void)
{
  if (symbol_rootP)
    {
      symbolS *symp;

      for (symp = symbol_rootP; symp; symp = symbol_next (symp))
	if (!S_IS_DEFINED (symp))
	  {
	    if (symbol_get_obj (symp)->versioned_name)
	      {
		char *p;

		/* The @@@ syntax is a special case. If the symbol is
		   not defined, 2 `@'s will be removed from the
		   versioned_name.  */

		p = strchr (symbol_get_obj (symp)->versioned_name,
			    ELF_VER_CHR);
		know (p != NULL);
		if (p[1] == ELF_VER_CHR && p[2] == ELF_VER_CHR)
		  {
		    size_t l = strlen (&p[3]) + 1;
		    memmove (&p[1], &p[3], l);
		  }
		if (symbol_used_p (symp) == 0
		    && symbol_used_in_reloc_p (symp) == 0)
		  symbol_remove (symp, &symbol_rootP, &symbol_lastP);
	      }

	    /* If there was .weak foo, but foo was neither defined nor
	       used anywhere, remove it.  */

	    else if (S_IS_WEAK (symp)
		     && symbol_used_p (symp) == 0
		     && symbol_used_in_reloc_p (symp) == 0)
	      symbol_remove (symp, &symbol_rootP, &symbol_lastP);
	  }
    }
}

/* It is required that we let write_relocs have the opportunity to
   optimize away fixups before output has begun, since it is possible
   to eliminate all fixups for a section and thus we never should
   have generated the relocation section.  */

void
elf_frob_file_after_relocs (void)
{
#ifdef NEED_ECOFF_DEBUG
  if (ECOFF_DEBUGGING)
    /* Generate the ECOFF debugging information.  */
    {
      const struct ecoff_debug_swap *debug_swap;
      struct ecoff_debug_info debug;
      char *buf;
      asection *sec;

      debug_swap
	= get_elf_backend_data (stdoutput)->elf_backend_ecoff_debug_swap;
      know (debug_swap != NULL);
      ecoff_build_debug (&debug.symbolic_header, &buf, debug_swap);

      /* Set up the pointers in debug.  */
#define SET(ptr, offset, type) \
    debug.ptr = (type) (buf + debug.symbolic_header.offset)

      SET (line, cbLineOffset, unsigned char *);
      SET (external_dnr, cbDnOffset, void *);
      SET (external_pdr, cbPdOffset, void *);
      SET (external_sym, cbSymOffset, void *);
      SET (external_opt, cbOptOffset, void *);
      SET (external_aux, cbAuxOffset, union aux_ext *);
      SET (ss, cbSsOffset, char *);
      SET (external_fdr, cbFdOffset, void *);
      SET (external_rfd, cbRfdOffset, void *);
      /* ssext and external_ext are set up just below.  */

#undef SET

      /* Set up the external symbols.  */
      debug.ssext = debug.ssext_end = NULL;
      debug.external_ext = debug.external_ext_end = NULL;
      if (! bfd_ecoff_debug_externals (stdoutput, &debug, debug_swap, TRUE,
				       elf_get_extr, elf_set_index))
	as_fatal (_("failed to set up debugging information: %s"),
		  bfd_errmsg (bfd_get_error ()));

      sec = bfd_get_section_by_name (stdoutput, ".mdebug");
      assert (sec != NULL);

      know (!stdoutput->output_has_begun);

      /* We set the size of the section, call bfd_set_section_contents
	 to force the ELF backend to allocate a file position, and then
	 write out the data.  FIXME: Is this really the best way to do
	 this?  */
      bfd_set_section_size
	(stdoutput, sec, bfd_ecoff_debug_size (stdoutput, &debug, debug_swap));

      /* Pass BUF to bfd_set_section_contents because this will
	 eventually become a call to fwrite, and ISO C prohibits
	 passing a NULL pointer to a stdio function even if the
	 pointer will not be used.  */
      if (! bfd_set_section_contents (stdoutput, sec, buf, 0, 0))
	as_fatal (_("can't start writing .mdebug section: %s"),
		  bfd_errmsg (bfd_get_error ()));

      know (stdoutput->output_has_begun);
      know (sec->filepos != 0);

      if (! bfd_ecoff_write_debug (stdoutput, &debug, debug_swap,
				   sec->filepos))
	as_fatal (_("could not write .mdebug section: %s"),
		  bfd_errmsg (bfd_get_error ()));
    }
#endif /* NEED_ECOFF_DEBUG */
}

#ifdef SCO_ELF

/* Heavily plagiarized from obj_elf_version.  The idea is to emit the
   SCO specific identifier in the .notes section to satisfy the SCO
   linker.

   This looks more complicated than it really is.  As opposed to the
   "obvious" solution, this should handle the cross dev cases
   correctly.  (i.e, hosting on a 64 bit big endian processor, but
   generating SCO Elf code) Efficiency isn't a concern, as there
   should be exactly one of these sections per object module.

   SCO OpenServer 5 identifies it's ELF modules with a standard ELF
   .note section.

   int_32 namesz  = 4 ;  Name size
   int_32 descsz  = 12 ; Descriptive information
   int_32 type    = 1 ;
   char   name[4] = "SCO" ; Originator name ALWAYS SCO + NULL
   int_32 version = (major ver # << 16)  | version of tools ;
   int_32 source  = (tool_id << 16 ) | 1 ;
   int_32 info    = 0 ;    These are set by the SCO tools, but we
			   don't know enough about the source
			   environment to set them.  SCO ld currently
			   ignores them, and recommends we set them
			   to zero.  */

#define SCO_MAJOR_VERSION 0x1
#define SCO_MINOR_VERSION 0x1

void
sco_id (void)
{

  char *name;
  unsigned int c;
  char ch;
  char *p;
  asection *seg = now_seg;
  subsegT subseg = now_subseg;
  Elf_Internal_Note i_note;
  Elf_External_Note e_note;
  asection *note_secp = NULL;
  int i, len;

  /* create the .note section */

  note_secp = subseg_new (".note", 0);
  bfd_set_section_flags (stdoutput,
			 note_secp,
			 SEC_HAS_CONTENTS | SEC_READONLY);

  /* process the version string */

  i_note.namesz = 4;
  i_note.descsz = 12;		/* 12 descriptive bytes */
  i_note.type = NT_VERSION;	/* Contains a version string */

  p = frag_more (sizeof (i_note.namesz));
  md_number_to_chars (p, i_note.namesz, 4);

  p = frag_more (sizeof (i_note.descsz));
  md_number_to_chars (p, i_note.descsz, 4);

  p = frag_more (sizeof (i_note.type));
  md_number_to_chars (p, i_note.type, 4);

  p = frag_more (4);
  strcpy (p, "SCO");

  /* Note: this is the version number of the ELF we're representing */
  p = frag_more (4);
  md_number_to_chars (p, (SCO_MAJOR_VERSION << 16) | (SCO_MINOR_VERSION), 4);

  /* Here, we pick a magic number for ourselves (yes, I "registered"
     it with SCO.  The bottom bit shows that we are compat with the
     SCO ABI.  */
  p = frag_more (4);
  md_number_to_chars (p, 0x4c520000 | 0x0001, 4);

  /* If we knew (or cared) what the source language options were, we'd
     fill them in here.  SCO has given us permission to ignore these
     and just set them to zero.  */
  p = frag_more (4);
  md_number_to_chars (p, 0x0000, 4);

  frag_align (2, 0, 0);

  /* We probably can't restore the current segment, for there likely
     isn't one yet...  */
  if (seg && subseg)
    subseg_set (seg, subseg);

}

#endif /* SCO_ELF */

static int
elf_separate_stab_sections (void)
{
#ifdef NEED_ECOFF_DEBUG
  return (!ECOFF_DEBUGGING);
#else
  return 1;
#endif
}

static void
elf_init_stab_section (segT seg)
{
#ifdef NEED_ECOFF_DEBUG
  if (!ECOFF_DEBUGGING)
#endif
    obj_elf_init_stab_section (seg);
}

const struct format_ops elf_format_ops =
{
  bfd_target_elf_flavour,
  0,	/* dfl_leading_underscore */
  1,	/* emit_section_symbols */
  elf_begin,
  elf_file_symbol,
  elf_frob_symbol,
  elf_frob_file,
  elf_frob_file_before_adjust,
  0,	/* obj_frob_file_before_fix */
  elf_frob_file_after_relocs,
  elf_s_get_size, elf_s_set_size,
  elf_s_get_align, elf_s_set_align,
  elf_s_get_other,
  elf_s_set_other,
  0,	/* s_get_desc */
  0,	/* s_set_desc */
  0,	/* s_get_type */
  0,	/* s_set_type */
  elf_copy_symbol_attributes,
#ifdef NEED_ECOFF_DEBUG
  ecoff_generate_asm_lineno,
  ecoff_stab,
#else
  0,	/* generate_asm_lineno */
  0,	/* process_stab */
#endif
  elf_separate_stab_sections,
  elf_init_stab_section,
  elf_sec_sym_ok_for_reloc,
  elf_pop_insert,
#ifdef NEED_ECOFF_DEBUG
  elf_ecoff_set_ext,
#else
  0,	/* ecoff_set_ext */
#endif
  elf_obj_read_begin_hook,
  elf_obj_symbol_new_hook
};