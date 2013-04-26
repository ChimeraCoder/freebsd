
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

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <string>
using namespace llvm;

enum OpKind {
  OpNone,
  OpUnavailable,
  OpAdd,
  OpAddl,
  OpAddw,
  OpSub,
  OpSubl,
  OpSubw,
  OpMul,
  OpMla,
  OpMlal,
  OpMls,
  OpMlsl,
  OpMulN,
  OpMlaN,
  OpMlsN,
  OpMlalN,
  OpMlslN,
  OpMulLane,
  OpMullLane,
  OpMlaLane,
  OpMlsLane,
  OpMlalLane,
  OpMlslLane,
  OpQDMullLane,
  OpQDMlalLane,
  OpQDMlslLane,
  OpQDMulhLane,
  OpQRDMulhLane,
  OpEq,
  OpGe,
  OpLe,
  OpGt,
  OpLt,
  OpNeg,
  OpNot,
  OpAnd,
  OpOr,
  OpXor,
  OpAndNot,
  OpOrNot,
  OpCast,
  OpConcat,
  OpDup,
  OpDupLane,
  OpHi,
  OpLo,
  OpSelect,
  OpRev16,
  OpRev32,
  OpRev64,
  OpReinterpret,
  OpAbdl,
  OpAba,
  OpAbal
};

enum ClassKind {
  ClassNone,
  ClassI,           // generic integer instruction, e.g., "i8" suffix
  ClassS,           // signed/unsigned/poly, e.g., "s8", "u8" or "p8" suffix
  ClassW,           // width-specific instruction, e.g., "8" suffix
  ClassB            // bitcast arguments with enum argument to specify type
};

/// NeonTypeFlags - Flags to identify the types for overloaded Neon
/// builtins.  These must be kept in sync with the flags in
/// include/clang/Basic/TargetBuiltins.h.
namespace {
class NeonTypeFlags {
  enum {
    EltTypeMask = 0xf,
    UnsignedFlag = 0x10,
    QuadFlag = 0x20
  };
  uint32_t Flags;

public:
  enum EltType {
    Int8,
    Int16,
    Int32,
    Int64,
    Poly8,
    Poly16,
    Float16,
    Float32
  };

  NeonTypeFlags(unsigned F) : Flags(F) {}
  NeonTypeFlags(EltType ET, bool IsUnsigned, bool IsQuad) : Flags(ET) {
    if (IsUnsigned)
      Flags |= UnsignedFlag;
    if (IsQuad)
      Flags |= QuadFlag;
  }

  uint32_t getFlags() const { return Flags; }
};
} // end anonymous namespace

namespace {
class NeonEmitter {
  RecordKeeper &Records;
  StringMap<OpKind> OpMap;
  DenseMap<Record*, ClassKind> ClassMap;

public:
  NeonEmitter(RecordKeeper &R) : Records(R) {
    OpMap["OP_NONE"]  = OpNone;
    OpMap["OP_UNAVAILABLE"] = OpUnavailable;
    OpMap["OP_ADD"]   = OpAdd;
    OpMap["OP_ADDL"]  = OpAddl;
    OpMap["OP_ADDW"]  = OpAddw;
    OpMap["OP_SUB"]   = OpSub;
    OpMap["OP_SUBL"]  = OpSubl;
    OpMap["OP_SUBW"]  = OpSubw;
    OpMap["OP_MUL"]   = OpMul;
    OpMap["OP_MLA"]   = OpMla;
    OpMap["OP_MLAL"]  = OpMlal;
    OpMap["OP_MLS"]   = OpMls;
    OpMap["OP_MLSL"]  = OpMlsl;
    OpMap["OP_MUL_N"] = OpMulN;
    OpMap["OP_MLA_N"] = OpMlaN;
    OpMap["OP_MLS_N"] = OpMlsN;
    OpMap["OP_MLAL_N"] = OpMlalN;
    OpMap["OP_MLSL_N"] = OpMlslN;
    OpMap["OP_MUL_LN"]= OpMulLane;
    OpMap["OP_MULL_LN"] = OpMullLane;
    OpMap["OP_MLA_LN"]= OpMlaLane;
    OpMap["OP_MLS_LN"]= OpMlsLane;
    OpMap["OP_MLAL_LN"] = OpMlalLane;
    OpMap["OP_MLSL_LN"] = OpMlslLane;
    OpMap["OP_QDMULL_LN"] = OpQDMullLane;
    OpMap["OP_QDMLAL_LN"] = OpQDMlalLane;
    OpMap["OP_QDMLSL_LN"] = OpQDMlslLane;
    OpMap["OP_QDMULH_LN"] = OpQDMulhLane;
    OpMap["OP_QRDMULH_LN"] = OpQRDMulhLane;
    OpMap["OP_EQ"]    = OpEq;
    OpMap["OP_GE"]    = OpGe;
    OpMap["OP_LE"]    = OpLe;
    OpMap["OP_GT"]    = OpGt;
    OpMap["OP_LT"]    = OpLt;
    OpMap["OP_NEG"]   = OpNeg;
    OpMap["OP_NOT"]   = OpNot;
    OpMap["OP_AND"]   = OpAnd;
    OpMap["OP_OR"]    = OpOr;
    OpMap["OP_XOR"]   = OpXor;
    OpMap["OP_ANDN"]  = OpAndNot;
    OpMap["OP_ORN"]   = OpOrNot;
    OpMap["OP_CAST"]  = OpCast;
    OpMap["OP_CONC"]  = OpConcat;
    OpMap["OP_HI"]    = OpHi;
    OpMap["OP_LO"]    = OpLo;
    OpMap["OP_DUP"]   = OpDup;
    OpMap["OP_DUP_LN"] = OpDupLane;
    OpMap["OP_SEL"]   = OpSelect;
    OpMap["OP_REV16"] = OpRev16;
    OpMap["OP_REV32"] = OpRev32;
    OpMap["OP_REV64"] = OpRev64;
    OpMap["OP_REINT"] = OpReinterpret;
    OpMap["OP_ABDL"]  = OpAbdl;
    OpMap["OP_ABA"]   = OpAba;
    OpMap["OP_ABAL"]  = OpAbal;

    Record *SI = R.getClass("SInst");
    Record *II = R.getClass("IInst");
    Record *WI = R.getClass("WInst");
    ClassMap[SI] = ClassS;
    ClassMap[II] = ClassI;
    ClassMap[WI] = ClassW;
  }

  // run - Emit arm_neon.h.inc
  void run(raw_ostream &o);

  // runHeader - Emit all the __builtin prototypes used in arm_neon.h
  void runHeader(raw_ostream &o);

  // runTests - Emit tests for all the Neon intrinsics.
  void runTests(raw_ostream &o);

private:
  void emitIntrinsic(raw_ostream &OS, Record *R);
};
} // end anonymous namespace

