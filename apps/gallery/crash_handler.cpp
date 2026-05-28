#include "crash_handler.h"

#ifdef CARHMI_USE_BACKWARD
#include <backward.hpp>

namespace CarHMI::Gallery {

void InstallCrashHandler() {
    static backward::SignalHandling sh; // installs signal / SEH handlers
}

} // namespace CarHMI::Gallery

#else  // fallback: no crash handler available

namespace CarHMI::Gallery {
void InstallCrashHandler() {}
} // namespace CarHMI::Gallery

#endif
