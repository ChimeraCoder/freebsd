
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

#include "llvm/MC/MCParser/MCAsmParserExtension.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCTargetAsmParser.h"
#include "llvm/Support/COFF.h"
using namespace llvm;

namespace {

class COFFAsmParser : public MCAsmParserExtension {
  template<bool (COFFAsmParser::*HandlerMethod)(StringRef, SMLoc)>
  void addDirectiveHandler(StringRef Directive) {
    MCAsmParser::ExtensionDirectiveHandler Handler = std::make_pair(
        this, HandleDirective<COFFAsmParser, HandlerMethod>);
    getParser().addDirectiveHandler(Directive, Handler);
  }

  bool ParseSectionSwitch(StringRef Section,
                          unsigned Characteristics,
                          SectionKind Kind);

  virtual void Initialize(MCAsmParser &Parser) {
    // Call the base implementation.
    MCAsmParserExtension::Initialize(Parser);

    addDirectiveHandler<&COFFAsmParser::ParseSectionDirectiveText>(".text");
    addDirectiveHandler<&COFFAsmParser::ParseSectionDirectiveData>(".data");
    addDirectiveHandler<&COFFAsmParser::ParseSectionDirectiveBSS>(".bss");
    addDirectiveHandler<&COFFAsmParser::ParseDirectiveDef>(".def");
    addDirectiveHandler<&COFFAsmParser::ParseDirectiveScl>(".scl");
    addDirectiveHandler<&COFFAsmParser::ParseDirectiveType>(".type");
    addDirectiveHandler<&COFFAsmParser::ParseDirectiveEndef>(".endef");
    addDirectiveHandler<&COFFAsmParser::ParseDirectiveSecRel32>(".secrel32");

    // Win64 EH directives.
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveStartProc>(
                                                                   ".seh_proc");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveEndProc>(
                                                                ".seh_endproc");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveStartChained>(
                                                           ".seh_startchained");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveEndChained>(
                                                             ".seh_endchained");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveHandler>(
                                                                ".seh_handler");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveHandlerData>(
                                                            ".seh_handlerdata");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectivePushReg>(
                                                                ".seh_pushreg");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveSetFrame>(
                                                               ".seh_setframe");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveAllocStack>(
                                                             ".seh_stackalloc");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveSaveReg>(
                                                                ".seh_savereg");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveSaveXMM>(
                                                                ".seh_savexmm");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectivePushFrame>(
                                                              ".seh_pushframe");
    addDirectiveHandler<&COFFAsmParser::ParseSEHDirectiveEndProlog>(
                                                            ".seh_endprologue");
    addDirectiveHandler<&COFFAsmParser::ParseDirectiveSymbolAttribute>(".weak");
  }

  bool ParseSectionDirectiveText(StringRef, SMLoc) {
    return ParseSectionSwitch(".text",
                              COFF::IMAGE_SCN_CNT_CODE
                            | COFF::IMAGE_SCN_MEM_EXECUTE
                            | COFF::IMAGE_SCN_MEM_READ,
                              SectionKind::getText());
  }
  bool ParseSectionDirectiveData(StringRef, SMLoc) {
    return ParseSectionSwitch(".data",
                              COFF::IMAGE_SCN_CNT_INITIALIZED_DATA
                            | COFF::IMAGE_SCN_MEM_READ
                            | COFF::IMAGE_SCN_MEM_WRITE,
                              SectionKind::getDataRel());
  }
  bool ParseSectionDirectiveBSS(StringRef, SMLoc) {
    return ParseSectionSwitch(".bss",
                              COFF::IMAGE_SCN_CNT_UNINITIALIZED_DATA
                            | COFF::IMAGE_SCN_MEM_READ
                            | COFF::IMAGE_SCN_MEM_WRITE,
                              SectionKind::getBSS());
  }

