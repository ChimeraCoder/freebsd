
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

#include "llvm/Object/ObjectFile.h"
#include "llvm-c/Object.h"

using namespace llvm;
using namespace object;

// ObjectFile creation
LLVMObjectFileRef LLVMCreateObjectFile(LLVMMemoryBufferRef MemBuf) {
  return wrap(ObjectFile::createObjectFile(unwrap(MemBuf)));
}

void LLVMDisposeObjectFile(LLVMObjectFileRef ObjectFile) {
  delete unwrap(ObjectFile);
}

// ObjectFile Section iterators
LLVMSectionIteratorRef LLVMGetSections(LLVMObjectFileRef ObjectFile) {
  section_iterator SI = unwrap(ObjectFile)->begin_sections();
  return wrap(new section_iterator(SI));
}

void LLVMDisposeSectionIterator(LLVMSectionIteratorRef SI) {
  delete unwrap(SI);
}

LLVMBool LLVMIsSectionIteratorAtEnd(LLVMObjectFileRef ObjectFile,
                                LLVMSectionIteratorRef SI) {
  return (*unwrap(SI) == unwrap(ObjectFile)->end_sections()) ? 1 : 0;
}

void LLVMMoveToNextSection(LLVMSectionIteratorRef SI) {
  error_code ec;
  unwrap(SI)->increment(ec);
  if (ec) report_fatal_error("LLVMMoveToNextSection failed: " + ec.message());
}

void LLVMMoveToContainingSection(LLVMSectionIteratorRef Sect,
                                 LLVMSymbolIteratorRef Sym) {
  if (error_code ec = (*unwrap(Sym))->getSection(*unwrap(Sect)))
    report_fatal_error(ec.message());
}

// ObjectFile Symbol iterators
LLVMSymbolIteratorRef LLVMGetSymbols(LLVMObjectFileRef ObjectFile) {
  symbol_iterator SI = unwrap(ObjectFile)->begin_symbols();
  return wrap(new symbol_iterator(SI));
}

void LLVMDisposeSymbolIterator(LLVMSymbolIteratorRef SI) {
  delete unwrap(SI);
}

LLVMBool LLVMIsSymbolIteratorAtEnd(LLVMObjectFileRef ObjectFile,
                                LLVMSymbolIteratorRef SI) {
  return (*unwrap(SI) == unwrap(ObjectFile)->end_symbols()) ? 1 : 0;
}

void LLVMMoveToNextSymbol(LLVMSymbolIteratorRef SI) {
  error_code ec;
  unwrap(SI)->increment(ec);
  if (ec) report_fatal_error("LLVMMoveToNextSymbol failed: " + ec.message());
}

// SectionRef accessors
const char *LLVMGetSectionName(LLVMSectionIteratorRef SI) {
  StringRef ret;
  if (error_code ec = (*unwrap(SI))->getName(ret))
   report_fatal_error(ec.message());
  return ret.data();
}

uint64_t LLVMGetSectionSize(LLVMSectionIteratorRef SI) {
  uint64_t ret;
  if (error_code ec = (*unwrap(SI))->getSize(ret))
    report_fatal_error(ec.message());
  return ret;
}

const char *LLVMGetSectionContents(LLVMSectionIteratorRef SI) {
  StringRef ret;
  if (error_code ec = (*unwrap(SI))->getContents(ret))
    report_fatal_error(ec.message());
  return ret.data();
}

uint64_t LLVMGetSectionAddress(LLVMSectionIteratorRef SI) {
  uint64_t ret;
  if (error_code ec = (*unwrap(SI))->getAddress(ret))
    report_fatal_error(ec.message());
  return ret;
}

LLVMBool LLVMGetSectionContainsSymbol(LLVMSectionIteratorRef SI,
                                 LLVMSymbolIteratorRef Sym) {
  bool ret;
  if (error_code ec = (*unwrap(SI))->containsSymbol(**unwrap(Sym), ret))
    report_fatal_error(ec.message());
  return ret;
}

