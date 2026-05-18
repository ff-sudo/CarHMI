#include <pal/platform.h>
#include <memory>

// Include the SDL2 platform implementation
namespace CarHMI::PAL { class SDL2Platform; }

namespace CarHMI::Gallery {

std::unique_ptr<PAL::Platform> CreateSDL2Platform();

} // namespace CarHMI::Gallery
