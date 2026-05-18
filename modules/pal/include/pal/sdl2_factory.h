#pragma once

#include <pal/platform.h>
#include <memory>

namespace CarHMI::PAL {

std::unique_ptr<Platform> CreateSDL2Platform();

} // namespace CarHMI::PAL
