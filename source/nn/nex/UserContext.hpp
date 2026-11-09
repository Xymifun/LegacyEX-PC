#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_USERCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_USERCONTEXT_HPP

namespace nn
{
namespace nex
{

class UserContext
{
public:
    UserContext() : m_pPointer(0) {}

    UserContext(void* pPointer) : m_pPointer(pPointer) {}

    void* GetPointer() const { return m_pPointer; }

private:
    void* m_pPointer;
};

}
}

#endif
