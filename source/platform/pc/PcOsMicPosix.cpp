#ifndef _WIN32

#include "platform/pc/PcOs.hpp"

void PcOsMicListDevices(std::vector<PcOsMicDevice>* devices)
{
    devices->clear();
}

bool PcOsMicOpen(const std::string&, unsigned int, PcOsMicDeliverFn, std::string*)
{
    return false;
}

void PcOsMicClose()
{
}

#endif
