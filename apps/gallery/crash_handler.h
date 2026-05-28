#pragma once

namespace CarHMI::Gallery {

// Install cross-platform crash handler (backward-cpp).
// On crash, prints a full stack trace with source file / line numbers
// before the process exits.
void InstallCrashHandler();

} // namespace CarHMI::Gallery
