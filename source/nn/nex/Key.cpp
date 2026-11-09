#include "nn/nex/Key.hpp"

#include <string.h>

#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

Key::Key()
{
}

Key::Key(const unsigned char* pContent, unsigned int uiLength) : m_content(uiLength)
{
    for (unsigned int i = 0; i < uiLength; ++i)
        m_content[i] = pContent[i];
}

Key::Key(unsigned int uiLength) : m_content(uiLength)
{
    memset(&m_content[0], 0, uiLength);
}

Key::Key(const String& rString)
{
    unsigned int uiLength = rString.GetLength() >> 1;
    m_content.resize(uiLength);
    for (unsigned int i = 0; i < uiLength; ++i)
    {
        unsigned int cHigh = static_cast<const wchar_t*>(rString)[i * 2];
        unsigned int cLow = static_cast<const wchar_t*>(rString)[i * 2 + 1];
        unsigned int uiHigh = (cHigh >= '0' && cHigh <= '9') ? cHigh - '0' : cHigh - 'a' + 10;
        unsigned int uiLow = (cLow >= '0' && cLow <= '9') ? cLow - '0' : cLow - 'a' + 10;
        m_content[i] = static_cast<unsigned char>((uiHigh << 4) | uiLow);
    }
}

Key::Key(const Key& rKey) : RefCountedObject(), m_content(rKey.m_content)
{
}

Key::~Key()
{
}

Key& Key::operator=(const Key& rKey)
{
    m_content = rKey.m_content;
    return *this;
}

unsigned int Key::GetLength() const
{
    return static_cast<unsigned int>(m_content.size());
}

const unsigned char* Key::GetContentPtr() const
{
    if (m_content.empty())
        return 0;

    return &m_content[0];
}

unsigned char* Key::GetContentPtr()
{
    if (m_content.empty())
        return 0;

    return &m_content[0];
}

unsigned char* Key::PrepareContentPtr(unsigned int uiLength)
{
    m_content.resize(uiLength);
    return &m_content[0];
}

void Key::ExtractToString(String* pString) const
{
    String& string = *pString;
    string.Reserve(GetLength() * 2 + 1);
    unsigned int i;
    for (i = 0; i < GetLength(); ++i)
    {
        unsigned char uiHigh = (m_content[i] >> 4) & 0x0F;
        unsigned char uiLow = m_content[i] & 0x0F;
        string[i * 2] = (uiHigh <= 9 ? '0' : 'a' - 10) + uiHigh;
        string[i * 2 + 1] = (uiLow <= 9 ? '0' : 'a' - 10) + uiLow;
    }
    string[i * 2] = 0;
}

String Key::ToString() const
{
    String string;
    ExtractToString(&string);
    return string;
}

void Key::Trace(uint64_t uiFilter) const
{
    if (GetLength() == 0)
        return;

    String string;
    ExtractToString(&string);
}

}
}