/// ParseTypes - break down a string such as "fQf" into a vector of StringRefs,
/// which each StringRef representing a single type declared in the string.
/// for "fQf" we would end up with 2 StringRefs, "f", and "Qf", representing
/// 2xfloat and 4xfloat respectively.
static void ParseTypes(Record *r, std::string &s,
                       SmallVectorImpl<StringRef> &TV) {
  const char *data = s.data();
  int len = 0;

  for (unsigned i = 0, e = s.size(); i != e; ++i, ++len) {
    if (data[len] == 'P' || data[len] == 'Q' || data[len] == 'U')
      continue;

    switch (data[len]) {
      case 'c':
      case 's':
      case 'i':
      case 'l':
      case 'h':
      case 'f':
        break;
      default:
        PrintFatalError(r->getLoc(),
                      "Unexpected letter: " + std::string(data + len, 1));
    }
    TV.push_back(StringRef(data, len + 1));
    data += len + 1;
    len = -1;
  }
}

/// Widen - Convert a type code into the next wider type.  char -> short,
/// short -> int, etc.
static char Widen(const char t) {
  switch (t) {
    case 'c':
      return 's';
    case 's':
      return 'i';
    case 'i':
      return 'l';
    case 'h':
      return 'f';
    default:
      PrintFatalError("unhandled type in widen!");
  }
}

/// Narrow - Convert a type code into the next smaller type.  short -> char,
/// float -> half float, etc.
static char Narrow(const char t) {
  switch (t) {
    case 's':
      return 'c';
    case 'i':
      return 's';
    case 'l':
      return 'i';
    case 'f':
      return 'h';
    default:
      PrintFatalError("unhandled type in narrow!");
  }
}

/// For a particular StringRef, return the base type code, and whether it has
/// the quad-vector, polynomial, or unsigned modifiers set.
static char ClassifyType(StringRef ty, bool &quad, bool &poly, bool &usgn) {
  unsigned off = 0;

  // remember quad.
  if (ty[off] == 'Q') {
    quad = true;
    ++off;
  }

  // remember poly.
  if (ty[off] == 'P') {
    poly = true;
    ++off;
  }

  // remember unsigned.
  if (ty[off] == 'U') {
    usgn = true;
    ++off;
  }

  // base type to get the type string for.
  return ty[off];
}

/// ModType - Transform a type code and its modifiers based on a mod code. The
/// mod code definitions may be found at the top of arm_neon.td.
static char ModType(const char mod, char type, bool &quad, bool &poly,
                    bool &usgn, bool &scal, bool &cnst, bool &pntr) {
  switch (mod) {
    case 't':
      if (poly) {
        poly = false;
        usgn = true;
      }
      break;
    case 'u':
      usgn = true;
      poly = false;
      if (type == 'f')
        type = 'i';
      break;
    case 'x':
      usgn = false;
      poly = false;
      if (type == 'f')
        type = 'i';
      break;
    case 'f':
      if (type == 'h')
        quad = true;
      type = 'f';
      usgn = false;
      break;
    case 'g':
      quad = false;
      break;
    case 'w':
      type = Widen(type);
      quad = true;
      break;
    case 'n':
      type = Widen(type);
      break;
    case 'i':
      type = 'i';
      scal = true;
      break;
    case 'l':
      type = 'l';
      scal = true;
      usgn = true;
      break;
    case 's':
    case 'a':
      scal = true;
      break;
    case 'k':
      quad = true;
      break;
    case 'c':
      cnst = true;
    case 'p':
      pntr = true;
      scal = true;
      break;
    case 'h':
      type = Narrow(type);
      if (type == 'h')
        quad = false;
      break;
    case 'e':
      type = Narrow(type);
      usgn = true;
      break;
    default:
      break;
  }
  return type;
}

/// TypeString - for a modifier and type, generate the name of the typedef for
/// that type.  QUc -> uint8x8_t.
static std::string TypeString(const char mod, StringRef typestr) {
  bool quad = false;
  bool poly = false;
  bool usgn = false;
  bool scal = false;
  bool cnst = false;
  bool pntr = false;

  if (mod == 'v')
    return "void";
  if (mod == 'i')
    return "int";

  // base type to get the type string for.
  char type = ClassifyType(typestr, quad, poly, usgn);

  // Based on the modifying character, change the type and width if necessary.
  type = ModType(mod, type, quad, poly, usgn, scal, cnst, pntr);

  SmallString<128> s;

  if (usgn)
    s.push_back('u');

  switch (type) {
    case 'c':
      s += poly ? "poly8" : "int8";
      if (scal)
        break;
      s += quad ? "x16" : "x8";
      break;
    case 's':
      s += poly ? "poly16" : "int16";
      if (scal)
        break;
      s += quad ? "x8" : "x4";
      break;
    case 'i':
      s += "int32";
      if (scal)
        break;
      s += quad ? "x4" : "x2";
      break;
    case 'l':
      s += "int64";
      if (scal)
        break;
      s += quad ? "x2" : "x1";
      break;
    case 'h':
      s += "float16";
      if (scal)
        break;
      s += quad ? "x8" : "x4";
      break;
    case 'f':
      s += "float32";
      if (scal)
        break;
      s += quad ? "x4" : "x2";
      break;
    default:
      PrintFatalError("unhandled type!");
  }

  if (mod == '2')
    s += "x2";
  if (mod == '3')
    s += "x3";
  if (mod == '4')
    s += "x4";

  // Append _t, finishing the type string typedef type.
  s += "_t";

  if (cnst)
    s += " const";

  if (pntr)
    s += " *";

  return s.str();
}

/// BuiltinTypeString - for a modifier and type, generate the clang
/// BuiltinsARM.def prototype code for the function.  See the top of clang's
/// Builtins.def for a description of the type strings.
static std::string BuiltinTypeString(const char mod, StringRef typestr,
                                     ClassKind ck, bool ret) {
  bool quad = false;
  bool poly = false;
  bool usgn = false;
  bool scal = false;
  bool cnst = false;
  bool pntr = false;

  if (mod == 'v')
    return "v"; // void
  if (mod == 'i')
    return "i"; // int

  // base type to get the type string for.
  char type = ClassifyType(typestr, quad, poly, usgn);

  // Based on the modifying character, change the type and width if necessary.
  type = ModType(mod, type, quad, poly, usgn, scal, cnst, pntr);

  // All pointers are void* pointers.  Change type to 'v' now.
  if (pntr) {
    usgn = false;
    poly = false;
    type = 'v';
  }
  // Treat half-float ('h') types as unsigned short ('s') types.
  if (type == 'h') {
    type = 's';
    usgn = true;
  }
  usgn = usgn | poly | ((ck == ClassI || ck == ClassW) && scal && type != 'f');

  if (scal) {
    SmallString<128> s;

    if (usgn)
      s.push_back('U');
    else if (type == 'c')
      s.push_back('S'); // make chars explicitly signed

    if (type == 'l') // 64-bit long
      s += "LLi";
    else
      s.push_back(type);

    if (cnst)
      s.push_back('C');
    if (pntr)
      s.push_back('*');
    return s.str();
  }

  // Since the return value must be one type, return a vector type of the
  // appropriate width which we will bitcast.  An exception is made for
  // returning structs of 2, 3, or 4 vectors which are returned in a sret-like
  // fashion, storing them to a pointer arg.
  if (ret) {
    if (mod >= '2' && mod <= '4')
      return "vv*"; // void result with void* first argument
    if (mod == 'f' || (ck != ClassB && type == 'f'))
      return quad ? "V4f" : "V2f";
    if (ck != ClassB && type == 's')
      return quad ? "V8s" : "V4s";
    if (ck != ClassB && type == 'i')
      return quad ? "V4i" : "V2i";
    if (ck != ClassB && type == 'l')
      return quad ? "V2LLi" : "V1LLi";

    return quad ? "V16Sc" : "V8Sc";
  }

  // Non-return array types are passed as individual vectors.
  if (mod == '2')
    return quad ? "V16ScV16Sc" : "V8ScV8Sc";
  if (mod == '3')
    return quad ? "V16ScV16ScV16Sc" : "V8ScV8ScV8Sc";
  if (mod == '4')
    return quad ? "V16ScV16ScV16ScV16Sc" : "V8ScV8ScV8ScV8Sc";

  if (mod == 'f' || (ck != ClassB && type == 'f'))
    return quad ? "V4f" : "V2f";
  if (ck != ClassB && type == 's')
    return quad ? "V8s" : "V4s";
  if (ck != ClassB && type == 'i')
    return quad ? "V4i" : "V2i";
  if (ck != ClassB && type == 'l')
    return quad ? "V2LLi" : "V1LLi";

  return quad ? "V16Sc" : "V8Sc";
}

