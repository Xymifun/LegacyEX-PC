#ifndef GAME_UI_UISCENE_CREDITS_HPP
#define GAME_UI_UISCENE_CREDITS_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"

struct SCreditTextItemDef;
class UILayer;

enum ECreditTextTypes
{
    ECreditTextTypes_Title = 0,
    ECreditTextTypes_Role = 1,
    ECreditTextTypes_Text = 3
};

class UIScene_Credits : public UIScene
{
public:
    enum ECreditIcons
    {
        ECreditIcons_Iggy,
        ECreditIcons_Miles,
        ECreditIcons_Dolby,
        ECreditIcons_Nintendo
    };

    static const int CREDIT_TEXT_COUNT = 544;
    static const int PRELOADED_CREDIT_COUNT = 20;

    UIScene_Credits(int type, void* data, UILayer* layer);

    bool AlreadySeenCreditText(const std::wstring& text);
    void AddCreditText(const wchar_t* text);
    void addCreditLine(const std::wstring& text);
    int GetDLCCreditsCount();
    void setNextLabel(const std::wstring& text, ECreditTextTypes type);
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleReload();
    SCreditTextItemDef* GetDLCCredits(int index);
    void addImage(ECreditIcons icon);
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleRequestMoreData(int id, bool forward);
    virtual int getSceneType();
    void setLogoLanguage(int language);

private:
    int m_currentCredit;
    int m_creditCount;
    bool m_requestMoreData;
    std::vector<SCreditTextItemDef*> m_dlcCredits;
    std::vector<std::wstring> m_seenCreditText;
};

struct SCreditTextItemDef
{
    const wchar_t* text;
    int primaryId;
    int secondaryId;
    ECreditTextTypes type;
};

#endif
