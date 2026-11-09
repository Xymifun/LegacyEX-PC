#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_ISESSIONINFOLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_ISESSIONINFOLIST_HPP

namespace nn
{
namespace pia
{
namespace session
{

class ISessionInfo;

class ISessionInfoList
{
public:
    virtual ~ISessionInfoList() {}
    virtual const ISessionInfo** Begin() const = 0;
    virtual ISessionInfo** Begin() = 0;
    virtual const ISessionInfo** End() const = 0;
    virtual ISessionInfo** End() = 0;
    virtual unsigned short GetSize() const = 0;
    virtual unsigned short GetMaxSize() const = 0;
    virtual void Clear() = 0;
};

}
}
}

#endif