/// MangleName - Append a type or width suffix to a base neon function name,
/// and insert a 'q' in the appropriate location if the operation works on
/// 128b rather than 64b.   E.g. turn "vst2_lane" into "vst2q_lane_f32", etc.
static std::string MangleName(const std::string &name, StringRef typestr,
                              ClassKind ck) {
  if (name == "vcvt_f32_f16")
    return name;

  bool quad = false;
  bool poly = false;
  bool usgn = false;
  char type = ClassifyType(typestr, quad, poly, usgn);

  std::string s = name;

  switch (type) {
  case 'c':
    switch (ck) {
    case ClassS: s += poly ? "_p8" : usgn ? "_u8" : "_s8"; break;
    case ClassI: s += "_i8"; break;
    case ClassW: s += "_8"; break;
    default: break;
    }
    break;
  case 's':
    switch (ck) {
    case ClassS: s += poly ? "_p16" : usgn ? "_u16" : "_s16"; break;
    case ClassI: s += "_i16"; break;
    case ClassW: s += "_16"; break;
    default: break;
    }
    break;
  case 'i':
    switch (ck) {
    case ClassS: s += usgn ? "_u32" : "_s32"; break;
    case ClassI: s += "_i32"; break;
    case ClassW: s += "_32"; break;
    default: break;
    }
    break;
  case 'l':
    switch (ck) {
    case ClassS: s += usgn ? "_u64" : "_s64"; break;
    case ClassI: s += "_i64"; break;
    case ClassW: s += "_64"; break;
    default: break;
    }
    break;
  case 'h':
    switch (ck) {
    case ClassS:
    case ClassI: s += "_f16"; break;
    case ClassW: s += "_16"; break;
    default: break;
    }
    break;
  case 'f':
    switch (ck) {
    case ClassS:
    case ClassI: s += "_f32"; break;
    case ClassW: s += "_32"; break;
    default: break;
    }
    break;
  default:
    PrintFatalError("unhandled type!");
  }
  if (ck == ClassB)
    s += "_v";

  // Insert a 'q' before the first '_' character so that it ends up before
  // _lane or _n on vector-scalar operations.
  if (quad) {
    size_t pos = s.find('_');
    s = s.insert(pos, "q");
  }
  return s;
}

/// UseMacro - Examine the prototype string to determine if the intrinsic
/// should be defined as a preprocessor macro instead of an inline function.
static bool UseMacro(const std::string &proto) {
  // If this builtin takes an immediate argument, we need to #define it rather
  // than use a standard declaration, so that SemaChecking can range check
  // the immediate passed by the user.
  if (proto.find('i') != std::string::npos)
    return true;

  // Pointer arguments need to use macros to avoid hiding aligned attributes
  // from the pointer type.
  if (proto.find('p') != std::string::npos ||
      proto.find('c') != std::string::npos)
    return true;

  return false;
}

/// MacroArgUsedDirectly - Return true if argument i for an intrinsic that is
/// defined as a macro should be accessed directly instead of being first
/// assigned to a local temporary.
static bool MacroArgUsedDirectly(const std::string &proto, unsigned i) {
  // True for constant ints (i), pointers (p) and const pointers (c).
  return (proto[i] == 'i' || proto[i] == 'p' || proto[i] == 'c');
}

// Generate the string "(argtype a, argtype b, ...)"
static std::string GenArgs(const std::string &proto, StringRef typestr) {
  bool define = UseMacro(proto);
  char arg = 'a';

  std::string s;
  s += "(";

  for (unsigned i = 1, e = proto.size(); i != e; ++i, ++arg) {
    if (define) {
      // Some macro arguments are used directly instead of being assigned
      // to local temporaries; prepend an underscore prefix to make their
      // names consistent with the local temporaries.
      if (MacroArgUsedDirectly(proto, i))
        s += "__";
    } else {
      s += TypeString(proto[i], typestr) + " __";
    }
    s.push_back(arg);
    if ((i + 1) < e)
      s += ", ";
  }

  s += ")";
  return s;
}

// Macro arguments are not type-checked like inline function arguments, so
// assign them to local temporaries to get the right type checking.
static std::string GenMacroLocals(const std::string &proto, StringRef typestr) {
  char arg = 'a';
  std::string s;
  bool generatedLocal = false;

  for (unsigned i = 1, e = proto.size(); i != e; ++i, ++arg) {
    // Do not create a temporary for an immediate argument.
    // That would defeat the whole point of using a macro!
    if (MacroArgUsedDirectly(proto, i))
      continue;
    generatedLocal = true;

    s += TypeString(proto[i], typestr) + " __";
    s.push_back(arg);
    s += " = (";
    s.push_back(arg);
    s += "); ";
  }

  if (generatedLocal)
    s += "\\\n  ";
  return s;
}

// Use the vmovl builtin to sign-extend or zero-extend a vector.
static std::string Extend(StringRef typestr, const std::string &a) {
  std::string s;
  s = MangleName("vmovl", typestr, ClassS);
  s += "(" + a + ")";
  return s;
}

static std::string Duplicate(unsigned nElts, StringRef typestr,
                             const std::string &a) {
  std::string s;

  s = "(" + TypeString('d', typestr) + "){ ";
  for (unsigned i = 0; i != nElts; ++i) {
    s += a;
    if ((i + 1) < nElts)
      s += ", ";
  }
  s += " }";

  return s;
}

static std::string SplatLane(unsigned nElts, const std::string &vec,
                             const std::string &lane) {
  std::string s = "__builtin_shufflevector(" + vec + ", " + vec;
  for (unsigned i = 0; i < nElts; ++i)
    s += ", " + lane;
  s += ")";
  return s;
}

static unsigned GetNumElements(StringRef typestr, bool &quad) {
  quad = false;
  bool dummy = false;
  char type = ClassifyType(typestr, quad, dummy, dummy);
  unsigned nElts = 0;
  switch (type) {
  case 'c': nElts = 8; break;
  case 's': nElts = 4; break;
  case 'i': nElts = 2; break;
  case 'l': nElts = 1; break;
  case 'h': nElts = 4; break;
  case 'f': nElts = 2; break;
  default:
    PrintFatalError("unhandled type!");
  }
  if (quad) nElts <<= 1;
  return nElts;
}

