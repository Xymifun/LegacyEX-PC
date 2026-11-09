#include "game/ui/FJ_HorseInventoryMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_HorseInventoryMenu::FJ_HorseInventoryMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_AbstractContainerDocument* FJ_HorseInventoryMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_HorseInventoryMenu(renderNode);
}

unsigned int FJ_HorseInventoryMenu::getType()
{
    return 0x24;
}

void FJ_HorseInventoryMenu::SetInventoryType(int type)
{
    FJ_FuiNode* mainPanel = getChildByName("MainPanel");
    switch (type)
    {
    case 0:
    {
        FJ_FuiNode* carpetIcon = mainPanel->getChildByName("SlotTypes.SlotArmour.CarpetIcon");
        carpetIcon->setVisible(false);
        break;
    }
    case 1:
    {
        FJ_FuiNode* armourType = mainPanel->getChildByName("SlotTypes.SlotArmour");
        FJ_FuiNode* armourSlot = mainPanel->getChildByName("SlotArmor");
        armourType->setVisible(false);
        mainPanel->removeChild(armourSlot, true);
        break;
    }
    case 2:
    {
        FJ_FuiNode* saddleType = mainPanel->getChildByName("SlotTypes.SlotSaddle");
        FJ_FuiNode* saddleSlot = mainPanel->getChildByName("SlotSaddle");
        saddleType->setVisible(false);
        mainPanel->removeChild(saddleSlot, true);
        FJ_FuiNode* horseArmorIcon = mainPanel->getChildByName("SlotTypes.SlotArmour.HorseArmorIcon");
        horseArmorIcon->setVisible(false);
        break;
    }
    }
}

void FJ_HorseInventoryMenu::SetHasInventory(bool hasInventory)
{
    FJ_FuiNode* mainPanel = getChildByName("MainPanel");
    FJ_FuiNode* inventoryList = mainPanel->getChildByName("DonkeyInventoryList");
    if (!hasInventory)
        mainPanel->removeChild(inventoryList, true);
}

void FJ_HorseInventoryMenu::SetCanHaveSaddle(bool canHaveSaddle)
{
    FJ_FuiNode* mainPanel = getChildByName("MainPanel");
    FJ_FuiNode* saddleType = mainPanel->getChildByName("SlotTypes.SlotSaddle");
    FJ_FuiNode* saddleSlot = mainPanel->getChildByName("SlotSaddle");
    if (!canHaveSaddle && saddleSlot != 0)
    {
        saddleType->setVisible(false);
        mainPanel->removeChild(saddleSlot, true);
    }
}

void FJ_HorseInventoryMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        setX(-right);
        setY(top);
    }
}
