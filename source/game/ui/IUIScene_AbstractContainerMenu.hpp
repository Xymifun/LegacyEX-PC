#ifndef GAME_UI_IUISCENE_ABSTRACTCONTAINERMENU_HPP
#define GAME_UI_IUISCENE_ABSTRACTCONTAINERMENU_HPP

#include "game/ui/UIVec2D.hpp"
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/ClickType.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/Tutorial.hpp"

class AbstractContainerMenu;
class Container;
class HtmlString;
class ItemInstance;
class Slot;

enum EToolTipButton
{
    eToolTipButton_Count = 16
};

enum EToolTipItem
{
    eToolTipNone = -1,
    eToolTipTakePlace,
    eToolTipExit,
    eToolTipTake,
    eToolTipTakeAll,
    eToolTipTakeHalf,
    eToolTipPlace,
    eToolTipPlaceOne,
    eToolTipDistribute,
    eToolTipPlaceAll,
    eToolTipDrop,
    eToolTipDropOne,
    eToolTipDropAll,
    eToolTipSwap,
    eToolTipQuickMove,
    eToolTipQuickMoveIngredient,
    eToolTipQuickMoveFuel,
    eToolTipWhatIsThis,
    eToolTipEquip,
    eToolTipClearQuickSelect,
    eToolTipQuickMoveTool,
    eToolTipQuickMoveArmor,
    eToolTipQuickMoveWeapon,
    eToolTipDye,
    eToolTipRepair,
    eToolTipTakeEverything,
    eToolTipSelect
};

class IUIScene_AbstractContainerMenu
{
public:
    enum ESceneSection
    {
        ESceneSection_None = -1,
        ESceneSection_ContainerHotbar,
        ESceneSection_ContainerInventory,
        ESceneSection_ContainerChest,
        ESceneSection_ContainerMax,
        ESceneSection_FurnaceHotbar,
        ESceneSection_FurnaceInventory,
        ESceneSection_FurnaceIngredient,
        ESceneSection_FurnaceFuel,
        ESceneSection_FurnaceResult,
        ESceneSection_FurnaceMax,
        ESceneSection_InventoryHotbar,
        ESceneSection_InventoryInventory,
        ESceneSection_InventoryArmor,
        ESceneSection_InventoryShield,
        ESceneSection_InventoryCrafting,
        ESceneSection_InventoryResult,
        ESceneSection_InventoryMax,
        ESceneSection_DispenserHotbar,
        ESceneSection_DispenserInventory,
        ESceneSection_DispenserSlots,
        ESceneSection_DispenserMax,
        ESceneSection_CreativeHotbar,
        ESceneSection_CreativeSelector,
        ESceneSection_CreativeTab0,
        ESceneSection_CreativeTab1,
        ESceneSection_CreativeTab2,
        ESceneSection_CreativeTab3,
        ESceneSection_CreativeTab4,
        ESceneSection_CreativeTab5,
        ESceneSection_CreativeTab6,
        ESceneSection_CreativeTab7,
        ESceneSection_CreativeSlider,
        ESceneSection_CreativeMax,
        ESceneSection_EnchantHotbar,
        ESceneSection_EnchantInventory,
        ESceneSection_EnchantItem,
        ESceneSection_EnchantLapis,
        ESceneSection_EnchantButton1,
        ESceneSection_EnchantButton2,
        ESceneSection_EnchantButton3,
        ESceneSection_EnchantMax,
        ESceneSection_BrewingHotbar,
        ESceneSection_BrewingInventory,
        ESceneSection_BrewingBottle1,
        ESceneSection_BrewingBottle2,
        ESceneSection_BrewingBottle3,
        ESceneSection_BrewingIngredient,
        ESceneSection_BrewingFuel,
        ESceneSection_BrewingMax,
        ESceneSection_AnvilHotbar,
        ESceneSection_AnvilInventory,
        ESceneSection_AnvilItem1,
        ESceneSection_AnvilItem2,
        ESceneSection_AnvilResult,
        ESceneSection_AnvilName,
        ESceneSection_AnvilMax,
        ESceneSection_BeaconHotbar,
        ESceneSection_BeaconInventory,
        ESceneSection_BeaconActivator,
        ESceneSection_BeaconPrimary1,
        ESceneSection_BeaconPrimary2,
        ESceneSection_BeaconPrimary3,
        ESceneSection_BeaconPrimary4,
        ESceneSection_BeaconPrimary5,
        ESceneSection_BeaconSecondary1,
        ESceneSection_BeaconSecondary2,
        ESceneSection_BeaconConfirm,
        ESceneSection_BeaconMax,
        ESceneSection_HopperHotbar,
        ESceneSection_HopperInventory,
        ESceneSection_HopperSlots,
        ESceneSection_HopperMax,
        ESceneSection_HorseHotbar,
        ESceneSection_HorseInventory,
        ESceneSection_HorseChest,
        ESceneSection_HorseArmor,
        ESceneSection_HorseSaddle,
        ESceneSection_HorseMax,
        ESceneSection_ClassicCraftingHotbar,
        ESceneSection_ClassicCraftingInventory,
        ESceneSection_ClassicCraftingResult,
        ESceneSection_ClassicCraftingGrid,
        ESceneSection_ClassicCraftingMax
    };

    enum ETapState
    {
        ETapState_None = 0,
        ETapState_Up = 1,
        ETapState_Down = 2,
        ETapState_Left = 3,
        ETapState_Right = 4,
        ETapState_NoMove = 5,
        ETapState_Cancelled = 6
    };