// Generate the definition for this intrinsic, e.g. "a + b" for OpAdd.
static std::string GenOpString(OpKind op, const std::string &proto,
                               StringRef typestr) {
  bool quad;
  unsigned nElts = GetNumElements(typestr, quad);
  bool define = UseMacro(proto);

  std::string ts = TypeString(proto[0], typestr);
  std::string s;
  if (!define) {
    s = "return ";
  }

  switch(op) {
  case OpAdd:
    s += "__a + __b;";
    break;
  case OpAddl:
    s += Extend(typestr, "__a") + " + " + Extend(typestr, "__b") + ";";
    break;
  case OpAddw:
    s += "__a + " + Extend(typestr, "__b") + ";";
    break;
  case OpSub:
    s += "__a - __b;";
    break;
  case OpSubl:
    s += Extend(typestr, "__a") + " - " + Extend(typestr, "__b") + ";";
    break;
  case OpSubw:
    s += "__a - " + Extend(typestr, "__b") + ";";
    break;
  case OpMulN:
    s += "__a * " + Duplicate(nElts, typestr, "__b") + ";";
    break;
  case OpMulLane:
    s += "__a * " + SplatLane(nElts, "__b", "__c") + ";";
    break;
  case OpMul:
    s += "__a * __b;";
    break;
  case OpMullLane:
    s += MangleName("vmull", typestr, ClassS) + "(__a, " +
      SplatLane(nElts, "__b", "__c") + ");";
    break;
  case OpMlaN:
    s += "__a + (__b * " + Duplicate(nElts, typestr, "__c") + ");";
    break;
  case OpMlaLane:
    s += "__a + (__b * " + SplatLane(nElts, "__c", "__d") + ");";
    break;
  case OpMla:
    s += "__a + (__b * __c);";
    break;
  case OpMlalN:
    s += "__a + " + MangleName("vmull", typestr, ClassS) + "(__b, " +
      Duplicate(nElts, typestr, "__c") + ");";
    break;
  case OpMlalLane:
    s += "__a + " + MangleName("vmull", typestr, ClassS) + "(__b, " +
      SplatLane(nElts, "__c", "__d") + ");";
    break;
  case OpMlal:
    s += "__a + " + MangleName("vmull", typestr, ClassS) + "(__b, __c);";
    break;
  case OpMlsN:
    s += "__a - (__b * " + Duplicate(nElts, typestr, "__c") + ");";
    break;
  case OpMlsLane:
    s += "__a - (__b * " + SplatLane(nElts, "__c", "__d") + ");";
    break;
  case OpMls:
    s += "__a - (__b * __c);";
    break;
  case OpMlslN:
    s += "__a - " + MangleName("vmull", typestr, ClassS) + "(__b, " +
      Duplicate(nElts, typestr, "__c") + ");";
    break;
  case OpMlslLane:
    s += "__a - " + MangleName("vmull", typestr, ClassS) + "(__b, " +
      SplatLane(nElts, "__c", "__d") + ");";
    break;
  case OpMlsl:
    s += "__a - " + MangleName("vmull", typestr, ClassS) + "(__b, __c);";
    break;
  case OpQDMullLane:
    s += MangleName("vqdmull", typestr, ClassS) + "(__a, " +
      SplatLane(nElts, "__b", "__c") + ");";
    break;
  case OpQDMlalLane:
    s += MangleName("vqdmlal", typestr, ClassS) + "(__a, __b, " +
      SplatLane(nElts, "__c", "__d") + ");";
    break;
  case OpQDMlslLane:
    s += MangleName("vqdmlsl", typestr, ClassS) + "(__a, __b, " +
      SplatLane(nElts, "__c", "__d") + ");";
    break;
  case OpQDMulhLane:
    s += MangleName("vqdmulh", typestr, ClassS) + "(__a, " +
      SplatLane(nElts, "__b", "__c") + ");";
    break;
  case OpQRDMulhLane:
    s += MangleName("vqrdmulh", typestr, ClassS) + "(__a, " +
      SplatLane(nElts, "__b", "__c") + ");";
    break;
  case OpEq:
    s += "(" + ts + ")(__a == __b);";
    break;
  case OpGe:
    s += "(" + ts + ")(__a >= __b);";
    break;
  case OpLe:
    s += "(" + ts + ")(__a <= __b);";
    break;
  case OpGt:
    s += "(" + ts + ")(__a > __b);";
    break;
  case OpLt:
    s += "(" + ts + ")(__a < __b);";
    break;
  case OpNeg:
    s += " -__a;";
    break;
  case OpNot:
    s += " ~__a;";
    break;
  case OpAnd:
    s += "__a & __b;";
    break;
  case OpOr:
    s += "__a | __b;";
    break;
  case OpXor:
    s += "__a ^ __b;";
    break;
  case OpAndNot:
    s += "__a & ~__b;";
    break;
  case OpOrNot:
    s += "__a | ~__b;";
    break;
  case OpCast:
    s += "(" + ts + ")__a;";
    break;
  case OpConcat:
    s += "(" + ts + ")__builtin_shufflevector((int64x1_t)__a";
    s += ", (int64x1_t)__b, 0, 1);";
    break;
  case OpHi:
    s += "(" + ts +
      ")__builtin_shufflevector((int64x2_t)__a, (int64x2_t)__a, 1);";
    break;
  case OpLo:
    s += "(" + ts +
      ")__builtin_shufflevector((int64x2_t)__a, (int64x2_t)__a, 0);";
    break;
  case OpDup:
    s += Duplicate(nElts, typestr, "__a") + ";";
    break;
  case OpDupLane:
    s += SplatLane(nElts, "__a", "__b") + ";";
    break;
  case OpSelect:
    // ((0 & 1) | (~0 & 2))
    s += "(" + ts + ")";
    ts = TypeString(proto[1], typestr);
    s += "((__a & (" + ts + ")__b) | ";
    s += "(~__a & (" + ts + ")__c));";
    break;
  case OpRev16:
    s += "__builtin_shufflevector(__a, __a";
    for (unsigned i = 2; i <= nElts; i += 2)
      for (unsigned j = 0; j != 2; ++j)
        s += ", " + utostr(i - j - 1);
    s += ");";
    break;
  case OpRev32: {
    unsigned WordElts = nElts >> (1 + (int)quad);
    s += "__builtin_shufflevector(__a, __a";
    for (unsigned i = WordElts; i <= nElts; i += WordElts)
      for (unsigned j = 0; j != WordElts; ++j)
        s += ", " + utostr(i - j - 1);
    s += ");";
    break;
  }
  case OpRev64: {
    unsigned DblWordElts = nElts >> (int)quad;
    s += "__builtin_shufflevector(__a, __a";
    for (unsigned i = DblWordElts; i <= nElts; i += DblWordElts)
      for (unsigned j = 0; j != DblWordElts; ++j)
        s += ", " + utostr(i - j - 1);
    s += ");";
    break;
  }
  case OpAbdl: {
    std::string abd = MangleName("vabd", typestr, ClassS) + "(__a, __b)";
    if (typestr[0] != 'U') {
      // vabd results are always unsigned and must be zero-extended.
      std::string utype = "U" + typestr.str();
      s += "(" + TypeString(proto[0], typestr) + ")";
      abd = "(" + TypeString('d', utype) + ")" + abd;
      s += Extend(utype, abd) + ";";
    } else {
      s += Extend(typestr, abd) + ";";
    }
    break;
  }
  case OpAba:
    s += "__a + " + MangleName("vabd", typestr, ClassS) + "(__b, __c);";
    break;
  case OpAbal: {
    s += "__a + ";
    std::string abd = MangleName("vabd", typestr, ClassS) + "(__b, __c)";
    if (typestr[0] != 'U') {
      // vabd results are always unsigned and must be zero-extended.
      std::string utype = "U" + typestr.str();
      s += "(" + TypeString(proto[0], typestr) + ")";
      abd = "(" + TypeString('d', utype) + ")" + abd;
      s += Extend(utype, abd) + ";";
    } else {
      s += Extend(typestr, abd) + ";";
    }
    break;
  }
  default:
    PrintFatalError("unknown OpKind!");
  }
  return s;
}

