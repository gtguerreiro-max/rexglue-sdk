/**
 * @file        rex/xenos/draw_hook.h
 * @brief       A per-draw hook on the pixel shader's float constants, for game ports.
 *
 * A game whose constants assume a fixed frame rate (a per-draw step made for 30 Hz) can scale
 * them for the frames it actually draws. For each draw, just before its constants are uploaded,
 * the hook is first called with the pixel shader's ucode hash and null constants, and answers
 * whether it wants that draw (keep this cheap: it runs for every draw). If it does, it is called
 * again with the 256 pixel float constants (guest register values: 4 x 32-bit float bits each,
 * c0.x first), may change them for this draw and returns true when it did; the backend uploads
 * them and restores the guest's own values right after. The storage is in rexbase, shared by the
 * game and the GPU driver modules. The D3D12 and Vulkan Xenos drivers call it.
 */

#pragma once

#include <rex/platform.h>
#include <rex/types.h>

namespace rex::xenos {

/// used_float_constants: the 256 pixel float constants the shader reads, as 4 x 64 bits (bit n of
/// word n / 64 for c<n>), which tells passes apart better than the constants' values (shared
/// state that outlives the draw that set it).
using PixelConstantHook = bool (*)(u64 pixel_shader_ucode_hash, const u64* used_float_constants,
                                   u32* pixel_float_constants);

REX_API void SetPixelConstantHook(PixelConstantHook hook);
REX_API PixelConstantHook GetPixelConstantHook();

/**
 * The guest's gamma ramps as the Xenos command processor holds them, for a host renderer that
 * presents the title itself: the 256 entry table (DC_LUT_30_COLOR words, 10:10:10) and the PWL
 * ramp (128 entries of 3 DC_LUT_PWL_DATA words, base 16 and delta 16 bits). The command
 * processor's thread writes them as it executes the guest's register writes. Null while no
 * command processor runs.
 */
struct GammaRampView {
  const u32* table_256 = nullptr;
  const u32* pwl_rgb = nullptr;
};
REX_API void SetGammaRampView(const GammaRampView& view);
REX_API GammaRampView GetGammaRampView();

}  // namespace rex::xenos