    AbstractContainerMenu* m_menu;
    bool m_autoDeleteMenu;
    bool m_takeAllEnabled;
    bool m_quickCraftDisabled;
    eTutorial_State m_previousTutorialState;
    _UIVec2D m_pointerPos;
    _UIVec2D m_pointerImageOffset;
    float m_pointerMinX;
    float m_pointerMaxX;
    float m_pointerMinY;
    float m_pointerMaxY;
    float m_panelMinX;
    float m_panelMaxX;
    float m_panelMinY;
    float m_panelMaxY;
    int m_consecutiveInputTicks;
    ETapState m_tapState;
    ESceneSection m_currentSection;
    int m_currentSlotX;
    int m_currentSlotY;
    ESceneSection m_firstSection;
    ESceneSection m_maxSection;
    EToolTipItem m_toolTips[eToolTipButton_Count];
    bool m_pointerOutsideMenu;
    Slot* m_pointerLabelSlot;
    bool m_smallInventory;
    bool m_pointerControlled;
    bool m_navigateBack;

    IUIScene_AbstractContainerMenu();
    virtual bool IsSectionSlotList(ESceneSection section);
#ifdef MINECRAFT_PC
    void PcContainerExtras();
    virtual bool PcAllowsHotbarSwap() { return true; }
    virtual void PcWheelAndDrag(int wheelSteps, bool leftHeld) {}
#endif
    virtual unsigned int CanHaveFocus(ESceneSection section);
    virtual unsigned int IsVisible(ESceneSection section);
    virtual int getSectionColumns(ESceneSection section) = 0;
    virtual int getSectionRows(ESceneSection section) = 0;
    virtual unsigned int GetSectionAndSlotInDirection(ESceneSection section, ETapState tapState, int* slotX,
                                                      int* slotY) = 0;
    virtual void GetPositionOfSection(ESceneSection section, _UIVec2D* position) = 0;
    virtual void GetItemScreenData(ESceneSection section, int itemIndex, _UIVec2D* position, _UIVec2D* size) = 0;
    virtual void PlatformInitialize(bool navigateBack, int pad, int startIndex) = 0;
    virtual void InitDataAssociations(int pad, AbstractContainerMenu* menu, int startIndex) = 0;
    virtual unsigned int handleValidKeyPress(int pad, int buttonNum, int quickKeyHeld);
    virtual void handleOutsideClicked(int pad, int buttonNum, int quickKeyHeld);
    virtual void handleOtherClicked(int pad, ESceneSection section, int buttonNum, bool quickKey);
    virtual void handleAdditionalKeyPress(int action);
    virtual void handleSlotListClicked(ESceneSection section, int buttonNum, int quickKeyHeld);
    virtual void handleTakeAll(int buttonNum);
    virtual void handleSectionClick(ESceneSection section) = 0;
    virtual int getSectionStartOffset(ESceneSection section) = 0;
    virtual bool doesSectionTreeHaveFocus(ESceneSection section) = 0;
    virtual void setSectionFocus(ESceneSection section, int pad) = 0;
    virtual void setSectionSelectedSlot(ESceneSection section, int slotX, int slotY, bool pointer) = 0;
    virtual void setFocusToPointer(int pad) = 0;
    virtual void SetPointerText(std::vector<HtmlString>* description, bool newSlot) = 0;
    virtual std::vector<HtmlString>* GetSectionHoverText(ESceneSection section);
    virtual not_null_ptr<ItemInstance> getSlotItem(ESceneSection section, int slotIndex) = 0;
    virtual Slot* getSlot(ESceneSection section, int slotIndex) = 0;
    virtual bool isSlotEmpty(ESceneSection section, int slotIndex) = 0;
    virtual void adjustPointerForSafeZone() = 0;
    virtual unsigned int overrideTooltips(ESceneSection section, not_null_ptr<ItemInstance> slotItem,
                                          bool carriedIsItem, bool slotIsItem, bool carriedSameAsSlot, int slotSpace,
                                          EToolTipItem& buttonA, EToolTipItem& buttonX, EToolTipItem& buttonY,
                                          EToolTipItem& buttonRT, EToolTipItem& buttonBack, EToolTipItem& buttonLB,
                                          EToolTipItem& buttonRB);
    virtual ~IUIScene_AbstractContainerMenu();
    virtual int getPad() = 0;

    bool canTakeAll() const;
    bool canQuickCraft() const;
    int GetSectionDimensions(ESceneSection section, int* columns, int* rows);
    void updateSlotPosition(ESceneSection section, ESceneSection newSection, ETapState tapState, int* slotX, int* slotY,
                            int xOffset, int yOffset);
    ETapState GetTapInputType(float inputX, float inputY);
    void UpdateTooltips();
    void SetToolTip(EToolTipButton button, EToolTipItem item);
    void Initialize(bool navigateBack, int pad, AbstractContainerMenu* menu, bool autoDeleteMenu, int startIndex,
                    ESceneSection firstSection, ESceneSection maxSection, bool navigateBackOnClose);
    std::vector<HtmlString>* GetItemDescription(Slot* slot);
    bool IsSameItemAs(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second);
    int GetEmptyStackSpace(Slot* slot);
    void recalculateQuickCraftRemaining();
    void onMouseTick();
    int getCurrentIndex(ESceneSection section);
    void slotClicked(int slotId, int buttonNum, ClickType::eClickType clickType);
    void handleQuickCraft();
    bool handleKeyDown(int pad, int action, bool repeat);
    int GetInventoryRows();
};

#endif