static unsigned GetNeonEnum(const std::string &proto, StringRef typestr) {
  unsigned mod = proto[0];

  if (mod == 'v' || mod == 'f')
    mod = proto[1];

  bool quad = false;
  bool poly = false;
  bool usgn = false;
  bool scal = false;
  bool cnst = false;
  bool pntr = false;

  // Base type to get the type string for.
  char type = ClassifyType(typestr, quad, poly, usgn);

  // Based on the modifying character, change the type and width if necessary.
  type = ModType(mod, type, quad, poly, usgn, scal, cnst, pntr);

  NeonTypeFlags::EltType ET;
  switch (type) {
    case 'c':
      ET = poly ? NeonTypeFlags::Poly8 : NeonTypeFlags::Int8;
      break;
    case 's':
      ET = poly ? NeonTypeFlags::Poly16 : NeonTypeFlags::Int16;
      break;
    case 'i':
      ET = NeonTypeFlags::Int32;
      break;
    case 'l':
      ET = NeonTypeFlags::Int64;
      break;
    case 'h':
      ET = NeonTypeFlags::Float16;
      break;
    case 'f':
      ET = NeonTypeFlags::Float32;
      break;
    default:
      PrintFatalError("unhandled type!");
  }
  NeonTypeFlags Flags(ET, usgn, quad && proto[1] != 'g');
  return Flags.getFlags();
}

// Generate the definition for this intrinsic, e.g. __builtin_neon_cls(a)
static std::string GenBuiltin(const std::string &name, const std::string &proto,
                              StringRef typestr, ClassKind ck) {
  std::string s;

  // If this builtin returns a struct 2, 3, or 4 vectors, pass it as an implicit
  // sret-like argument.
  bool sret = (proto[0] >= '2' && proto[0] <= '4');

  bool define = UseMacro(proto);

  // Check if the prototype has a scalar operand with the type of the vector
  // elements.  If not, bitcasting the args will take care of arg checking.
  // The actual signedness etc. will be taken care of with special enums.
  if (proto.find('s') == std::string::npos)
    ck = ClassB;

  if (proto[0] != 'v') {
    std::string ts = TypeString(proto[0], typestr);

    if (define) {
      if (sret)
        s += ts + " r; ";
      else
        s += "(" + ts + ")";
    } else if (sret) {
      s += ts + " r; ";
    } else {
      s += "return (" + ts + ")";
    }
  }

  bool splat = proto.find('a') != std::string::npos;

  s += "__builtin_neon_";
  if (splat) {
    // Call the non-splat builtin: chop off the "_n" suffix from the name.
    std::string vname(name, 0, name.size()-2);
    s += MangleName(vname, typestr, ck);
  } else {
    s += MangleName(name, typestr, ck);
  }
  s += "(";

  // Pass the address of the return variable as the first argument to sret-like
  // builtins.
  if (sret)
    s += "&r, ";

  char arg = 'a';
  for (unsigned i = 1, e = proto.size(); i != e; ++i, ++arg) {
    std::string args = std::string(&arg, 1);

    // Use the local temporaries instead of the macro arguments.
    args = "__" + args;

    bool argQuad = false;
    bool argPoly = false;
    bool argUsgn = false;
    bool argScalar = false;
    bool dummy = false;
    char argType = ClassifyType(typestr, argQuad, argPoly, argUsgn);
    argType = ModType(proto[i], argType, argQuad, argPoly, argUsgn, argScalar,
                      dummy, dummy);

    // Handle multiple-vector values specially, emitting each subvector as an
    // argument to the __builtin.
    if (proto[i] >= '2' && proto[i] <= '4') {
      // Check if an explicit cast is needed.
      if (argType != 'c' || argPoly || argUsgn)
        args = (argQuad ? "(int8x16_t)" : "(int8x8_t)") + args;

      for (unsigned vi = 0, ve = proto[i] - '0'; vi != ve; ++vi) {
        s += args + ".val[" + utostr(vi) + "]";
        if ((vi + 1) < ve)
          s += ", ";
      }
      if ((i + 1) < e)
        s += ", ";

      continue;
    }

    if (splat && (i + 1) == e)
      args = Duplicate(GetNumElements(typestr, argQuad), typestr, args);

    // Check if an explicit cast is needed.
    if ((splat || !argScalar) &&
        ((ck == ClassB && argType != 'c') || argPoly || argUsgn)) {
      std::string argTypeStr = "c";
      if (ck != ClassB)
        argTypeStr = argType;
      if (argQuad)
        argTypeStr = "Q" + argTypeStr;
      args = "(" + TypeString('d', argTypeStr) + ")" + args;
    }

    s += args;
    if ((i + 1) < e)
      s += ", ";
  }

  // Extra constant integer to hold type class enum for this function, e.g. s8
  if (ck == ClassB)
    s += ", " + utostr(GetNeonEnum(proto, typestr));

  s += ");";

  if (proto[0] != 'v' && sret) {
    if (define)
      s += " r;";
    else
      s += " return r;";
  }
  return s;
}

static std::string GenBuiltinDef(const std::string &name,
                                 const std::string &proto,
                                 StringRef typestr, ClassKind ck) {
  std::string s("BUILTIN(__builtin_neon_");

  // If all types are the same size, bitcasting the args will take care
  // of arg checking.  The actual signedness etc. will be taken care of with
  // special enums.
  if (proto.find('s') == std::string::npos)
    ck = ClassB;

  s += MangleName(name, typestr, ck);
  s += ", \"";

  for (unsigned i = 0, e = proto.size(); i != e; ++i)
    s += BuiltinTypeString(proto[i], typestr, ck, i == 0);

  // Extra constant integer to hold type class enum for this function, e.g. s8
  if (ck == ClassB)
    s += "i";

  s += "\", \"n\")";
  return s;
}

