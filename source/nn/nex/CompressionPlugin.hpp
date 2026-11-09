#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_COMPRESSIONPLUGIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_COMPRESSIONPLUGIN_HPP

#include "nn/nex/Plugin.hpp"

namespace nn
{
namespace nex
{

class CompressionPlugin : public Plugin
{
public:
    enum
    {
        PLUGIN_TYPE_COMPRESSION = 2
    };

    virtual ~CompressionPlugin() {}

    virtual unsigned int GetType() { return PLUGIN_TYPE_COMPRESSION; }
};

}
}

#endif
