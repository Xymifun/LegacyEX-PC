#ifndef GAME_UI_JOINMULTIPLAYERSCREEN_HPP
#define GAME_UI_JOINMULTIPLAYERSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class EditBox;

class JoinMultiplayerScreen : public Screen
{
public:
    JoinMultiplayerScreen(Screen* parentScreen);

    virtual void render(int x, int y, float partialTick);
    virtual void keyPressed(wchar_t key, int action);
    virtual void mouseClicked(int x, int y, int button);
    virtual void buttonClicked(Button* button);
    virtual void init();
    virtual void tick();
    virtual void removed();
    virtual void parseInt(const std::wstring& value, int radix);

private:
    Screen* mParentScreen;
    EditBox* mEditBox;
};

#endif
