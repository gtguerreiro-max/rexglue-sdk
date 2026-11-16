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

}  // namespace rex::xenos
