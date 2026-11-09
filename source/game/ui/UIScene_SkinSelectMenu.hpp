#ifndef GAME_UI_UISCENE_SKINSELECTMENU_HPP
#define GAME_UI_UISCENE_SKINSELECTMENU_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/IUIScene_DlcSelectMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIControl_PlayerSkinPreview.hpp"
#include "game/ui/UIControl_Touch.hpp"

struct SKIN_BOX;

class UIScene_SkinSelectMenu : public UIScene, public IUIScene_DlcSelectMenu
{
public:
    enum ECharacter
    {
        eCharacter_Current,
        eCharacter_Next1,
        eCharacter_Next2,
        eCharacter_Next3,
        eCharacter_Previous1,
        eCharacter_Previous2,
        eCharacter_Previous3,
        eCharacter_Count
    };

    UIScene_SkinSelectMenu(int player, void* data, UILayer* layer);
    virtual ~UIScene_SkinSelectMenu();

    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleInput(int player, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleAnimationEnd();
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled();
    virtual void HandleDLCLicenseChange();
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual int getSceneType();

    void setCharacterLocked(bool locked);
    void setCharacterSelected(bool selected);
    void setCharacterFavourite(bool favourite);
    void setCharacterBlocked(bool blocked);
    void setActivePackIndex();
    unsigned int registerTexture(std::wstring textureName);
    unsigned char getTextureId(int index);
    unsigned int getNextSkinIndex(unsigned int index);
    int getPreviousSkinIndex(unsigned int index);
    void handleSkinIndexChanged();
    unsigned char IsTrialDiscontinuedPack(int packId);
    unsigned int GetIsNewPack(int packId);
    static bool SORTNEWEST(int first, int second);
    static bool SORTALPHABETICALLY(int first, int second);
    void SetSkinPackButtonList();
    void setPackLabel();
    void updatePackDisplay();
    void handlePackIndexChanged();
    void InputActionOK(unsigned int player);
    static int ManageFavoritesReturned(void* context, int player, C4JStorage::EMessageResult result);
    void SetFavoriteSkin(int player, int skin);
    void InputActionFavorite(unsigned int player);
    unsigned int IsFavoriteSkin(int player, int skin);

private:
    UIControl_PlayerSkinPreview m_characters[eCharacter_Count];
    UIControl_Label m_labelSkinName;
    UIControl_Label m_labelSkinOrigin;
    UIControl_Label m_labelPackName;
    UIControl_Label m_labelPackType;
    UIControl m_controlCharacters;
    UIControl m_controlTimer;
    UIControl_Touch m_touchCharacters;
    UIControl_MultiList m_packList;
    UIControl_BitmapIcon m_packIcon;
    std::vector<int> m_packIds;
    std::vector<int> m_newPackIds;
    std::vector<int> m_addedPackIds;
    int m_selectedPackId;
    unsigned int m_packIndex;
    unsigned int m_skinIndex;
    int m_originalSkinId;
    std::wstring m_currentSkinPath;
    std::wstring m_selectedSkinPath;
    std::wstring m_selectedCapePath;
    std::vector<SKIN_BOX*>* m_additionalSkinBoxes;
    bool m_skinAnimating;
    bool m_skinSliding;
    bool m_noSkinsToShow;
    bool m_skinUpdatePending;
    bool m_packFocusChanged;
    bool m_packListDirty;
    bool m_hasTexturePackSkinPack;
    int m_texturePackSkinPackId;
    int m_touchStartX;
    bool m_touchConsumed;
};

#endif
