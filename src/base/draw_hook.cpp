/**
 * @file        base/draw_hook.cpp
 * @brief       Storage of the per-draw pixel constant hook (rex/xenos/draw_hook.h).
 */

#include <rex/xenos/draw_hook.h>

#include <atomic>
#include <mutex>

namespace rex::xenos {

namespace {
std::atomic<PixelConstantHook> g_pixel_constant_hook{nullptr};
}  // namespace

void SetPixelConstantHook(PixelConstantHook hook) {
  g_pixel_constant_hook.store(hook, std::memory_order_release);
}

PixelConstantHook GetPixelConstantHook() {
  return g_pixel_constant_hook.load(std::memory_order_acquire);
}

namespace {
std::atomic<const u32*> g_gamma_table_256{nullptr};
std::atomic<const u32*> g_gamma_pwl_rgb{nullptr};
}  // namespace

void SetGammaRampView(const GammaRampView& view) {
  g_gamma_table_256.store(view.table_256, std::memory_order_release);
  g_gamma_pwl_rgb.store(view.pwl_rgb, std::memory_order_release);
}

GammaRampView GetGammaRampView() {
  return {g_gamma_table_256.load(std::memory_order_acquire),
          g_gamma_pwl_rgb.load(std::memory_order_acquire)};
}

namespace {
// Held across a copy, so clearing the source waits for a copy running through it.
std::mutex g_guest_output_mutex;
GuestOutputSource g_guest_output_source;
}  // namespace

void SetGuestOutputSource(const GuestOutputSource& source) {
  std::lock_guard<std::mutex> lock(g_guest_output_mutex);
  g_guest_output_source = source;
}

void ClearGuestOutputSource(void* context) {
  std::lock_guard<std::mutex> lock(g_guest_output_mutex);
  if (g_guest_output_source.context == context) g_guest_output_source = {};
}

void* GetGuestOutputDevice() {
  std::lock_guard<std::mutex> lock(g_guest_output_mutex);
  return g_guest_output_source.copy ? g_guest_output_source.d3d12_device : nullptr;
}

bool CopyGuestOutput(const GuestOutputCopy& request, GuestOutputInfo* info) {
  std::lock_guard<std::mutex> lock(g_guest_output_mutex);
  GuestOutputInfo local;
  if (!info) info = &local;
  *info = {};
  const GuestOutputSource& s = g_guest_output_source;
  return s.copy && s.copy(s.context, request, info);
}

}  // namespace rex::xenos