  bool ParseDirectiveDef(StringRef, SMLoc);
  bool ParseDirectiveScl(StringRef, SMLoc);
  bool ParseDirectiveType(StringRef, SMLoc);
  bool ParseDirectiveEndef(StringRef, SMLoc);
  bool ParseDirectiveSecRel32(StringRef, SMLoc);

  // Win64 EH directives.
  bool ParseSEHDirectiveStartProc(StringRef, SMLoc);
  bool ParseSEHDirectiveEndProc(StringRef, SMLoc);
  bool ParseSEHDirectiveStartChained(StringRef, SMLoc);
  bool ParseSEHDirectiveEndChained(StringRef, SMLoc);
  bool ParseSEHDirectiveHandler(StringRef, SMLoc);
  bool ParseSEHDirectiveHandlerData(StringRef, SMLoc);
  bool ParseSEHDirectivePushReg(StringRef, SMLoc);
  bool ParseSEHDirectiveSetFrame(StringRef, SMLoc);
  bool ParseSEHDirectiveAllocStack(StringRef, SMLoc);
  bool ParseSEHDirectiveSaveReg(StringRef, SMLoc);
  bool ParseSEHDirectiveSaveXMM(StringRef, SMLoc);
  bool ParseSEHDirectivePushFrame(StringRef, SMLoc);
  bool ParseSEHDirectiveEndProlog(StringRef, SMLoc);

  bool ParseAtUnwindOrAtExcept(bool &unwind, bool &except);
  bool ParseSEHRegisterNumber(unsigned &RegNo);
  bool ParseDirectiveSymbolAttribute(StringRef Directive, SMLoc);
public:
  COFFAsmParser() {}
};

} // end annonomous namespace.

/// ParseDirectiveSymbolAttribute
///  ::= { ".weak", ... } [ identifier ( , identifier )* ]
bool COFFAsmParser::ParseDirectiveSymbolAttribute(StringRef Directive, SMLoc) {
  MCSymbolAttr Attr = StringSwitch<MCSymbolAttr>(Directive)
    .Case(".weak", MCSA_Weak)
    .Default(MCSA_Invalid);
  assert(Attr != MCSA_Invalid && "unexpected symbol attribute directive!");
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    for (;;) {
      StringRef Name;

      if (getParser().parseIdentifier(Name))
        return TokError("expected identifier in directive");

      MCSymbol *Sym = getContext().GetOrCreateSymbol(Name);

      getStreamer().EmitSymbolAttribute(Sym, Attr);

      if (getLexer().is(AsmToken::EndOfStatement))
        break;

      if (getLexer().isNot(AsmToken::Comma))
        return TokError("unexpected token in directive");
      Lex();
    }
  }

  Lex();
  return false;
}

bool COFFAsmParser::ParseSectionSwitch(StringRef Section,
                                       unsigned Characteristics,
                                       SectionKind Kind) {
  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in section switching directive");
  Lex();

  getStreamer().SwitchSection(getContext().getCOFFSection(
                                Section, Characteristics, Kind));

  return false;
}

bool COFFAsmParser::ParseDirectiveDef(StringRef, SMLoc) {
  StringRef SymbolName;

  if (getParser().parseIdentifier(SymbolName))
    return TokError("expected identifier in directive");

  MCSymbol *Sym = getContext().GetOrCreateSymbol(SymbolName);

  getStreamer().BeginCOFFSymbolDef(Sym);

  Lex();
  return false;
}

bool COFFAsmParser::ParseDirectiveScl(StringRef, SMLoc) {
  int64_t SymbolStorageClass;
  if (getParser().parseAbsoluteExpression(SymbolStorageClass))
    return true;

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  Lex();
  getStreamer().EmitCOFFSymbolStorageClass(SymbolStorageClass);
  return false;
}

bool COFFAsmParser::ParseDirectiveType(StringRef, SMLoc) {
  int64_t Type;
  if (getParser().parseAbsoluteExpression(Type))
    return true;

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  Lex();
  getStreamer().EmitCOFFSymbolType(Type);
  return false;
}

