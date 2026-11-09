#ifndef GAME_UI_UISCENE_COMMANDBLOCKMENU_HPP
#define GAME_UI_UISCENE_COMMANDBLOCKMENU_HPP

#include <string>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/IUIScene_AbstractContainerMenu.hpp"

class UILayer;
class CommandBlockEntity;

class UIScene_CommandBlockMenu : public UIScene_AbstractContainerMenu
{
public:
    UIScene_CommandBlockMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_CommandBlockMenu();
    virtual std::wstring getMoviePath();
    void Initialise(CommandBlockEntity* commandBlock);
    void ConfirmButtonClicked();
};

#endif
