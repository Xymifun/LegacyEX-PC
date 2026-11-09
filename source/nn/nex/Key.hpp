#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_KEY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_KEY_HPP

#include <stdint.h>

#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class String;

class Key : public RefCountedObject
{
public:
    Key();
    Key(const unsigned char* pContent, unsigned int uiLength);
    explicit Key(unsigned int uiLength);
    explicit Key(const String& rString);
    Key(const Key& rKey);
    virtual ~Key();

    Key& operator=(const Key& rKey);

    unsigned int GetLength() const;
    const unsigned char* GetContentPtr() const;
    unsigned char* GetContentPtr();
    unsigned char* PrepareContentPtr(unsigned int uiLength);

    void ExtractToString(String* pString) const;
    String ToString() const;
    void Trace(uint64_t uiFilter) const;

private:
    qVector<unsigned char> m_content;
};

}
}

#endif
