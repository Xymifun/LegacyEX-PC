#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_ICOMMUNITYINFOLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_ICOMMUNITYINFOLIST_HPP

namespace nn
{
namespace pia
{
namespace session
{

class ICommunityInfo;

class ICommunityInfoList
{
public:
    virtual ~ICommunityInfoList() {}
    virtual ICommunityInfo* const* Begin() const = 0;
    virtual ICommunityInfo** Begin() = 0;
    virtual ICommunityInfo* const* End() const = 0;
    virtual ICommunityInfo** End() = 0;
    virtual unsigned int GetSize() const = 0;
    virtual unsigned int GetMaxSize() const = 0;
    virtual void Clear() = 0;
};

}
}
}

#endif
