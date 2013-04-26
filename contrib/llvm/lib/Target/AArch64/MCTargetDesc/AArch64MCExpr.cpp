
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

#define DEBUG_TYPE "aarch64mcexpr"
#include "AArch64MCExpr.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELF.h"
#include "llvm/Object/ELF.h"

using namespace llvm;

const AArch64MCExpr*
AArch64MCExpr::Create(VariantKind Kind, const MCExpr *Expr,
                      MCContext &Ctx) {
  return new (Ctx) AArch64MCExpr(Kind, Expr);
}

void AArch64MCExpr::PrintImpl(raw_ostream &OS) const {
  switch (Kind) {
  default: llvm_unreachable("Invalid kind!");
  case VK_AARCH64_GOT:              OS << ":got:"; break;
  case VK_AARCH64_GOT_LO12:         OS << ":got_lo12:"; break;
  case VK_AARCH64_LO12:             OS << ":lo12:"; break;
  case VK_AARCH64_ABS_G0:           OS << ":abs_g0:"; break;
  case VK_AARCH64_ABS_G0_NC:        OS << ":abs_g0_nc:"; break;
  case VK_AARCH64_ABS_G1:           OS << ":abs_g1:"; break;
  case VK_AARCH64_ABS_G1_NC:        OS << ":abs_g1_nc:"; break;
  case VK_AARCH64_ABS_G2:           OS << ":abs_g2:"; break;
  case VK_AARCH64_ABS_G2_NC:        OS << ":abs_g2_nc:"; break;
  case VK_AARCH64_ABS_G3:           OS << ":abs_g3:"; break;
  case VK_AARCH64_SABS_G0:          OS << ":abs_g0_s:"; break;
  case VK_AARCH64_SABS_G1:          OS << ":abs_g1_s:"; break;
  case VK_AARCH64_SABS_G2:          OS << ":abs_g2_s:"; break;
  case VK_AARCH64_DTPREL_G2:        OS << ":dtprel_g2:"; break;
  case VK_AARCH64_DTPREL_G1:        OS << ":dtprel_g1:"; break;
  case VK_AARCH64_DTPREL_G1_NC:     OS << ":dtprel_g1_nc:"; break;
  case VK_AARCH64_DTPREL_G0:        OS << ":dtprel_g0:"; break;
  case VK_AARCH64_DTPREL_G0_NC:     OS << ":dtprel_g0_nc:"; break;
  case VK_AARCH64_DTPREL_HI12:      OS << ":dtprel_hi12:"; break;
  case VK_AARCH64_DTPREL_LO12:      OS << ":dtprel_lo12:"; break;
  case VK_AARCH64_DTPREL_LO12_NC:   OS << ":dtprel_lo12_nc:"; break;
  case VK_AARCH64_GOTTPREL_G1:      OS << ":gottprel_g1:"; break;
  case VK_AARCH64_GOTTPREL_G0_NC:   OS << ":gottprel_g0_nc:"; break;
  case VK_AARCH64_GOTTPREL:         OS << ":gottprel:"; break;
  case VK_AARCH64_GOTTPREL_LO12:    OS << ":gottprel_lo12:"; break;
  case VK_AARCH64_TPREL_G2:         OS << ":tprel_g2:"; break;
  case VK_AARCH64_TPREL_G1:         OS << ":tprel_g1:"; break;
  case VK_AARCH64_TPREL_G1_NC:      OS << ":tprel_g1_nc:"; break;
  case VK_AARCH64_TPREL_G0:         OS << ":tprel_g0:"; break;
  case VK_AARCH64_TPREL_G0_NC:      OS << ":tprel_g0_nc:"; break;
  case VK_AARCH64_TPREL_HI12:       OS << ":tprel_hi12:"; break;
  case VK_AARCH64_TPREL_LO12:       OS << ":tprel_lo12:"; break;
  case VK_AARCH64_TPREL_LO12_NC:    OS << ":tprel_lo12_nc:"; break;
  case VK_AARCH64_TLSDESC:          OS << ":tlsdesc:"; break;
  case VK_AARCH64_TLSDESC_LO12:     OS << ":tlsdesc_lo12:"; break;

  }

  const MCExpr *Expr = getSubExpr();
  if (Expr->getKind() != MCExpr::SymbolRef)
    OS << '(';
  Expr->print(OS);
  if (Expr->getKind() != MCExpr::SymbolRef)
    OS << ')';
}

