#ifndef GAME_UI_RENAMEWORLDSCREEN_HPP
#define GAME_UI_RENAMEWORLDSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class Button;
class EditBox;

class RenameWorldScreen : public Screen
{
public:
    RenameWorldScreen(Screen* parentScreen, const std::wstring& levelId);

    virtual void render(int x, int y, float partialTick);
    virtual void keyPressed(wchar_t key, int action);
    virtual void mouseClicked(int x, int y, int button);
    virtual void buttonClicked(Button* button);
    virtual void init();
    virtual void tick();
    virtual void removed();

private:
    Screen* mParentScreen;
    EditBox* mEditBox;
    std::wstring mLevelId;
};

#endif
