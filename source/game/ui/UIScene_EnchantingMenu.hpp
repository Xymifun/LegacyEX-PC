#ifndef GAME_UI_UISCENE_ENCHANTINGMENU_HPP
#define GAME_UI_UISCENE_ENCHANTINGMENU_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl_EnchantmentBook.hpp"
#include "game/ui/UIControl_EnchantmentButton.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SlotList.hpp"

class EnchantmentMenu;
class UILayer;

class UIScene_EnchantingMenu : public UIScene_AbstractContainerMenu
{
public:
    UIScene_EnchantingMenu(int pad, void* data, UILayer* layer);
    virtual ~UIScene_EnchantingMenu();

    virtual int getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position);
    virtual void GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, _UIVec2D* position,
                                   _UIVec2D* size);
    virtual void handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction,
                                        bool wrap);
    virtual unsigned int GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                      IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y);
    virtual void handleOtherClicked(int pad, IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                    bool pressed);
    virtual int getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual bool IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual std::vector<HtmlString>* GetSectionHoverText(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int overrideTooltips(IUIScene_AbstractContainerMenu::ESceneSection section,
                                          not_null_ptr<ItemInstance> item, bool itemValid, bool canTake, bool canPlace,
                                          int count, EToolTipItem& first, EToolTipItem& second, EToolTipItem& third,
                                          EToolTipItem& fourth, EToolTipItem& fifth, EToolTipItem& sixth,
                                          EToolTipItem& seventh);

    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual int getSceneType();

    EnchantmentMenu* getMenu();
    bool IsActiveButton(IUIScene_AbstractContainerMenu::ESceneSection section);

private:
    UIControl_SlotList m_ingredientSlot;
    UIControl_SlotList m_lapisSlot;
    UIControl_Label m_enchantLabel;
    UIControl_EnchantmentButton m_enchantmentButtons[3];
    UIControl_EnchantmentBook m_enchantmentBook;
};

#endif