static std::string GenIntrinsic(const std::string &name,
                                const std::string &proto,
                                StringRef outTypeStr, StringRef inTypeStr,
                                OpKind kind, ClassKind classKind) {
  assert(!proto.empty() && "");
  bool define = UseMacro(proto) && kind != OpUnavailable;
  std::string s;

  // static always inline + return type
  if (define)
    s += "#define ";
  else
    s += "__ai " + TypeString(proto[0], outTypeStr) + " ";

  // Function name with type suffix
  std::string mangledName = MangleName(name, outTypeStr, ClassS);
  if (outTypeStr != inTypeStr) {
    // If the input type is different (e.g., for vreinterpret), append a suffix
    // for the input type.  String off a "Q" (quad) prefix so that MangleName
    // does not insert another "q" in the name.
    unsigned typeStrOff = (inTypeStr[0] == 'Q' ? 1 : 0);
    StringRef inTypeNoQuad = inTypeStr.substr(typeStrOff);
    mangledName = MangleName(mangledName, inTypeNoQuad, ClassS);
  }
  s += mangledName;

  // Function arguments
  s += GenArgs(proto, inTypeStr);

  // Definition.
  if (define) {
    s += " __extension__ ({ \\\n  ";
    s += GenMacroLocals(proto, inTypeStr);
  } else if (kind == OpUnavailable) {
    s += " __attribute__((unavailable));\n";
    return s;
  } else
    s += " {\n  ";

  if (kind != OpNone)
    s += GenOpString(kind, proto, outTypeStr);
  else
    s += GenBuiltin(name, proto, outTypeStr, classKind);
  if (define)
    s += " })";
  else
    s += " }";
  s += "\n";
  return s;
}

/// run - Read the records in arm_neon.td and output arm_neon.h.  arm_neon.h
/// is comprised of type definitions and function declarations.
void NeonEmitter::run(raw_ostream &OS) {
  OS << 
    "/*===---- arm_neon.h - ARM Neon intrinsics ------------------------------"
    "---===\n"
    " *\n"
    " * Permission is hereby granted, free of charge, to any person obtaining "
    "a copy\n"
    " * of this software and associated documentation files (the \"Software\"),"
    " to deal\n"
    " * in the Software without restriction, including without limitation the "
    "rights\n"
    " * to use, copy, modify, merge, publish, distribute, sublicense, "
    "and/or sell\n"
    " * copies of the Software, and to permit persons to whom the Software is\n"
    " * furnished to do so, subject to the following conditions:\n"
    " *\n"
    " * The above copyright notice and this permission notice shall be "
    "included in\n"
    " * all copies or substantial portions of the Software.\n"
    " *\n"
    " * THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, "
    "EXPRESS OR\n"
    " * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
    "MERCHANTABILITY,\n"
    " * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT "
    "SHALL THE\n"
    " * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR "
    "OTHER\n"
    " * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, "
    "ARISING FROM,\n"
    " * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER "
    "DEALINGS IN\n"
    " * THE SOFTWARE.\n"
    " *\n"
    " *===--------------------------------------------------------------------"
    "---===\n"
    " */\n\n";

  OS << "#ifndef __ARM_NEON_H\n";
  OS << "#define __ARM_NEON_H\n\n";

  OS << "#ifndef __ARM_NEON__\n";
  OS << "#error \"NEON support not enabled\"\n";
  OS << "#endif\n\n";

  OS << "#include <stdint.h>\n\n";

  // Emit NEON-specific scalar typedefs.
  OS << "typedef float float32_t;\n";
  OS << "typedef int8_t poly8_t;\n";
  OS << "typedef int16_t poly16_t;\n";
  OS << "typedef uint16_t float16_t;\n";

  // Emit Neon vector typedefs.
  std::string TypedefTypes("cQcsQsiQilQlUcQUcUsQUsUiQUiUlQUlhQhfQfPcQPcPsQPs");
  SmallVector<StringRef, 24> TDTypeVec;
  ParseTypes(0, TypedefTypes, TDTypeVec);

  // Emit vector typedefs.
  for (unsigned i = 0, e = TDTypeVec.size(); i != e; ++i) {
    bool dummy, quad = false, poly = false;
    (void) ClassifyType(TDTypeVec[i], quad, poly, dummy);
    if (poly)
      OS << "typedef __attribute__((neon_polyvector_type(";
    else
      OS << "typedef __attribute__((neon_vector_type(";

    unsigned nElts = GetNumElements(TDTypeVec[i], quad);
    OS << utostr(nElts) << "))) ";
    if (nElts < 10)
      OS << " ";

    OS << TypeString('s', TDTypeVec[i]);
    OS << " " << TypeString('d', TDTypeVec[i]) << ";\n";
  }
  OS << "\n";

  // Emit struct typedefs.
  for (unsigned vi = 2; vi != 5; ++vi) {
    for (unsigned i = 0, e = TDTypeVec.size(); i != e; ++i) {
      std::string ts = TypeString('d', TDTypeVec[i]);
      std::string vs = TypeString('0' + vi, TDTypeVec[i]);
      OS << "typedef struct " << vs << " {\n";
      OS << "  " << ts << " val";
      OS << "[" << utostr(vi) << "]";
      OS << ";\n} ";
      OS << vs << ";\n\n";
    }
  }

  OS<<"#define __ai static __attribute__((__always_inline__, __nodebug__))\n\n";

  std::vector<Record*> RV = Records.getAllDerivedDefinitions("Inst");

  // Emit vmovl, vmull and vabd intrinsics first so they can be used by other
  // intrinsics.  (Some of the saturating multiply instructions are also
  // used to implement the corresponding "_lane" variants, but tablegen
  // sorts the records into alphabetical order so that the "_lane" variants
  // come after the intrinsics they use.)
  emitIntrinsic(OS, Records.getDef("VMOVL"));
  emitIntrinsic(OS, Records.getDef("VMULL"));
  emitIntrinsic(OS, Records.getDef("VABD"));

  for (unsigned i = 0, e = RV.size(); i != e; ++i) {
    Record *R = RV[i];
    if (R->getName() != "VMOVL" &&
        R->getName() != "VMULL" &&
        R->getName() != "VABD")
      emitIntrinsic(OS, R);
  }

  OS << "#undef __ai\n\n";
  OS << "#endif /* __ARM_NEON_H */\n";
}

/// emitIntrinsic - Write out the arm_neon.h header file definitions for the
/// intrinsics specified by record R.
void NeonEmitter::emitIntrinsic(raw_ostream &OS, Record *R) {
  std::string name = R->getValueAsString("Name");
  std::string Proto = R->getValueAsString("Prototype");
  std::string Types = R->getValueAsString("Types");

  SmallVector<StringRef, 16> TypeVec;
  ParseTypes(R, Types, TypeVec);

  OpKind kind = OpMap[R->getValueAsDef("Operand")->getName()];

  ClassKind classKind = ClassNone;
  if (R->getSuperClasses().size() >= 2)
    classKind = ClassMap[R->getSuperClasses()[1]];
  if (classKind == ClassNone && kind == OpNone)
    PrintFatalError(R->getLoc(), "Builtin has no class kind");

  for (unsigned ti = 0, te = TypeVec.size(); ti != te; ++ti) {
    if (kind == OpReinterpret) {
      bool outQuad = false;
      bool dummy = false;
      (void)ClassifyType(TypeVec[ti], outQuad, dummy, dummy);
      for (unsigned srcti = 0, srcte = TypeVec.size();
           srcti != srcte; ++srcti) {
        bool inQuad = false;
        (void)ClassifyType(TypeVec[srcti], inQuad, dummy, dummy);
        if (srcti == ti || inQuad != outQuad)
          continue;
        OS << GenIntrinsic(name, Proto, TypeVec[ti], TypeVec[srcti],
                           OpCast, ClassS);
      }
    } else {
      OS << GenIntrinsic(name, Proto, TypeVec[ti], TypeVec[ti],
                         kind, classKind);
    }
  }
  OS << "\n";
}

