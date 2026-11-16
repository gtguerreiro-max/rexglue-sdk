/**
 * @file        xam/ui.h
 * @brief       Whether an XAM dialog is on screen, for a host renderer that presents itself
 */

#pragma once

#include <rex/platform.h>

namespace rex::xam {

/**
 * Reports whether an XAM dialog (a message box, the keyboard, a device selector) is on screen.
 * A host renderer that presents the title itself gives the window back to the SDK's presenter,
 * which draws the dialog, while this is true.
 */
REX_XAM_API bool XAMIsUIActive();

}  // namespace rex::xam
