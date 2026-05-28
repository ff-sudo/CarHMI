#include "crash_handler.h"

#ifdef CARHMI_USE_BACKWARD
#include <backward.hpp>

namespace CarHMI::Gallery {

void InstallCrashHandler() {
    // Stack trace goes to stderr AND crash.log (avoids spdlog dependency during crash)
    static backward::SignalHandling sh("crash.log");
}

} // namespace CarHMI::Gallery

#else  // fallback: no crash handler available

namespace CarHMI::Gallery {
void InstallCrashHandler() {}
} // namespace CarHMI::Gallery

#endif
