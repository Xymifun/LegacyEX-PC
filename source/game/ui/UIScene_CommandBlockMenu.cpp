#include "game/ui/UIScene_CommandBlockMenu.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

extern CConsoleMinecraftApp gApp;

UIScene_CommandBlockMenu::UIScene_CommandBlockMenu(int type, void*, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer)
{
}

UIScene_CommandBlockMenu::~UIScene_CommandBlockMenu()
{
}

std::wstring UIScene_CommandBlockMenu::getMoviePath()
{
    int playerCount = gApp.GetLocalPlayerCount();
    if (playerCount > 1)
        return std::wstring(L"CommandBlockMenuSplit");
    return std::wstring(L"CommandBlockMenu");
}

void UIScene_CommandBlockMenu::Initialise(CommandBlockEntity* commandBlock)
{
}

void UIScene_CommandBlockMenu::ConfirmButtonClicked()
{
}