bool COFFAsmParser::ParseDirectiveEndef(StringRef, SMLoc) {
  Lex();
  getStreamer().EndCOFFSymbolDef();
  return false;
}

bool COFFAsmParser::ParseDirectiveSecRel32(StringRef, SMLoc) {
  StringRef SymbolID;
  if (getParser().parseIdentifier(SymbolID))
    return true;

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  MCSymbol *Symbol = getContext().GetOrCreateSymbol(SymbolID);

  Lex();
  getStreamer().EmitCOFFSecRel32(Symbol);
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveStartProc(StringRef, SMLoc) {
  StringRef SymbolID;
  if (getParser().parseIdentifier(SymbolID))
    return true;

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  MCSymbol *Symbol = getContext().GetOrCreateSymbol(SymbolID);

  Lex();
  getStreamer().EmitWin64EHStartProc(Symbol);
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveEndProc(StringRef, SMLoc) {
  Lex();
  getStreamer().EmitWin64EHEndProc();
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveStartChained(StringRef, SMLoc) {
  Lex();
  getStreamer().EmitWin64EHStartChained();
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveEndChained(StringRef, SMLoc) {
  Lex();
  getStreamer().EmitWin64EHEndChained();
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveHandler(StringRef, SMLoc) {
  StringRef SymbolID;
  if (getParser().parseIdentifier(SymbolID))
    return true;

  if (getLexer().isNot(AsmToken::Comma))
    return TokError("you must specify one or both of @unwind or @except");
  Lex();
  bool unwind = false, except = false;
  if (ParseAtUnwindOrAtExcept(unwind, except))
    return true;
  if (getLexer().is(AsmToken::Comma)) {
    Lex();
    if (ParseAtUnwindOrAtExcept(unwind, except))
      return true;
  }
  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  MCSymbol *handler = getContext().GetOrCreateSymbol(SymbolID);

  Lex();
  getStreamer().EmitWin64EHHandler(handler, unwind, except);
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveHandlerData(StringRef, SMLoc) {
  Lex();
  getStreamer().EmitWin64EHHandlerData();
  return false;
}

bool COFFAsmParser::ParseSEHDirectivePushReg(StringRef, SMLoc L) {
  unsigned Reg;
  if (ParseSEHRegisterNumber(Reg))
    return true;

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  Lex();
  getStreamer().EmitWin64EHPushReg(Reg);
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveSetFrame(StringRef, SMLoc L) {
  unsigned Reg;
  int64_t Off;
  if (ParseSEHRegisterNumber(Reg))
    return true;
  if (getLexer().isNot(AsmToken::Comma))
    return TokError("you must specify a stack pointer offset");

  Lex();
  SMLoc startLoc = getLexer().getLoc();
  if (getParser().parseAbsoluteExpression(Off))
    return true;

  if (Off & 0x0F)
    return Error(startLoc, "offset is not a multiple of 16");

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  Lex();
  getStreamer().EmitWin64EHSetFrame(Reg, Off);
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveAllocStack(StringRef, SMLoc) {
  int64_t Size;
  SMLoc startLoc = getLexer().getLoc();
  if (getParser().parseAbsoluteExpression(Size))
    return true;

  if (Size & 7)
    return Error(startLoc, "size is not a multiple of 8");

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  Lex();
  getStreamer().EmitWin64EHAllocStack(Size);
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveSaveReg(StringRef, SMLoc L) {
  unsigned Reg;
  int64_t Off;
  if (ParseSEHRegisterNumber(Reg))
    return true;
  if (getLexer().isNot(AsmToken::Comma))
    return TokError("you must specify an offset on the stack");

  Lex();
  SMLoc startLoc = getLexer().getLoc();
  if (getParser().parseAbsoluteExpression(Off))
    return true;

  if (Off & 7)
    return Error(startLoc, "size is not a multiple of 8");

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  Lex();
  // FIXME: Err on %xmm* registers
  getStreamer().EmitWin64EHSaveReg(Reg, Off);
  return false;
}

// FIXME: This method is inherently x86-specific. It should really be in the
// x86 backend.
bool COFFAsmParser::ParseSEHDirectiveSaveXMM(StringRef, SMLoc L) {
  unsigned Reg;
  int64_t Off;
  if (ParseSEHRegisterNumber(Reg))
    return true;
  if (getLexer().isNot(AsmToken::Comma))
    return TokError("you must specify an offset on the stack");

  Lex();
  SMLoc startLoc = getLexer().getLoc();
  if (getParser().parseAbsoluteExpression(Off))
    return true;

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  if (Off & 0x0F)
    return Error(startLoc, "offset is not a multiple of 16");

  Lex();
  // FIXME: Err on non-%xmm* registers
  getStreamer().EmitWin64EHSaveXMM(Reg, Off);
  return false;
}

bool COFFAsmParser::ParseSEHDirectivePushFrame(StringRef, SMLoc) {
  bool Code = false;
  StringRef CodeID;
  if (getLexer().is(AsmToken::At)) {
    SMLoc startLoc = getLexer().getLoc();
    Lex();
    if (!getParser().parseIdentifier(CodeID)) {
      if (CodeID != "code")
        return Error(startLoc, "expected @code");
      Code = true;
    }
  }

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  Lex();
  getStreamer().EmitWin64EHPushFrame(Code);
  return false;
}

bool COFFAsmParser::ParseSEHDirectiveEndProlog(StringRef, SMLoc) {
  Lex();
  getStreamer().EmitWin64EHEndProlog();
  return false;
}

bool COFFAsmParser::ParseAtUnwindOrAtExcept(bool &unwind, bool &except) {
  StringRef identifier;
  if (getLexer().isNot(AsmToken::At))
    return TokError("a handler attribute must begin with '@'");
  SMLoc startLoc = getLexer().getLoc();
  Lex();
  if (getParser().parseIdentifier(identifier))
    return Error(startLoc, "expected @unwind or @except");
  if (identifier == "unwind")
    unwind = true;
  else if (identifier == "except")
    except = true;
  else
    return Error(startLoc, "expected @unwind or @except");
  return false;
}

bool COFFAsmParser::ParseSEHRegisterNumber(unsigned &RegNo) {
  SMLoc startLoc = getLexer().getLoc();
  if (getLexer().is(AsmToken::Percent)) {
    const MCRegisterInfo &MRI = getContext().getRegisterInfo();
    SMLoc endLoc;
    unsigned LLVMRegNo;
    if (getParser().getTargetParser().ParseRegister(LLVMRegNo,startLoc,endLoc))
      return true;

#if 0
    // FIXME: TargetAsmInfo::getCalleeSavedRegs() commits a serious layering
    // violation so this validation code is disabled.

    // Check that this is a non-volatile register.
    const unsigned *NVRegs = TAI.getCalleeSavedRegs();
    unsigned i;
    for (i = 0; NVRegs[i] != 0; ++i)
      if (NVRegs[i] == LLVMRegNo)
        break;
    if (NVRegs[i] == 0)
      return Error(startLoc, "expected non-volatile register");
#endif

    int SEHRegNo = MRI.getSEHRegNum(LLVMRegNo);
    if (SEHRegNo < 0)
      return Error(startLoc,"register can't be represented in SEH unwind info");
    RegNo = SEHRegNo;
  }
  else {
    int64_t n;
    if (getParser().parseAbsoluteExpression(n))
      return true;
    if (n > 15)
      return Error(startLoc, "register number is too high");
    RegNo = n;
  }

  return false;
}

namespace llvm {

MCAsmParserExtension *createCOFFAsmParser() {
  return new COFFAsmParser;
}

}