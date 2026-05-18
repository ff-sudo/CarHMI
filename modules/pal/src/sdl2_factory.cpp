#include <pal/sdl2_factory.h>
#include "sdl2/sdl2_platform.h"

namespace CarHMI::PAL {

std::unique_ptr<Platform> CreateSDL2Platform() {
    return std::make_unique<SDL2Platform>();
}

} // namespace CarHMI::PAL