static unsigned RangeFromType(const char mod, StringRef typestr) {
  // base type to get the type string for.
  bool quad = false, dummy = false;
  char type = ClassifyType(typestr, quad, dummy, dummy);
  type = ModType(mod, type, quad, dummy, dummy, dummy, dummy, dummy);

  switch (type) {
    case 'c':
      return (8 << (int)quad) - 1;
    case 'h':
    case 's':
      return (4 << (int)quad) - 1;
    case 'f':
    case 'i':
      return (2 << (int)quad) - 1;
    case 'l':
      return (1 << (int)quad) - 1;
    default:
      PrintFatalError("unhandled type!");
  }
}

/// runHeader - Emit a file with sections defining:
/// 1. the NEON section of BuiltinsARM.def.
/// 2. the SemaChecking code for the type overload checking.
/// 3. the SemaChecking code for validation of intrinsic immediate arguments.
void NeonEmitter::runHeader(raw_ostream &OS) {
  std::vector<Record*> RV = Records.getAllDerivedDefinitions("Inst");

  StringMap<OpKind> EmittedMap;

  // Generate BuiltinsARM.def for NEON
  OS << "#ifdef GET_NEON_BUILTINS\n";
  for (unsigned i = 0, e = RV.size(); i != e; ++i) {
    Record *R = RV[i];
    OpKind k = OpMap[R->getValueAsDef("Operand")->getName()];
    if (k != OpNone)
      continue;

    std::string Proto = R->getValueAsString("Prototype");

    // Functions with 'a' (the splat code) in the type prototype should not get
    // their own builtin as they use the non-splat variant.
    if (Proto.find('a') != std::string::npos)
      continue;

    std::string Types = R->getValueAsString("Types");
    SmallVector<StringRef, 16> TypeVec;
    ParseTypes(R, Types, TypeVec);

    if (R->getSuperClasses().size() < 2)
      PrintFatalError(R->getLoc(), "Builtin has no class kind");

    std::string name = R->getValueAsString("Name");
    ClassKind ck = ClassMap[R->getSuperClasses()[1]];

    for (unsigned ti = 0, te = TypeVec.size(); ti != te; ++ti) {
      // Generate the BuiltinsARM.def declaration for this builtin, ensuring
      // that each unique BUILTIN() macro appears only once in the output
      // stream.
      std::string bd = GenBuiltinDef(name, Proto, TypeVec[ti], ck);
      if (EmittedMap.count(bd))
        continue;

      EmittedMap[bd] = OpNone;
      OS << bd << "\n";
    }
  }
  OS << "#endif\n\n";

  // Generate the overloaded type checking code for SemaChecking.cpp
  OS << "#ifdef GET_NEON_OVERLOAD_CHECK\n";
  for (unsigned i = 0, e = RV.size(); i != e; ++i) {
    Record *R = RV[i];
    OpKind k = OpMap[R->getValueAsDef("Operand")->getName()];
    if (k != OpNone)
      continue;

    std::string Proto = R->getValueAsString("Prototype");
    std::string Types = R->getValueAsString("Types");
    std::string name = R->getValueAsString("Name");

    // Functions with 'a' (the splat code) in the type prototype should not get
    // their own builtin as they use the non-splat variant.
    if (Proto.find('a') != std::string::npos)
      continue;

    // Functions which have a scalar argument cannot be overloaded, no need to
    // check them if we are emitting the type checking code.
    if (Proto.find('s') != std::string::npos)
      continue;

    SmallVector<StringRef, 16> TypeVec;
    ParseTypes(R, Types, TypeVec);

    if (R->getSuperClasses().size() < 2)
      PrintFatalError(R->getLoc(), "Builtin has no class kind");

    int si = -1, qi = -1;
    uint64_t mask = 0, qmask = 0;
    for (unsigned ti = 0, te = TypeVec.size(); ti != te; ++ti) {
      // Generate the switch case(s) for this builtin for the type validation.
      bool quad = false, poly = false, usgn = false;
      (void) ClassifyType(TypeVec[ti], quad, poly, usgn);

      if (quad) {
        qi = ti;
        qmask |= 1ULL << GetNeonEnum(Proto, TypeVec[ti]);
      } else {
        si = ti;
        mask |= 1ULL << GetNeonEnum(Proto, TypeVec[ti]);
      }
    }

    // Check if the builtin function has a pointer or const pointer argument.
    int PtrArgNum = -1;
    bool HasConstPtr = false;
    for (unsigned arg = 1, arge = Proto.size(); arg != arge; ++arg) {
      char ArgType = Proto[arg];
      if (ArgType == 'c') {
        HasConstPtr = true;
        PtrArgNum = arg - 1;
        break;
      }
      if (ArgType == 'p') {
        PtrArgNum = arg - 1;
        break;
      }
    }
    // For sret builtins, adjust the pointer argument index.
    if (PtrArgNum >= 0 && (Proto[0] >= '2' && Proto[0] <= '4'))
      PtrArgNum += 1;

    // Omit type checking for the pointer arguments of vld1_lane, vld1_dup,
    // and vst1_lane intrinsics.  Using a pointer to the vector element
    // type with one of those operations causes codegen to select an aligned
    // load/store instruction.  If you want an unaligned operation,
    // the pointer argument needs to have less alignment than element type,
    // so just accept any pointer type.
    if (name == "vld1_lane" || name == "vld1_dup" || name == "vst1_lane") {
      PtrArgNum = -1;
      HasConstPtr = false;
    }

    if (mask) {
      OS << "case ARM::BI__builtin_neon_"
         << MangleName(name, TypeVec[si], ClassB)
         << ": mask = " << "0x" << utohexstr(mask) << "ULL";
      if (PtrArgNum >= 0)
        OS << "; PtrArgNum = " << PtrArgNum;
      if (HasConstPtr)
        OS << "; HasConstPtr = true";
      OS << "; break;\n";
    }
    if (qmask) {
      OS << "case ARM::BI__builtin_neon_"
         << MangleName(name, TypeVec[qi], ClassB)
         << ": mask = " << "0x" << utohexstr(qmask) << "ULL";
      if (PtrArgNum >= 0)
        OS << "; PtrArgNum = " << PtrArgNum;
      if (HasConstPtr)
        OS << "; HasConstPtr = true";
      OS << "; break;\n";
    }
  }
  OS << "#endif\n\n";

  // Generate the intrinsic range checking code for shift/lane immediates.
  OS << "#ifdef GET_NEON_IMMEDIATE_CHECK\n";
  for (unsigned i = 0, e = RV.size(); i != e; ++i) {
    Record *R = RV[i];

    OpKind k = OpMap[R->getValueAsDef("Operand")->getName()];
    if (k != OpNone)
      continue;

    std::string name = R->getValueAsString("Name");
    std::string Proto = R->getValueAsString("Prototype");
    std::string Types = R->getValueAsString("Types");

    // Functions with 'a' (the splat code) in the type prototype should not get
    // their own builtin as they use the non-splat variant.
    if (Proto.find('a') != std::string::npos)
      continue;

    // Functions which do not have an immediate do not need to have range
    // checking code emitted.
    size_t immPos = Proto.find('i');
    if (immPos == std::string::npos)
      continue;

    SmallVector<StringRef, 16> TypeVec;
    ParseTypes(R, Types, TypeVec);

    if (R->getSuperClasses().size() < 2)
      PrintFatalError(R->getLoc(), "Builtin has no class kind");

    ClassKind ck = ClassMap[R->getSuperClasses()[1]];

    for (unsigned ti = 0, te = TypeVec.size(); ti != te; ++ti) {
      std::string namestr, shiftstr, rangestr;

      if (R->getValueAsBit("isVCVT_N")) {
        // VCVT between floating- and fixed-point values takes an immediate
        // in the range 1 to 32.
        ck = ClassB;
        rangestr = "l = 1; u = 31"; // upper bound = l + u
      } else if (Proto.find('s') == std::string::npos) {
        // Builtins which are overloaded by type will need to have their upper
        // bound computed at Sema time based on the type constant.
        ck = ClassB;
        if (R->getValueAsBit("isShift")) {
          shiftstr = ", true";

          // Right shifts have an 'r' in the name, left shifts do not.
          if (name.find('r') != std::string::npos)
            rangestr = "l = 1; ";
        }
        rangestr += "u = RFT(TV" + shiftstr + ")";
      } else {
        // The immediate generally refers to a lane in the preceding argument.
        assert(immPos > 0 && "unexpected immediate operand");
        rangestr = "u = " + utostr(RangeFromType(Proto[immPos-1], TypeVec[ti]));
      }
      // Make sure cases appear only once by uniquing them in a string map.
      namestr = MangleName(name, TypeVec[ti], ck);
      if (EmittedMap.count(namestr))
        continue;
      EmittedMap[namestr] = OpNone;

      // Calculate the index of the immediate that should be range checked.
      unsigned immidx = 0;

      // Builtins that return a struct of multiple vectors have an extra
      // leading arg for the struct return.
      if (Proto[0] >= '2' && Proto[0] <= '4')
        ++immidx;

      // Add one to the index for each argument until we reach the immediate
      // to be checked.  Structs of vectors are passed as multiple arguments.
      for (unsigned ii = 1, ie = Proto.size(); ii != ie; ++ii) {
        switch (Proto[ii]) {
          default:  immidx += 1; break;
          case '2': immidx += 2; break;
          case '3': immidx += 3; break;
          case '4': immidx += 4; break;
          case 'i': ie = ii + 1; break;
        }
      }
      OS << "case ARM::BI__builtin_neon_" << MangleName(name, TypeVec[ti], ck)
         << ": i = " << immidx << "; " << rangestr << "; break;\n";
    }
  }
  OS << "#endif\n\n";
}

