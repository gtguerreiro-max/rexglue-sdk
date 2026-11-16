#pragma once
// From Dust port: integer texture fetches (num_format = 1).
//
// On Xenos a fixed-point texture fetched with num_format "integer" returns the stored integers
// (e.g. -32768...32767 for a signed 16-bit component), which games scale with exp_adjust (From
// Dust samples its k_16_16 height maps that way with exp_adjust -15). The host samples a
// normalized view instead (-1...1), so the result is 2^15 times too small and the terrain is
// flat. Xenia (newer forks) rescales the normalized value in the translated shader with a
// per-texture system constant. Here the integer range is folded into exp_adjust of the copy of
// the fetch constants the shaders read (they apply 2^exp_adjust at run time, so no shader change
// is needed on either backend): +bits-1 for signed components, +bits for unsigned. The error is
// the difference between 2^n and 2^n - 1 (0.003% for 16-bit, 0.8% at most for 8-bit).
#include <algorithm>
#include <cstdint>

namespace rex::xenos {

// dest: the 32 fetch constant groups (6 dwords each) as uploaded for the shaders, already copied
// from source (the register file). Only source is read: dest is usually an upload heap mapping,
// write-combined memory that is very slow to read back.
inline void AdjustIntegerTextureFetchConstants(uint32_t* dest, const uint32_t* source) {
  for (uint32_t i = 0; i < 32; ++i) {
    const uint32_t* f = source + i * 6;
    if ((f[0] & 3) != 2 || !(f[3] & 1)) {
      continue;  // Not a texture fetch constant, or a fraction (normalized) fetch.
    }
    uint32_t bits;
    switch (f[1] & 0x3F) {
      case 2:   // k_8
      case 6:   // k_8_8_8_8
      case 10:  // k_8_8
        bits = 8;
        break;
      case 24:  // k_16
      case 25:  // k_16_16
      case 26:  // k_16_16_16_16
        bits = 16;
        break;
      default:
        continue;
    }
    // Signedness of the first component that is not gamma (sign_x..w at dword 0 bits 2...9):
    // 0 unsigned, 1 signed, 2 unsigned biased (to -1...1 like signed), 3 gamma.
    int32_t add = -1;
    for (uint32_t c = 0; c < 4; ++c) {
      uint32_t sign = (f[0] >> (2 + c * 2)) & 3;
      if (sign == 3) continue;
      add = int32_t(sign == 0 ? bits : bits - 1);
      break;
    }
    if (add < 0) continue;
    int32_t exp_adjust = int32_t(f[3] << (32 - 19)) >> (32 - 6);  // Signed bits 13...18.
    exp_adjust = std::clamp(exp_adjust + add, -32, 31);
    dest[i * 6 + 3] = (f[3] & ~(UINT32_C(0x3F) << 13)) | ((uint32_t(exp_adjust) & 0x3F) << 13);
  }
}

}  // namespace rex::xenos
