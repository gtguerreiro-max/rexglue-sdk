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

/**
 * The Xenos driver's frames for a host renderer that presents the title itself
 * (gpu.xenos.present_external) while the driver draws (a renderer switch): the window keeps the
 * host renderer's swap chain, which shows the driver's frames, so it never changes hands (under
 * vkd3d-proton a second swap chain on the window after the first one was released never
 * presented). The D3D12 presenter is the source while it exists.
 *
 * A copy request: `dest` an ID3D12Resource of the source's device (GetGuestOutputDevice), at
 * least the guest output's size, DXGI_FORMAT_R10G10B10A2_UNORM, in D3D12_RESOURCE_STATE_COPY_DEST
 * before and after; `fence` an ID3D12Fence of that device, signaled to `fence_value` on the
 * driver's direct queue once the copy ran (after the commands that drew the frame).
 */
struct GuestOutputCopy {
  void* dest = nullptr;
  u32 dest_width = 0, dest_height = 0;
  void* fence = nullptr;
  u64 fence_value = 0;
};
/// The guest output the presenter would paint next, gamma applied: its size (0 before the first
/// frame) and a serial that grows with each frame the driver drew.
struct GuestOutputInfo {
  u32 width = 0, height = 0;
  u64 serial = 0;
};
struct GuestOutputSource {
  void* context = nullptr;
  void* d3d12_device = nullptr;  // ID3D12Device the copies run on
  bool (*copy)(void* context, const GuestOutputCopy& request, GuestOutputInfo* info) = nullptr;
};
REX_API void SetGuestOutputSource(const GuestOutputSource& source);
/// Clears the source if it is still `context`'s, after any copy running through it.
REX_API void ClearGuestOutputSource(void* context);
/// The source's ID3D12Device, null without a source.
REX_API void* GetGuestOutputDevice();
/// Fills `info` (when not null) and, when the request's dest and fence are set and dest is large
/// enough, submits the copy: true once submitted. The source stays set while it runs.
REX_API bool CopyGuestOutput(const GuestOutputCopy& request, GuestOutputInfo* info);

}  // namespace rex::xenos