/// GenTest - Write out a test for the intrinsic specified by the name and
/// type strings, including the embedded patterns for FileCheck to match.
static std::string GenTest(const std::string &name,
                           const std::string &proto,
                           StringRef outTypeStr, StringRef inTypeStr,
                           bool isShift) {
  assert(!proto.empty() && "");
  std::string s;

  // Function name with type suffix
  std::string mangledName = MangleName(name, outTypeStr, ClassS);
  if (outTypeStr != inTypeStr) {
    // If the input type is different (e.g., for vreinterpret), append a suffix
    // for the input type.  String off a "Q" (quad) prefix so that MangleName
    // does not insert another "q" in the name.
    unsigned typeStrOff = (inTypeStr[0] == 'Q' ? 1 : 0);
    StringRef inTypeNoQuad = inTypeStr.substr(typeStrOff);
    mangledName = MangleName(mangledName, inTypeNoQuad, ClassS);
  }

  // Emit the FileCheck patterns.
  s += "// CHECK: test_" + mangledName + "\n";
  // s += "// CHECK: \n"; // FIXME: + expected instruction opcode.

  // Emit the start of the test function.
  s += TypeString(proto[0], outTypeStr) + " test_" + mangledName + "(";
  char arg = 'a';
  std::string comma;
  for (unsigned i = 1, e = proto.size(); i != e; ++i, ++arg) {
    // Do not create arguments for values that must be immediate constants.
    if (proto[i] == 'i')
      continue;
    s += comma + TypeString(proto[i], inTypeStr) + " ";
    s.push_back(arg);
    comma = ", ";
  }
  s += ") {\n  ";

  if (proto[0] != 'v')
    s += "return ";
  s += mangledName + "(";
  arg = 'a';
  for (unsigned i = 1, e = proto.size(); i != e; ++i, ++arg) {
    if (proto[i] == 'i') {
      // For immediate operands, test the maximum value.
      if (isShift)
        s += "1"; // FIXME
      else
        // The immediate generally refers to a lane in the preceding argument.
        s += utostr(RangeFromType(proto[i-1], inTypeStr));
    } else {
      s.push_back(arg);
    }
    if ((i + 1) < e)
      s += ", ";
  }
  s += ");\n}\n\n";
  return s;
}

/// runTests - Write out a complete set of tests for all of the Neon
/// intrinsics.
void NeonEmitter::runTests(raw_ostream &OS) {
  OS <<
    "// RUN: %clang_cc1 -triple thumbv7-apple-darwin \\\n"
    "// RUN:  -target-cpu cortex-a9 -ffreestanding -S -o - %s | FileCheck %s\n"
    "\n"
    "#include <arm_neon.h>\n"
    "\n";

  std::vector<Record*> RV = Records.getAllDerivedDefinitions("Inst");
  for (unsigned i = 0, e = RV.size(); i != e; ++i) {
    Record *R = RV[i];
    std::string name = R->getValueAsString("Name");
    std::string Proto = R->getValueAsString("Prototype");
    std::string Types = R->getValueAsString("Types");
    bool isShift = R->getValueAsBit("isShift");

    SmallVector<StringRef, 16> TypeVec;
    ParseTypes(R, Types, TypeVec);

    OpKind kind = OpMap[R->getValueAsDef("Operand")->getName()];
    if (kind == OpUnavailable)
      continue;
    for (unsigned ti = 0, te = TypeVec.size(); ti != te; ++ti) {
      if (kind == OpReinterpret) {
        bool outQuad = false;
        bool dummy = false;
        (void)ClassifyType(TypeVec[ti], outQuad, dummy, dummy);
        for (unsigned srcti = 0, srcte = TypeVec.size();
             srcti != srcte; ++srcti) {
          bool inQuad = false;
          (void)ClassifyType(TypeVec[srcti], inQuad, dummy, dummy);
          if (srcti == ti || inQuad != outQuad)
            continue;
          OS << GenTest(name, Proto, TypeVec[ti], TypeVec[srcti], isShift);
        }
      } else {
        OS << GenTest(name, Proto, TypeVec[ti], TypeVec[ti], isShift);
      }
    }
    OS << "\n";
  }
}

namespace clang {
void EmitNeon(RecordKeeper &Records, raw_ostream &OS) {
  NeonEmitter(Records).run(OS);
}
void EmitNeonSema(RecordKeeper &Records, raw_ostream &OS) {
  NeonEmitter(Records).runHeader(OS);
}
void EmitNeonTest(RecordKeeper &Records, raw_ostream &OS) {
  NeonEmitter(Records).runTests(OS);
}
} // End namespace clang