bool
AArch64MCExpr::EvaluateAsRelocatableImpl(MCValue &Res,
                                         const MCAsmLayout *Layout) const {
  return getSubExpr()->EvaluateAsRelocatable(Res, *Layout);
}

static void fixELFSymbolsInTLSFixupsImpl(const MCExpr *Expr, MCAssembler &Asm) {
  switch (Expr->getKind()) {
  case MCExpr::Target:
    llvm_unreachable("Can't handle nested target expression");
    break;
  case MCExpr::Constant:
    break;

  case MCExpr::Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(Expr);
    fixELFSymbolsInTLSFixupsImpl(BE->getLHS(), Asm);
    fixELFSymbolsInTLSFixupsImpl(BE->getRHS(), Asm);
    break;
  }

  case MCExpr::SymbolRef: {
    // We're known to be under a TLS fixup, so any symbol should be
    // modified. There should be only one.
    const MCSymbolRefExpr &SymRef = *cast<MCSymbolRefExpr>(Expr);
    MCSymbolData &SD = Asm.getOrCreateSymbolData(SymRef.getSymbol());
    MCELF::SetType(SD, ELF::STT_TLS);
    break;
  }

  case MCExpr::Unary:
    fixELFSymbolsInTLSFixupsImpl(cast<MCUnaryExpr>(Expr)->getSubExpr(), Asm);
    break;
  }
}

void AArch64MCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {
  switch (getKind()) {
  default:
    return;
  case VK_AARCH64_DTPREL_G2:
  case VK_AARCH64_DTPREL_G1:
  case VK_AARCH64_DTPREL_G1_NC:
  case VK_AARCH64_DTPREL_G0:
  case VK_AARCH64_DTPREL_G0_NC:
  case VK_AARCH64_DTPREL_HI12:
  case VK_AARCH64_DTPREL_LO12:
  case VK_AARCH64_DTPREL_LO12_NC:
  case VK_AARCH64_GOTTPREL_G1:
  case VK_AARCH64_GOTTPREL_G0_NC:
  case VK_AARCH64_GOTTPREL:
  case VK_AARCH64_GOTTPREL_LO12:
  case VK_AARCH64_TPREL_G2:
  case VK_AARCH64_TPREL_G1:
  case VK_AARCH64_TPREL_G1_NC:
  case VK_AARCH64_TPREL_G0:
  case VK_AARCH64_TPREL_G0_NC:
  case VK_AARCH64_TPREL_HI12:
  case VK_AARCH64_TPREL_LO12:
  case VK_AARCH64_TPREL_LO12_NC:
  case VK_AARCH64_TLSDESC:
  case VK_AARCH64_TLSDESC_LO12:
    break;
  }

  fixELFSymbolsInTLSFixupsImpl(getSubExpr(), Asm);
}

// FIXME: This basically copies MCObjectStreamer::AddValueSymbols. Perhaps
// that method should be made public?
// FIXME: really do above: now that two backends are using it.
static void AddValueSymbolsImpl(const MCExpr *Value, MCAssembler *Asm) {
  switch (Value->getKind()) {
  case MCExpr::Target:
    llvm_unreachable("Can't handle nested target expr!");
    break;

  case MCExpr::Constant:
    break;

  case MCExpr::Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(Value);
    AddValueSymbolsImpl(BE->getLHS(), Asm);
    AddValueSymbolsImpl(BE->getRHS(), Asm);
    break;
  }

  case MCExpr::SymbolRef:
    Asm->getOrCreateSymbolData(cast<MCSymbolRefExpr>(Value)->getSymbol());
    break;

  case MCExpr::Unary:
    AddValueSymbolsImpl(cast<MCUnaryExpr>(Value)->getSubExpr(), Asm);
    break;
  }
}

void AArch64MCExpr::AddValueSymbols(MCAssembler *Asm) const {
  AddValueSymbolsImpl(getSubExpr(), Asm);
}