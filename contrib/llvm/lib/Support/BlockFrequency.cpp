
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

#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/BlockFrequency.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace llvm;

namespace {

/// mult96bit - Multiply FREQ by N and store result in W array.
void mult96bit(uint64_t freq, uint32_t N, uint64_t W[2]) {
  uint64_t u0 = freq & UINT32_MAX;
  uint64_t u1 = freq >> 32;

  // Represent 96-bit value as w[2]:w[1]:w[0];
  uint32_t w[3] = { 0, 0, 0 };

  uint64_t t = u0 * N;
  uint64_t k = t >> 32;
  w[0] = t;
  t = u1 * N + k;
  w[1] = t;
  w[2] = t >> 32;

  // W[1] - higher bits.
  // W[0] - lower bits.
  W[0] = w[0] + ((uint64_t) w[1] << 32);
  W[1] = w[2];
}


/// div96bit - Divide 96-bit value stored in W array by D. Return 64-bit frequency.
uint64_t div96bit(uint64_t W[2], uint32_t D) {
  uint64_t y = W[0];
  uint64_t x = W[1];
  int i;

  for (i = 1; i <= 64 && x; ++i) {
    uint32_t t = (int)x >> 31;
    x = (x << 1) | (y >> 63);
    y = y << 1;
    if ((x | t) >= D) {
      x -= D;
      ++y;
    }
  }

  return y << (64 - i + 1);
}

}


BlockFrequency &BlockFrequency::operator*=(const BranchProbability &Prob) {
  uint32_t n = Prob.getNumerator();
  uint32_t d = Prob.getDenominator();

  assert(n <= d && "Probability must be less or equal to 1.");

  // Calculate Frequency * n.
  uint64_t mulLo = (Frequency & UINT32_MAX) * n;
  uint64_t mulHi = (Frequency >> 32) * n;
  uint64_t mulRes = (mulHi << 32) + mulLo;

  // If there was overflow use 96-bit operations.
  if (mulHi > UINT32_MAX || mulRes < mulLo) {
    // 96-bit value represented as W[1]:W[0].
    uint64_t W[2];

    // Probability is less or equal to 1 which means that results must fit
    // 64-bit.
    mult96bit(Frequency, n, W);
    Frequency = div96bit(W, d);
    return *this;
  }

  Frequency = mulRes / d;
  return *this;
}

const BlockFrequency
BlockFrequency::operator*(const BranchProbability &Prob) const {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  return Freq;
}

BlockFrequency &BlockFrequency::operator+=(const BlockFrequency &Freq) {
  uint64_t Before = Freq.Frequency;
  Frequency += Freq.Frequency;

  // If overflow, set frequency to the maximum value.
  if (Frequency < Before)
    Frequency = UINT64_MAX;

  return *this;
}

const BlockFrequency
BlockFrequency::operator+(const BlockFrequency &Prob) const {
  BlockFrequency Freq(Frequency);
  Freq += Prob;
  return Freq;
}

void BlockFrequency::print(raw_ostream &OS) const {
  OS << Frequency;
}

namespace llvm {

raw_ostream &operator<<(raw_ostream &OS, const BlockFrequency &Freq) {
  Freq.print(OS);
  return OS;
}

}