#ifndef PLATFORM_PC_PCTEXTUREOVERRIDE_HPP
#define PLATFORM_PC_PCTEXTUREOVERRIDE_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_TexturePackList.hpp"
#include "game/ui/UIScene.hpp"

unsigned int PcTextureOverrideApply(unsigned int packId);
void PcTextureOverrideSkip(bool skip);
bool PcTextureOverrideActive();
unsigned int PcTextureOverrideWorldPack(unsigned int selectedId);

class TexturePack;
TexturePack* PcTextureOverrideMusicPack();
bool PcTextureOverrideGameMusic();
unsigned int PcTextureOverrideThemePack(unsigned int selectedId);

class UIScene_PcTextureOverride : public UIScene
{
public:
    UIScene_PcTextureOverride(int player, void* data, UILayer* layer);
    virtual ~UIScene_PcTextureOverride();

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleCheckboxToggled(int id, bool checked);
    virtual void handleSelectionChanged(int selection);
    virtual UIControl* GetMainPanel();

private:
    void FillPacks();
    void ShowPack(unsigned int packId);
    void Layout();
    void Leave();

    UIControl m_mainPanel;
    UIControl_Label m_title;
    UIControl m_nameBox;
    UIControl m_modeSlider;
    UIControl m_difficultySlider;
    UIControl m_moreButton;
    UIControl_CheckBox m_enableBox;
    UIControl_CheckBox m_musicBox;
    UIControl_CheckBox m_miniGameBox;
    UIControl_TexturePackList m_packList;
    UIControl_Button m_doneButton;
    UIControl_Label m_packName;
    UIControl_HTMLLabel m_packDesc;
    UIControl_BitmapIcon m_packIcon;
    UIControl_BitmapIcon m_packComparison;
    std::vector<unsigned int> m_packs;
    int m_slot;
    int m_pendingSlot;
    bool m_enabled;
    bool m_defaultMusic;
    bool m_miniGameMusic;
    bool m_changed;
    bool m_left;
};

#endif
