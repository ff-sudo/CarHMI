#pragma once

#include <string>

namespace CarHMI::Gallery {

// Install cross-platform crash handler.
// - Windows: minidump (.dmp) + crash text log next to exe
// - Linux/macOS: backward-cpp stack trace to stderr
void InstallCrashHandler();

// Returns the directory containing the executable (for absolute log paths)
const std::string& GetExeDirectory();

} // namespace CarHMI::Gallery
