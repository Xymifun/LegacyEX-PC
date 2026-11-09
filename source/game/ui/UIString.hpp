#ifndef GAME_UI_UISTRING_HPP
#define GAME_UI_UISTRING_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/core/FJStringBuilder.hpp"

class UIString
{
public:
    class UIStringCore : public boost::enable_shared_from_this<UIStringCore>
    {
    public:
        explicit UIStringCore(FJStringBuilder builder);
        explicit UIStringCore(const std::wstring& value);
        explicit UIStringCore(const std::string& value);
        unsigned char hasNewString();
        unsigned int update(bool force);
        unsigned char needsUpdating();
        void setUpdated();
        std::wstring& getString();
        std::string& getCharString();
        unsigned char isCharString();
        unsigned int length();

    private:
        unsigned int mLanguage;
        unsigned int mLocale;
        unsigned int mUpdatedLanguage;
        unsigned int mUpdatedLocale;
        std::wstring mString;
        std::string mCharString;
        bool mIsConstant;
        bool mIsCharString;
        FJStringBuilder mBuilder;
    };
    UIString();
    explicit UIString(int id);
    explicit UIString(FJStringBuilder builder);
    explicit UIString(const std::string& value);
    explicit UIString(const char* value);
    explicit UIString(const std::wstring& value);
    explicit UIString(const wchar_t* value);
    ~UIString();
    static unsigned int setCurrentLanguage();
    static unsigned int getCurrentLanguage();
    bool empty();
    bool compare(const UIString& value);
    unsigned char needsUpdating();
    void setUpdated();
    std::wstring& getString();
    std::string& getCharString();
    unsigned char isCharString();
    const wchar_t* c_str();
    unsigned int length();

private:
    boost::shared_ptr<UIStringCore> mCore;
};

#endif
