#include "nn/nex/HTTPStringUtilities.hpp"

namespace nn
{
namespace nex
{

String HTTPStringUtilities::TruncatePrefix(String& rSource, unsigned int uiPosition, unsigned int uiLength)
{
    String prefix = rSource.Truncate(uiPosition);
    rSource = String(static_cast<const wchar_t*>(rSource) + uiPosition + uiLength);
    return prefix;
}

String HTTPStringUtilities::GetNextToken(String& rSource, const String& rDelimiter)
{
    String token;
    int iPosition = rSource.FindSubstringCase(rDelimiter, 0);
    if (iPosition >= 0)
    {
        token = TruncatePrefix(rSource, iPosition, rDelimiter.GetLength());
        return token;
    }
    token = rSource;
    rSource = L"";
    return token;
}

String HTTPStringUtilities::TruncateSuffix(String& rSource, unsigned int uiPosition, unsigned int uiLength)
{
    String suffix(static_cast<const wchar_t*>(rSource) + uiPosition + uiLength);
    rSource = rSource.Truncate(uiPosition);
    return suffix;
}

}
}
