#include "game/ui/UIString.hpp"

#include "game/core/IdsStringBuilder.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/XLanguage.hpp"


static unsigned int gCurrentLanguage = static_cast<unsigned int>(-1);
static unsigned int gCurrentLocale = static_cast<unsigned int>(-1);

UIString::UIStringCore::UIStringCore(FJStringBuilder builder)
{
    mLanguage = static_cast<unsigned int>(-1);
    mLocale = static_cast<unsigned int>(-1);
    mUpdatedLanguage = static_cast<unsigned int>(-1);
    mUpdatedLocale = static_cast<unsigned int>(-1);
    mIsConstant = false;
    mIsCharString = false;
    mBuilder = builder;
    mString = L"";
    update(true);
}

UIString::UIStringCore::UIStringCore(const std::wstring& value)
{
    mLanguage = static_cast<unsigned int>(-1);
    mLocale = static_cast<unsigned int>(-1);
    mUpdatedLanguage = static_cast<unsigned int>(-1);
    mUpdatedLocale = static_cast<unsigned int>(-1);
    mIsConstant = true;
    mIsCharString = false;
    mString = value;
}

UIString::UIStringCore::UIStringCore(const std::string& value)
{
    mLanguage = static_cast<unsigned int>(-1);
    mLocale = static_cast<unsigned int>(-1);
    mUpdatedLanguage = static_cast<unsigned int>(-1);
    mUpdatedLocale = static_cast<unsigned int>(-1);
    mIsConstant = true;
    mIsCharString = true;
    mCharString = value;
}

unsigned char UIString::UIStringCore::hasNewString()
{
    if (mIsConstant)
        return 0;
    unsigned int result = 1;
    if (mLanguage == gCurrentLanguage && mLocale == gCurrentLocale)
        result = 0;
    return result;
}

unsigned int UIString::UIStringCore::update(bool force)
{
    if (mIsConstant)
        return 0;
    if (!force && hasNewString() == 0)
        return 0;
    mString = mBuilder();
    mLanguage = gCurrentLanguage;
    mLocale = gCurrentLocale;
    return 1;
}

unsigned char UIString::UIStringCore::needsUpdating()
{
    if (mIsConstant)
        return 0;
    unsigned int result = 1;
    if (mLanguage == gCurrentLanguage && mUpdatedLanguage == mLanguage && mLocale == gCurrentLocale
        && mUpdatedLocale == mLocale)
        result = 0;
    return result;
}

void UIString::UIStringCore::setUpdated()
{
    mUpdatedLanguage = mLanguage;
    mUpdatedLocale = mLocale;
}

std::wstring& UIString::UIStringCore::getString()
{
    if (hasNewString() != 0)
        update(true);
    return mString;
}

std::string& UIString::UIStringCore::getCharString()
{
    if (hasNewString() != 0)
        update(true);
    return mCharString;
}

unsigned char UIString::UIStringCore::isCharString()
{
    return mIsCharString;
}

UIString::UIString()
{
    mCore = boost::shared_ptr<UIStringCore>();
}

UIString::UIString(int id)
{
    FJStringBuilder builder(new IdsStringBuilder(id));
    UIStringCore* core = new UIStringCore(builder);
    mCore = boost::shared_ptr<UIStringCore>(core);
}

UIString::UIString(FJStringBuilder builder)
{
    UIStringCore* core = new UIStringCore(builder);
    mCore = boost::shared_ptr<UIStringCore>(core);
}

UIString::UIString(const std::string& value)
{
    UIStringCore* core = new UIStringCore(convStringToWstring(value));
    mCore = boost::shared_ptr<UIStringCore>(core);
}

UIString::UIString(const char* value)
{
    UIStringCore* core = new UIStringCore(std::string(value));
    mCore = boost::shared_ptr<UIStringCore>(core);
}

UIString::UIString(const std::wstring& value)
{
    UIStringCore* core = new UIStringCore(value);
    mCore = boost::shared_ptr<UIStringCore>(core);
}

UIString::UIString(const wchar_t* value)
{
    mCore = boost::shared_ptr<UIStringCore>(new UIStringCore(std::wstring(value)));
}

UIString::~UIString()
{
    mCore = boost::shared_ptr<UIStringCore>();
}

unsigned int UIString::setCurrentLanguage()
{
    unsigned int language = XGetLanguage();
    unsigned int localeId = static_cast<unsigned int>(XGetLocale());
    if (language == gCurrentLanguage && localeId == gCurrentLocale)
        return 0;
    gCurrentLocale = localeId;
    gCurrentLanguage = language;
    return 1;
}

unsigned int UIString::getCurrentLanguage()
{
    return gCurrentLanguage;
}

bool UIString::empty()
{
    return !mCore;
}

bool UIString::compare(const UIString& value)
{
    return mCore != value.mCore;
}

unsigned char UIString::needsUpdating()
{
    if (mCore)
        return mCore->needsUpdating();
    return 0;
}

void UIString::setUpdated()
{
    if (!mCore)
        return;
    mCore->setUpdated();
}

std::wstring& UIString::getString()
{
    static std::wstring emptyString(L"");
    if (mCore)
        return mCore->getString();
    return emptyString;
}

std::string& UIString::getCharString()
{
    static std::string emptyString("");
    if (mCore)
        return mCore->getCharString();
    return emptyString;
}

unsigned char UIString::isCharString()
{
    if (!mCore)
        return 0;
    return mCore->isCharString();
}

const wchar_t* UIString::c_str()
{
    return &getString()[0];
}
