#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DEBUGSTRING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DEBUGSTRING_HPP

namespace nn
{
namespace nex
{

class String;

class DebugString
{
public:
    DebugString(const String&) {}

    DebugString(const wchar_t*) {}
};

}
}

#endif
