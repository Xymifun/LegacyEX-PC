#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_UNSUPPORTEDNATREFEREE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_UNSUPPORTEDNATREFEREE_HPP

namespace nn
{
namespace nex
{

class UnsupportedNATReferee
{
public:
    UnsupportedNATReferee();

    void Succeeded();

private:
    unsigned int m_uiFailureCount;
};

}
}

#endif