// Section Relocation iterators
LLVMRelocationIteratorRef LLVMGetRelocations(LLVMSectionIteratorRef Section) {
  relocation_iterator SI = (*unwrap(Section))->begin_relocations();
  return wrap(new relocation_iterator(SI));
}

void LLVMDisposeRelocationIterator(LLVMRelocationIteratorRef SI) {
  delete unwrap(SI);
}

LLVMBool LLVMIsRelocationIteratorAtEnd(LLVMSectionIteratorRef Section,
                                       LLVMRelocationIteratorRef SI) {
  return (*unwrap(SI) == (*unwrap(Section))->end_relocations()) ? 1 : 0;
}

void LLVMMoveToNextRelocation(LLVMRelocationIteratorRef SI) {
  error_code ec;
  unwrap(SI)->increment(ec);
  if (ec) report_fatal_error("LLVMMoveToNextRelocation failed: " +
                             ec.message());
}


// SymbolRef accessors
const char *LLVMGetSymbolName(LLVMSymbolIteratorRef SI) {
  StringRef ret;
  if (error_code ec = (*unwrap(SI))->getName(ret))
    report_fatal_error(ec.message());
  return ret.data();
}

uint64_t LLVMGetSymbolAddress(LLVMSymbolIteratorRef SI) {
  uint64_t ret;
  if (error_code ec = (*unwrap(SI))->getAddress(ret))
    report_fatal_error(ec.message());
  return ret;
}

uint64_t LLVMGetSymbolFileOffset(LLVMSymbolIteratorRef SI) {
  uint64_t ret;
  if (error_code ec = (*unwrap(SI))->getFileOffset(ret))
    report_fatal_error(ec.message());
  return ret;
}

uint64_t LLVMGetSymbolSize(LLVMSymbolIteratorRef SI) {
  uint64_t ret;
  if (error_code ec = (*unwrap(SI))->getSize(ret))
    report_fatal_error(ec.message());
  return ret;
}

// RelocationRef accessors
uint64_t LLVMGetRelocationAddress(LLVMRelocationIteratorRef RI) {
  uint64_t ret;
  if (error_code ec = (*unwrap(RI))->getAddress(ret))
    report_fatal_error(ec.message());
  return ret;
}

uint64_t LLVMGetRelocationOffset(LLVMRelocationIteratorRef RI) {
  uint64_t ret;
  if (error_code ec = (*unwrap(RI))->getOffset(ret))
    report_fatal_error(ec.message());
  return ret;
}

LLVMSymbolIteratorRef LLVMGetRelocationSymbol(LLVMRelocationIteratorRef RI) {
  SymbolRef ret;
  if (error_code ec = (*unwrap(RI))->getSymbol(ret))
    report_fatal_error(ec.message());

  return wrap(new symbol_iterator(ret));
}

uint64_t LLVMGetRelocationType(LLVMRelocationIteratorRef RI) {
  uint64_t ret;
  if (error_code ec = (*unwrap(RI))->getType(ret))
    report_fatal_error(ec.message());
  return ret;
}

// NOTE: Caller takes ownership of returned string.
const char *LLVMGetRelocationTypeName(LLVMRelocationIteratorRef RI) {
  SmallVector<char, 0> ret;
  if (error_code ec = (*unwrap(RI))->getTypeName(ret))
    report_fatal_error(ec.message());

  char *str = static_cast<char*>(malloc(ret.size()));
  std::copy(ret.begin(), ret.end(), str);
  return str;
}

// NOTE: Caller takes ownership of returned string.
const char *LLVMGetRelocationValueString(LLVMRelocationIteratorRef RI) {
  SmallVector<char, 0> ret;
  if (error_code ec = (*unwrap(RI))->getValueString(ret))
    report_fatal_error(ec.message());

  char *str = static_cast<char*>(malloc(ret.size()));
  std::copy(ret.begin(), ret.end(), str);
  return str;
}