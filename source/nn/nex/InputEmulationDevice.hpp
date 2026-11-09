#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_INPUTEMULATIONDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_INPUTEMULATIONDEVICE_HPP

#include "nn/nex/EmulationDevice.hpp"

namespace nn
{
namespace nex
{

class InputEmulationDevice : public EmulationDevice
{
public:
    InputEmulationDevice() {}

    virtual ~InputEmulationDevice() {}
};

}
}

#endif
