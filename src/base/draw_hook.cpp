/**
 * @file        base/draw_hook.cpp
 * @brief       Storage of the per-draw pixel constant hook (rex/xenos/draw_hook.h).
 */

#include <rex/xenos/draw_hook.h>

#include <atomic>

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

}  // namespace rex::xenos
