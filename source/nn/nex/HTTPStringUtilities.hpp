#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPSTRINGUTILITIES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPSTRINGUTILITIES_HPP

#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class HTTPStringUtilities
{
public:
    static String TruncatePrefix(String& rSource, unsigned int uiPosition, unsigned int uiLength);
    static String TruncateSuffix(String& rSource, unsigned int uiPosition, unsigned int uiLength);
    static String GetNextToken(String& rSource, const String& rDelimiter);
};

}
}

#endif
